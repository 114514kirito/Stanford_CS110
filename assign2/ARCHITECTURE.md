# Stanford CS110 Assignment 2 — Unix V6 文件系统架构解析

## 1. 项目概览

本项目实现了 Unix Version 6 文件系统的**只读层**，编译产出 `diskimageaccess` 程序，用于读取 Unix V6 格式的磁盘镜像文件。

整体采用**自底向上的分层设计**：物理磁盘 I/O → 文件系统挂载 → Inode 管理 → 文件块读取 → 目录解析 → 路径查找 → 校验和计算 → 主程序入口。

---

## 2. 整体层次结构

```
┌──────────────────────────────────────────────────────────────────┐
│                        diskimageaccess.c                        │
│                    (main 入口 / 命令行解析 / 全盘扫描)             │
└────┬───────────────────┬───────────────────┬────────────────────┘
     │                   │                   │
  ┌──▼──────────┐  ┌─────▼───────┐  ┌────────▼──────────┐
  │ chksumfile  │  │  pathname   │  │  PrintDirectory   │
  │  计算 SHA1  │  │ 路径→inode  │  │  (调试输出)        │
  └──────┬──────┘  └─────┬───────┘  └────────┬──────────┘
         │               │                    │
         │        ┌──────▼────────┐           │
         │        │   directory   │           │
         │        │  目录项查找    │           │
         │        └──────┬────────┘           │
         │               │                    │
  ┌──────▼───────────────▼────────────────────▼──┐
  │                  file.c / .h                  │
  │      file_getblock(fs, inumber, bno, buf)    │
  │      ── 根据 inode 读取文件的某个逻辑块         │
  └──────────────────────┬───────────────────────┘
                         │
              ┌──────────▼──────────┐
              │    inode.c / .h     │
              │  inode_iget()       │   ← 从磁盘读取 inode
              │  inode_indexlookup()│   ← 逻辑块号 → 物理块号 (间接块解析)
              │  inode_getsize()    │
              └──────────┬──────────┘
                         │
              ┌──────────▼──────────┐
              │  unixfilesystem     │
              │  文件系统上下文       │
              │  (superblock + dfd) │
              └──────────┬──────────┘
                         │
              ┌──────────▼──────────┐
              │      diskimg        │
              │   以 512B 扇区为     │
              │   单位读写磁盘镜像    │
              └──────────┬──────────┘
                         │
              ┌──────────▼──────────┐
              │   磁盘镜像文件        │
              │  (raw disk image)   │
              │  例如 basicDiskImage │
              └─────────────────────┘
```

---

## 3. 磁盘布局 (V6 Unix 规范)

整块逻辑磁盘由连续编号的 **512 字节扇区（Block）** 组成：

```
┌──────────────┬──────────────┬─────────────────────────────┬──────────────────────┐
│   Block 0    │   Block 1    │   Block 2 ~ Block (2+isize) │     剩余 Block        │
│              │              │                             │                      │
│  Boot 块      │  Super 块     │       Inode 区域              │     数据区域            │
│  (引导代码)    │  (filsys)    │    (inode 表, 每个 32B)        │   (文件内容 + 间接块)     │
│              │              │                             │                      │
│  魔数: 0407   │              │  共 s_isize 个 Block           │  从 Block (2+isize)    │
│              │              │  每个 Block 含 16 个 inode      │  开始                  │
└──────────────┴──────────────┴─────────────────────────────┴──────────────────────┘
```

| 宏常量 | 值 | 含义 |
|---|---|------|
| `BOOTBLOCK_SECTOR` | `0` | Boot 块所在扇区 |
| `SUPERBLOCK_SECTOR` | `1` | Superblock 所在扇区 |
| `INODE_START_SECTOR` | `2` | Inode 区起始扇区 |
| `ROOT_INUMBER` | `1` | 根目录的 inode 号始终为 1 |
| `BOOTBLOCK_MAGIC_NUM` | `0407` | Boot 块的魔数，用于校验 |
| `DISKIMG_SECTOR_SIZE` | `512` | 一个扇区 = 512 字节 |

---

## 4. 逐层详解

### 4.1 `diskimg.c/h` — 物理磁盘 I/O 层

最底层模块，将磁盘镜像文件当作真实磁盘来读写。每个扇区严格 512 字节。

| 函数 | 功能 |
|---|---|
| `diskimg_open(pathname, readOnly)` | 打开磁盘镜像文件，返回 fd |
| `diskimg_readsector(fd, sectorNum, buf)` | 读取第 `sectorNum` 号扇区到 `buf` |
| `diskimg_writesector(fd, sectorNum, buf)` | 将 `buf` 写入第 `sectorNum` 号扇区 |
| `diskimg_getsize(fd)` | 获取磁盘镜像总字节数 |
| `diskimg_close(fd)` | 关闭磁盘镜像 |

**实现原理**：`sectorNum * 512 → lseek → read/write(512)`，用 POSIX 文件 I/O 模拟磁盘操作。

> **状态**：✅ 已完整实现

---

### 4.2 `filsys.h` — Superblock (超级块)

Superblock 是文件系统的**元数据总控**，存储在 Block 1，描述整个文件系统的格局。

```c
struct filsys {
    uint16_t  s_isize;        //  Inode 区域占用多少个 Block
    uint16_t  s_fsize;        //  整个文件系统总 Block 数
    uint16_t  s_nfree;        //  空闲块缓存计数 (0~100)
    uint16_t  s_free[100];    //  空闲块号数组
    uint16_t  s_ninode;       //  空闲 inode 缓存计数 (0~100)
    uint16_t  s_inode[100];   //  空闲 inode 号数组
    uint8_t   s_flock;        //  空闲链表操作锁
    uint8_t   s_ilock;        //  inode 链表操作锁
    uint8_t   s_fmod;         //  Superblock 修改标记
    uint8_t   s_ronly;        //  只读挂载标记
    uint16_t  s_time[2];      //  最后更新时间
    uint16_t  pad[48];        //  填充至 512 字节 (对齐一个 Block)
};
```

- `pad[48]` 使整个结构体刚好 **512 字节**，与 Block 大小对齐
- `s_isize` 是关键字段：决定了 Inode 区的大小 → 最大 inode 号 = `s_isize × 16`

> **状态**：✅ 结构定义，无需修改

---

### 4.3 `unixfilesystem.c/h` — 文件系统上下文

管理文件系统的**运行时状态**，是各模块之间的纽带。

```c
struct unixfilesystem {
    int dfd;                       // 磁盘镜像的文件描述符
    struct filsys superblock;      // 从磁盘读取的 Superblock
};
```

**`unixfilesystem_init(fd)`** 初始化流程：

1. 读取 Block 0 (Bootblock)，校验魔数是否为 `0407`
2. 校验 `sizeof(struct filsys) == 512`
3. `malloc` 分配 `unixfilesystem` 结构体
4. 读取 Block 1 (Superblock)，填充 `fs->superblock`
5. 保存 `dfd`，返回 `fs` 指针

> **状态**：✅ 已完整实现

---

### 4.4 `ino.h` — Inode 结构定义

Inode (Index Node) 是 Unix 文件系统的**核心数据结构**，每个文件/目录都由一个 Inode 描述。所有 Inode 存放在磁盘的 Inode 区域中。

```c
struct inode {
    uint16_t  i_mode;         //  2B: 文件类型 + 权限位
    uint8_t   i_nlink;        //  1B: 硬链接计数
    uint8_t   i_uid;          //  1B: 文件所有者 ID
    uint8_t   i_gid;          //  1B: 文件所属组 ID
    uint8_t   i_size0;        //  1B: 文件大小的高 8 位   ┐
    uint16_t  i_size1;        //  2B: 文件大小的低 16 位  ├─ 3 字节编码
    uint16_t  i_addr[8];      //  16B: 数据块地址数组 ⭐ 核心
    uint16_t  i_atime[2];     //  4B: 最后访问时间
    uint16_t  i_mtime[2];     //  4B: 最后修改时间
};
// 总计 32 字节，每个 512B Block 可容纳 16 个 Inode
```

**关键 mode 位掩码**：

| 宏 | 值 (八进制) | 含义 |
|---|---|------|
| `IALLOC` | `0100000` | Inode 已被分配使用 |
| `IFMT` | `060000` | 文件类型掩码 |
| `IFDIR` | `040000` | 目录 |
| `IFCHR` | `020000` | 字符设备 |
| `IFBLK` | `060000` | 块设备 (同时 `0` 表示普通文件) |
| `ILARG` | `010000` | 大文件标记 (使用间接块寻址) |
| `IREAD` | `0400` | 读权限 |
| `IWRITE` | `0200` | 写权限 |
| `IEXEC` | `0100` | 执行权限 |

---

### 4.5 `inode.c/h` — Inode 操作层

**3 个函数，其中 2 个待实现：**

| 函数 | 职责 | 状态 |
|---|---|------|
| `inode_iget(fs, inumber, inp)` | 从磁盘读取第 `inumber` 号 inode | ❌ 待实现 |
| `inode_indexlookup(fs, inp, blockNum)` | 逻辑块号 → 物理块号，含间接块解析 | ❌ 待实现 |
| `inode_getsize(inp)` | 计算文件大小 = `(i_size0 << 16) \| i_size1` | ✅ 已实现 |

#### `inode_iget` 实现要点

1. Inode 区的起始扇区为 `INODE_START_SECTOR` (即 Block 2)
2. 每个 Block 有 `16` 个 Inode (`512 / 32`)
3. 第 `inumber` 号 inode 的**物理扇区** = `INODE_START_SECTOR + (inumber - 1) / 16`
4. 在扇区内的**字节偏移** = `((inumber - 1) % 16) × 32`
5. 先读取整个扇区，再 `memcpy` 出目标 Inode 的 32 字节

#### `inode_indexlookup` 实现要点 — 块地址映射

`i_addr[8]` 中的 8 个 `uint16_t` 是文件数据块的物理地址数组，使用**分级索引**：

```
逻辑块号 0 ~ 6  (小文件, blockNum < 7):
    i_addr[blockNum] → 直接存物理块号
    每个物理块 = 512 字节
    最大文件: 7 × 512 = 3,584 字节

逻辑块号 ≥ 7  (大文件, ILARG 标记):
    i_addr[7] → 指向一个"单重间接块"
    间接块 = 512 字节 = 256 个 uint16_t 物理块号
    i_addr[7] 的第 (blockNum - 7) 个条目 → 物理块号
    
    寻址范围: 256 个逻辑块 = 256 × 512 = 131,072 字节
    总寻址: (7 + 256) × 512 = 134,656 字节 (约 131 KB)
```

**图解**：

```
         i_addr[0]  ──────────→  [数据块 0]   (512B)
直接索引  i_addr[1]  ──────────→  [数据块 1]   (512B)
block    i_addr[2]  ──────────→  [数据块 2]   (512B)
0~6      i_addr[3]  ──────────→  [数据块 3]   (512B)
         i_addr[4]  ──────────→  [数据块 4]   (512B)
         i_addr[5]  ──────────→  [数据块 5]   (512B)
         i_addr[6]  ──────────→  [数据块 6]   (512B)

一级间接  i_addr[7]  ──→ ┌─────────────┐
block                   │ ptr[0]  ────→ [数据块 7]
                        │ ptr[1]  ────→ [数据块 8]
                        │ ptr[2]  ────→ [数据块 9]
                        │  ...         ...
                        │ ptr[254] ───→ [数据块 261]
                        │ ptr[255] ───→ [数据块 262]
                        └─────────────┘
                        (间接块: 256 × uint16_t)
```

---

### 4.6 `file.c/h` — 文件块读取层

**1 个函数，待实现：**

```c
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf);
```

**功能**：将文件第 `blockNum` 个逻辑块的内容读入 `buf`。

**实现步骤**：

1. 调用 `inode_iget(fs, inumber, &in)` 获取 inode
2. 调用 `inode_indexlookup(fs, &in, blockNum)` 获取物理块号
3. 调用 `diskimg_readsector(fs->dfd, physicalBlock, buf)` 读取数据
4. **计算返回值**：
   - 正常情况返回 `DISKIMG_SECTOR_SIZE` (512)
   - 如果是文件的最后一个 Block 且不满 512 字节，返回实际有效字节数
   - 公式：`min(DISKIMG_SECTOR_SIZE, fileSize - blockNum * 512)`
   - 出错返回 `-1`

---

### 4.7 `direntv6.h` — 目录项结构

```c
struct direntv6 {
    uint16_t  d_inumber;     //  2B: 文件/子目录的 inode 号
    char      d_name[14];    //  14B: 文件名 (最长 14 字符, \0 结尾)
};
// 总计 16 字节，每个 512B Block 可容纳 32 个目录项
```

---

### 4.8 `directory.c/h` — 目录查找层

**1 个函数，待实现：**

```c
int directory_findname(struct unixfilesystem *fs, const char *name,
                       int dirinumber, struct direntv6 *dirEnt);
```

**功能**：在 `dirinumber` 所标识的目录中，按文件名查找 `name`，结果写入 `dirEnt`。

**实现步骤**：

1. 调用 `inode_iget` 获取目录的 inode
2. 验证：`(i_mode & IALLOC) && ((i_mode & IFMT) == IFDIR)`
3. 遍历目录的所有逻辑块（循环调用 `file_getblock`）
4. 每个 Block 包含 `512 / 16 = 32` 个 `direntv6` 条目
5. 逐个比较 `dir[i].d_name` 是否与 `name` 匹配
6. 匹配成功 → 填充 `dirEnt`，返回 `0`；否则返回 `-1`
7. 跳过 `d_inumber == 0` 的空条目

---

### 4.9 `pathname.c/h` — 路径解析层

**1 个函数，待实现：**

```c
int pathname_lookup(struct unixfilesystem *fs, const char *pathname);
```

**功能**：给定绝对路径 (如 `/usr/bin/sh`)，返回目标文件的 inode 号。

**实现步骤**：

1. 当前目录 inode 号 = `ROOT_INUMBER` (1)
2. 逐级切分 `pathname` ("/usr/bin/sh" → "usr", "bin", "sh")
3. 对每一级调用 `directory_findname` 查找，得到下一级的 inode 号
4. 继续遍历直到路径结束
5. 返回最终 inode 号；任一步失败返回 `-1`

> 注：只需处理绝对路径。

---

### 4.10 `chksumfile.c/h` — 校验和层

对文件内容计算 **SHA1** 哈希，用于自动化测试的 Golden 比对。

| 函数 | 功能 | 状态 |
|---|---|------|
| `chksumfile_byinumber(fs, inumber, chksum)` | 按 inode 号计算校验和 | ✅ 已实现 |
| `chksumfile_bypathname(fs, pathname, chksum)` | 按路径计算校验和 | ✅ 已实现 |
| `chksumfile_cvt2string(chksum, outstring)` | 二进制校验和 → 十六进制字符串 | ✅ 已实现 |
| `chksumfile_compare(chksum1, chksum2)` | 比较两个校验和 | ✅ 已实现 |

**依赖链**：`chksumfile_byinumber` → `inode_iget` + `file_getblock` + `inode_getsize`

> **注意**：chksumfile 已写死 SHA1，但是你实现的 inode/file 读取如果出错，chksumfile 输出的结果就会不正确。

---

### 4.11 `diskimageaccess.c` — 主程序入口

程序的 main 函数，编译产出 `diskimageaccess` 可执行文件。

```
Usage: diskimageaccess <options> diskimagePath
  -q     安静模式 (不输出磁盘基本信息)
  -i     输出所有已分配 inode 的 SHA1 校验和
  -p     遍历目录树，输出所有路径的校验和
```

**执行流程**：

1. 解析命令行参数
2. `diskimg_open` → 打开磁盘镜像
3. `unixfilesystem_init` → 初始化文件系统（校验 bootblock + 读 superblock）
4. 根据 `-i` / `-p` 标志执行全盘扫描或目录树遍历
5. `diskimg_close` → 关闭磁盘镜像
6. 释放资源，退出

**`-i` 模式** (`DumpInodeChecksum`)：遍历 inumber 1 → `s_isize × 16`，对每个已分配的 inode 计算 SHA1 并打印。

**`-p` 模式** (`DumpPathnameChecksum`)：从根目录 (`/`) 开始递归遍历整个目录树，对每个文件目录打印路径、inode、大小、SHA1。同时校验 `byinumber` 和 `bypathname` 两种方式的结果一致性。

---

## 5. 调用依赖关系汇总

```
diskimageaccess (main)
 │
 ├──▶ unixfilesystem_init(dfd)              ✅ 读 superblock, 初始化 fs
 │
 ├──▶ [ -i 模式 ] DumpInodeChecksum
 │    └──▶ chksumfile_byinumber(fs, inumber)
 │         ├──▶ inode_iget()               ❌ 待实现
 │         ├──▶ inode_getsize()            ✅ 已实现
 │         └──▶ file_getblock()            ❌ 待实现
 │              ├──▶ inode_indexlookup()   ❌ 待实现
 │              └──▶ diskimg_readsector()  ✅ 已实现
 │
 └──▶ [ -p 模式 ] DumpPathnameChecksum
      └──▶ DumpPathAndChildren("/")
           ├──▶ pathname_lookup()           ❌ 待实现
           │    └──▶ directory_findname()   ❌ 待实现
           │         ├──▶ inode_iget()      ❌ 同上
           │         └──▶ file_getblock()   ❌ 同上
           └──▶ chksumfile_byinumber()     ✅ 同上
```

---

## 6. 文件清单

| 文件 | 类型 | 说明 |
|---|---|---|
| `diskimg.c/h` | 已实现 | 物理磁盘 I/O (扇区读写) |
| `filsys.h` | 结构定义 | Superblock 结构体 |
| `ino.h` | 结构定义 | Inode 结构体 + mode 宏 |
| `unixfilesystem.c/h` | 已实现 | 文件系统上下文初始化 |
| `inode.c/h` | **待实现** | Inode 读取 + 间接块解析 |
| `file.c/h` | **待实现** | 文件块读取 |
| `direntv6.h` | 结构定义 | 目录项结构体 |
| `directory.c/h` | **待实现** | 目录内按名查找 |
| `pathname.c/h` | **待实现** | 绝对路径 → inode 号 |
| `chksumfile.c/h` | 已实现 | SHA1 校验和计算 |
| `diskimageaccess.c` | 已实现 | main 入口程序 |
| `Makefile` | 构建 | 编译规则 |
| `slink/` | 测试 | 参考答案 + 测试磁盘 + 脚本 |

---

## 7. 推荐实现顺序

由于模块之间存在严格的**垂直依赖关系**，应按以下顺序逐个实现并测试：

```
第 1 步: inode_iget()           ← 最底层新功能, 从磁盘读 inode
第 2 步: inode_indexlookup()    ← 依赖 inode 结构体, 解析 i_addr[]
第 3 步: file_getblock()        ← 依赖 1 和 2, 组装完整块读取流程
第 4 步: directory_findname()   ← 依赖 3, 遍历目录块匹配文件名
第 5 步: pathname_lookup()      ← 依赖 4, 逐级路径解析
```

完成全部 5 步后，`chksumfile` + `diskimageaccess` 即可正常工作，可用 `sanity.py` 或直接对比 `slink/diskimageaccess_soln` 验证正确性。

---

## 8. 关键数值速查

| 常量 / 公式 | 值 |
|---|---|
| 扇区大小 (`DISKIMG_SECTOR_SIZE`) | `512` 字节 |
| Inode 大小 (`sizeof(struct inode)`) | `32` 字节 |
| 每扇区 Inode 数 | `512 / 32 = 16` |
| 目录项大小 (`sizeof(struct direntv6)`) | `16` 字节 |
| 每扇区目录项数 | `512 / 16 = 32` |
| Superblock 大小 (`sizeof(struct filsys)`) | `512` 字节 |
| 间接块指针数 | `256` (512 / 2) |
| 最大文件名长度 | `14` 字符 |
| Inode 区起始扇区 | `2` |
| 根目录 inumber | `1` |
| 第 `n` 号 inode 所在扇区 | `2 + (n - 1) / 16` |
| 第 `n` 号 inode 扇区内偏移 | `((n - 1) % 16) × 32` |
