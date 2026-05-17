# CS110 Assignment 1: Six Degrees of Kevin Bacon — 中英对照

> **说明：** 黑色英文原文，引用块内为中文翻译。关键术语如 binary search, BFS, mmap, lower_bound, STL, vector, list, set, valgrind, C-string 等保留原文。

---

## Assignment 1: Six Degrees of Kevin Bacon
### 作业一：Kevin Bacon 的六度分隔

Craving a little Oscar trivia? Try your hand in an Internet parlor game about Kevin Bacon's acting career. He's never been nominated for an Oscar, but he's certainly achieved immortality—based on the premise that he is the hub of the entertainment universe. Mike Ginelli, Craig Fass and Brian Turtle invented the game while students at Albright College in 1993, and their Bacon bit spread rapidly after convincing then TV talk-show host Jon Stewart to demonstrate the game to all those who tuned in. From these humble beginnings, a website was built, a book was published and a nationwide cult-fad was born.

> 想来点 Oscar 冷知识吗？试试这个关于 Kevin Bacon 演艺生涯的互联网客厅游戏。他从未获得 Oscar 提名，但无疑已经获得了不朽——基于他是娱乐宇宙中心这一假设。Mike Ginelli、Craig Fass 和 Brian Turtle 于 1993 年在 Albright College 读书时发明了这个游戏。在说服当时的电视脱口秀主持人 Jon Stewart 向所有观众演示这个游戏后，他们的 Bacon 游戏迅速传播开来。从这些不起眼的开始，一个网站被建立，一本书被出版，一个全国性的狂热潮流由此诞生。

When you think about Hollywood heavyweights, you don't immediately think of Kevin Bacon. But his career spans almost 20 years through films such as Flatliners, The Air Up There, Footloose, The River Wild, JFK and Animal House. So brush up on your Bacon lore. To play an Internet version, visit http://oracleofbacon.org.

> 当你想到了好莱坞的重量级人物时，你不会立刻想到 Kevin Bacon。但他的职业生涯跨越了近 20 年，出演了 Flatliners、The Air Up There、Footloose、The River Wild、JFK 和 Animal House 等电影。所以好好补习一下你的 Bacon 知识吧。想玩网络版，请访问 http://oracleofbacon.org。

This assignment is first and foremost a low-level systems programming assignment, but it's also an opportunity to review your C++ while simultaneously exercising your software engineering and low-level memory manipulation skills. You'll also get to see that low-level C coding and high-level C++ data structuring can coexist in the same application.

> 本次作业首先是一个底层系统编程作业，但同时也是一次复习 C++、锻炼软件工程和底层内存操作技能的机会。你还会看到底层 C 编码和高层 C++ 数据结构可以在同一应用中并存。

---

## Overview
### 概述

There are two major components to this assignment:

> 本次作业有两个主要组成部分：

**1.** You need to provide the implementation for an **imdb** class, which allows you to determine who appeared in what. We could layer our imdb class over two STL maps—one mapping people to movies and another mapping movies to people—but that would require we read in several megabytes of data from flat text files. That type of configuration takes several minutes, even on fast machines, and it's the opposite of fun if you have to sit that long before you play. Instead, you'll tap your sophisticated understanding of data representation and learn about something called **memory mapping** in order to look up movie and actor information from a prepared data structure that's been saved to disk in its binary form (and I'll describe the binary image format in the pages to come). This is the meatier part of the assignment. (By the way, imdb is short for Internet Movie Database. Our name is a gesture to the company that provides all of the data for the hundreds of thousands of movies and movie stars.)

> **1.** 你需要实现一个 **imdb** 类，用于确定谁出演了哪部电影。我们可以把 imdb 类层叠在两个 STL map 之上——一个将人映射到电影，另一个将电影映射到人——但这需要从纯文本文件中读入数 MB 的数据。这种初始化方式即使在快速机器上也需要几分钟时间，而在开始玩之前等那么久一点也不好玩。相反，你将利用你对数据表示的深入理解，学习一种叫做 **memory mapping（内存映射）** 的技术，从已保存在磁盘上的二进制形式数据中查找电影和演员信息（我将在后面的页面描述二进制镜像的格式）。这是本次作业中更有分量的部分。（顺便说一下，imdb 是 Internet Movie Database 的缩写，我们的命名是对那家为数十万部电影和影星提供数据的公司致敬。）

**2.** You also need to implement a **breadth-first search** algorithm that consults your super-clever imdb class to find the shortest path connecting any two actor/actresses. If the search goes on for so long that you can tell it'll be of length 7 or more, then you can be reasonably confident (and pretend that you know for sure that) there's no path connecting them. This part of the assignment is more CS106B-like, and it's a chance to get a little more experience with the STL (using vectors, sets, and lists) and to see a legitimate scenario where a complex program benefits from coding in two different paradigms: high-level, object-oriented C++ (with its STL template containers and template algorithms) and low-level, imperative C (with its exposed memory, brought to you by CS107, `*`, `&`, `[]`, and `->`).

> **2.** 你还需要实现一个 **breadth-first search（广度优先搜索）** 算法，利用你精心实现的 imdb 类来查找连接任意两位演员的最短路径。如果搜索进行到你可以判断路径长度将超过 6（即 7 步及以上），那么你可以合理地确信（并假装肯定）他们之间没有连接路径。这部分作业更像 CS106B 的风格，是一个让你更多接触 STL（使用 vector、set 和 list）的机会，也是一个让你看到在复杂程序中两种不同编程范式如何共存的实际场景：高层、面向对象的 C++（及其 STL 模板容器和模板算法）与底层、命令式的 C（及其暴露的内存操作，来自 CS107 的 `*`, `&`, `[]`, `->`）。

---

## Task I: The imdb class
### 任务一：imdb 类

First off, I want you to complete the implementation of the imdb class. Here's the reduced interface:

> 首先，你需要完成 imdb 类的实现。以下是精简后的接口：

```cpp
struct film {
    string title;
    int year;
};

class imdb {
public:
    imdb(const string& directory);
    bool good() const;
    bool getCredits(const string& player, vector<film>& films) const;
    bool getCast(const film& movie, vector<string>& players) const;
    ~imdb();

private:
    const void *actorFile;
    const void *movieFile;
};
```

The constructor and destructor have already been implemented for you. All the constructor does is initialize `actorFile` and `movieFile` fields to point to on-disk data structures using the **mmap** routine you'll learn about later on in the course. Since you haven't used mmap before, I implemented the constructor and destructor for you. (The destructor just **munmap**s what was mmap-ed at construction time).

> 构造函数和析构函数已经为你实现好了。构造函数所做的就是初始化 `actorFile` 和 `movieFile` 字段，使用 **mmap** 例程将它们指向磁盘上的数据结构（你将在课程后部分学到 mmap）。由于你之前没有使用过 mmap，所以我替你实现了构造函数和析构函数。（析构函数只是将在构造时 mmap 的内容 munmap 掉。）

You'll need to implement the `getCredits` and `getCast` methods by manually crawling over these binary images in order to produce vectors of movies and actor names. When properly implemented, they provide lightning-speed access to a gargantuan amount of information, because the information is already compactly formatted in a preprepared data structure that permanently lives on the myth machines.

> 你需要通过手动遍历这些二进制镜像来实现 `getCredits` 和 `getCast` 方法，以产生电影和演员名称的 vector。正确实现后，它们将提供闪电般快速地访问海量信息，因为这些信息已经以紧凑格式存储在一个预先准备好的、永久驻留在 myth 机器上的数据结构中。

Understand up front that you are implementing these two methods to crawl over two arrays of bytes in order to synthesize data structures for the client. What appears below is a description of how that memory is laid out. You aren't responsible for creating the data files in any way, but you are just responsible for understanding how everything is encoded so that you can rehydrate information from their byte-level representations.

> 请从一开始就理解：你实现这两个方法是为了遍历两个字节数组，以便为客户端合成数据结构。下面是描述该内存布局的内容。你不需要以任何方式创建数据文件，只需要理解一切是如何编码的，以便能从其字节级别的表示中还原（rehydrate）信息。

---

## The Raw Data Files
### 原始数据文件

The private `actorFile` and `movieFile` fields each address gigantic blocks of memory. Each is configured to point to mutually referent database images, and the format of each is described below. The imdb constructor sets these pointers up for you, so you can proceed as if everything is initialized for `getCast` and `getCredits` to just work.

> 私有字段 `actorFile` 和 `movieFile` 各自指向巨大的内存块。每个都被配置为指向相互引用的数据库镜像，各自的格式如下所述。imdb 的构造函数已经为你设置好了这些指针，因此你可以假定一切已经初始化好，`getCast` 和 `getCredits` 可以直接使用。

For the purposes of illustration, let's assume that Hollywood has produced a mere three movies, and that they've always rotated through the same three actors whenever the time came to cast their three films. Let's pretend those three films are as follows:

- **Clerks**, released in 1993, starring Cher and Liberace.
- **Moonstruck**, released in 1988, starring Cher, Liberace, and Madonna.
- **Zoolander**, released in 1999, starring Liberace and Madonna.

Remember, we're pretending.

> 为了便于说明，假设好莱坞只制作了三部电影，并且每当轮到选角这三部电影时，他们总是轮换使用相同的三位演员。假设这三部电影如下：
>
> - **Clerks**，1993 年上映，由 Cher 和 Liberace 主演。
> - **Moonstruck**，1988 年上映，由 Cher、Liberace 和 Madonna 主演。
> - **Zoolander**，1999 年上映，由 Liberace 和 Madonna 主演。
>
> 记住，我们只是在假设。

Each of the records for the actors and movies will be of variable size. Some movie titles are longer than others; some films feature 75 actors, while others star only one or two. Some people have prolific careers, while several people are one-hit wonders. Defining a struct or class to overlay the blocks of data is a fine idea, except that doing so would constrain all records to be the same size. We don't want that, because we'd be wasting a good chunk of memory when storing information on actors who appeared in just one or two films (and for films that feature just a handful of actors).

> 每个演员和电影的记录都是可变大小的。有些电影标题更长；有些电影有 75 位演员，而其他电影只有一两位。有些人事业多产，而有些人则是一闪而过的奇迹。定义一个 struct 或 class 来覆盖数据块是个好主意，只是这样做会强制所有记录大小相同。我们不希望这样，因为这对于只出演了一两部电影的演员（以及只有少数几位演员的电影）来说会浪费大量内存。

However, by allowing the individual records to be of variable size, we lose our ability to **binary search** (hint: via the STL `lower_bound` algorithm) a sorted array of records. The number of actors and actresses is circa 1.6 million, and the number of movies is in the hundreds of thousands, so a linear search would be all turtle-like. All of the actors and movies are sorted by name (and then by year if two movies have the same name), so binary search is still within reach. The strong desire to search quickly motivated my decision to format the data files like this:

> 然而，允许单个记录可变大小后，我们就无法对排序的记录数组进行 **binary search（二分查找）**（提示：通过 STL 的 `lower_bound` 算法）。演员数量约为 160 万，电影数量达数十万级，因此 linear search（线性查找）会像乌龟一样缓慢。所有演员和电影都是按名称排序的（如果两部电影同名，则按年份排序），所以 binary search 仍然可行。对快速搜索的强烈需求促使我决定将数据文件格式化为如下形式：

Spliced in between the number of records and the records themselves is an array of integer offsets. They're drawn as pointers, but they really aren't stored that way. We want the data images to be **relocatable**—that is, we want the information stored in the data images pointed to by `actorFile` and `movieFile` to be useful, regardless of what addresses get stored there. By storing integer offsets, we can manually compute the location of Cher's record, Madonna's record, or Clerk's record, etc, by adding the corresponding offsets to whatever `actorFile` or `movieFile` turn out to be.

> 在记录数量和记录本身之间插入了一个整数偏移量（offset）数组。它们被画成指针的样子，但实际上并不是那样存储的。我们希望数据镜像是**可重定位的（relocatable）**——也就是说，我们希望存储在由 `actorFile` 和 `movieFile` 指向的数据镜像中的信息无论被加载到什么地址都能正常使用。通过存储整数偏移量，我们可以手动计算 Cher 的记录、Madonna 的记录或 Clerks 的记录等的位置，只需将相应的偏移量加到 `actorFile` 或 `movieFile` 的基地址上即可。

Because all of the offsets are stored as four-byte integers (and `int`s are four bytes, even on 64-bit systems like the myths), and because they are in a sense sorted if the records they reference are sorted, we can use binary search. Woo!

> 因为所有偏移量都以四字节整数存储（`int` 是四字节，即使在 myth 这样的 64 位系统上也是如此），而且如果它们引用的记录是排序的，那么它们在某种意义上也是排序的，我们就可以使用 binary search。棒！

**To summarize:**

> **总结：**

- `actorFile` points to a large mass of memory packing all of the information about all of the actors. The first four bytes store the number of actors (as an `int`); the next four bytes store the offset to the zeroth actor, the next four bytes store the offset to the first actor, and so forth. The last offset is followed by the zeroth record, then the first record, and so forth. The records themselves are sorted by name. Pinky swear they are.

> - `actorFile` 指向一大块内存，其中打包了所有演员的信息。前四个字节存储演员数量（以 `int` 形式）；接下来的四个字节存储第 0 个演员的偏移量，再接下来的四个字节存储第 1 个演员的偏移量，依此类推。最后一个偏移量之后是第 0 条记录，然后是第 1 条记录，以此类推。记录本身按名称排序。保证如此。

- `movieFile` also points to a large mass of memory, but this one packs the information about all films ever made. The first four bytes store the number of movies (again, as an `int`); the next `*(int *)movieFile * sizeof(int)` bytes store all of the int offsets, and everything beyond the offsets is real movie data. The movies are sorted by title, and those sharing the same title are sorted by year.

> - `movieFile` 也指向一大块内存，但这一块打包了所有电影的信息。前四个字节存储电影数量（同样以 `int` 形式）；接下来的 `*(int *)movieFile * sizeof(int)` 字节存储所有 int 偏移量，偏移量之后的所有内容都是真实的电影数据。电影按标题排序，标题相同的电影按年份排序。

- The above description generalizes to files with 1,600,000 actors and 440,000 movies.

> - 上述描述适用于包含 1,600,000 名演员和 440,000 部电影的文件。

---

## The Actor Record
### 演员记录

The actor record is a packed set of bytes collecting information about an actor and the movies he or she's appeared in. We don't use a struct or a class to overlay the memory associated with an actor, because doing so would constrain the record size to be constant for all actors. Instead, we lay out the relevant information in a series of bytes, the number of which depends on the length of the actor's name and the number of films he's appeared in. Here's what gets manually placed within each entry:

> 演员记录是一组紧凑排列的字节，收集了关于一位演员及其出演电影的信息。我们不使用 struct 或 class 来覆盖与演员关联的内存，因为这样做会强制所有演员的记录大小相同。相反，我们将相关信息以一系列字节的形式布局，字节数量取决于演员名字的长度和其出演的电影数量。以下是每条记录中手动放置的内容：

1. The name of the actor is laid out character by character, as a normal null-terminated **C-string**. If the length of the actor's name is even, then the string is padded with an extra `'\0'` so that the total number of bytes dedicated to the name is always an **even number**. The information that follows the name is most easily interpreted as a `short`, and the myths might constrain addresses manipulated as `short *`s to be even.

> 1. 演员的名字逐个字符排列，作为普通的以 null 结尾的 **C-string**。如果演员名字的长度为偶数，则字符串会用额外的 `'\0'` 填充，使得专用于名字的总字节数始终为**偶数**。名字后面的信息最容易解释为 `short` 类型，而 myth 机器可能要求以 `short*` 操作的地址为偶数对齐。

2. The number of movies in which the actor has appeared, expressed as a two-byte `short`. (Some people have been in more than 255 movies, so a single byte isn't always enough). If the number of bytes dedicated to the actor's name (always even) and the short (always 2) isn't a multiple of four, then two additional `'\0'`s appear after the two bytes storing the number of movies. This padding is conditionally done so that the four-byte integers that follow sit at addresses that are multiples of four (again, because the 64-bit myth's might be configured to require this).

> 2. 演员出演的电影数量，以两字节的 `short` 表示。（有些人出演了超过 255 部电影，所以一个字节不一定够用）。如果专用于演员名字的字节数（始终为偶数）加上 short（始终为 2）不是 4 的倍数，则在存储电影数量的两个字节之后再出现两个额外的 `'\0'`。这种填充是有条件地进行的，以便后面的四字节整数位于 4 的倍数的地址上（同样，因为 64 位 myth 机器可能配置为需要这样）。

3. An array of offsets into the `movieFile` image, where each offset identifies one of the actor's films.

> 3. 一组指向 `movieFile` 镜像的偏移量数组，每个偏移量标识演员出演的一部电影。

---

## The Movie Record
### 电影记录

The movie record is only slightly more complicated. The information is compressed as follows:

> 电影记录只是稍微复杂一些。信息压缩如下：

1. The title of the movie, terminated by a `'\0'`, so the character array behaves as a normal **C-string** incidentally wedged into a larger binary data figure.

> 1. 电影的标题，以 `'\0'` 结尾，因此字符数组表现为一个普通的 **C-string**，恰好嵌入在一个较大的二进制数据结构中。

2. The year the film was released, expressed as a single byte. This byte stores the year, minus 1900. Since Hollywood is less than 256 years old, it was fine to just store the year as an offset from 1900. If the total number of bytes used to encode the name and year of the movie is odd, then an extra `'\0'` sits in between the one-byte year and the data that follows.

> 2. 电影上映的年份，以单个字节表示。该字节存储的是年份减去 1900。由于好莱坞的历史不到 256 年，所以只需将年份存储为相对于 1900 的偏移量即可。如果用于编码电影名称和年份的总字节数是奇数，则在单字节年份和后续数据之间会多出一个 `'\0'`。

3. A two-byte `short` storing the number of actors appearing in the film, padded with two additional bytes of zeroes if needed.

> 3. 一个两字节的 `short`，存储电影中出演的演员数量，如果需要则用两个额外的零字节填充。

4. An array of four-byte integer offsets, where each integer offset identifies one of the actors accessible via `actorFile`. The number of offsets here is, of course, equal to the short read during step 3.

> 4. 一组四字节整数偏移量数组，每个整数偏移量标识一个可以通过 `actorFile` 访问的演员。这里的偏移量数量当然等于第 3 步中读到的 short 值。

**One major gotcha:** Some movies share the same title even though they are different. (The Manchurian Candidate, for instance, was first released in 1962, and then remade in 2004. They're two different films with two different casts.) If you look in the `imdb-utils.h` file, you'll see that the `film` struct provides `operator<` and `operator==` methods. That means that two films know how to compare themselves to each other using infix `==` and `<` (though not using `!=`, `>`, `>=`, or `<=`). You can just rely on the `<` and `==` to compare two film records. In fact, you have to, because the movies in the `movieData` binary image are sorted to respect `film::operator<`.

> **一个重要陷阱：** 有些电影虽然不同但共享相同的标题。（例如 The Manchurian Candidate 最初于 1962 年上映，然后在 2004 年重拍。它们是两部不同的电影，有不同的演员阵容。）如果你查看 `imdb-utils.h` 文件，你会看到 `film` struct 提供了 `operator<` 和 `operator==` 方法。这意味着两部电影知道如何使用中缀 `==` 和 `<` 来相互比较（但不能使用 `!=`, `>`, `>=` 或 `<=`）。你可以直接依赖 `<` 和 `==` 来比较两个 film 记录。事实上，你必须这样做，因为 `movieData` 二进制镜像中的电影是按照 `film::operator<` 排序的。

It's best to work on the implementation of the imdb class in isolation, not worrying about the details of the search algorithm you'll eventually need to write. I've provided a test harness to exercise the imdb all by itself, and that code sits in `imdbtest.cc`. The make system generates a test application called `imdbtest` which you can use to verify that your imdb implementation is solid. I provide my own version in `./slink/imdbtest_soln` (slink is a symbolic link in your repo to a shared directory with solution executables) so you can run your version and my version side by side and make sure they match character for character.

> 最好先独立完成 imdb 类的实现，暂时不要担心最终需要写的搜索算法的细节。我提供了一个测试框架来单独测试 imdb，代码在 `imdbtest.cc` 中。make 系统会生成一个名为 `imdbtest` 的测试程序，你可以用它来验证你的 imdb 实现是否正确。我在 `./slink/imdbtest_soln` 中提供了我的版本（slink 是你仓库中指向共享目录的符号链接，其中包含解决方案可执行文件），这样你可以并排运行你的版本和我的版本，确保它们的输出字符完全一致。

**Note:** Your implementation will be—and in fact is intended to be—an interesting mix of C and C++. You'll be relying on your mad C skillz to crawl over these binary images, and you'll be leveraging your C++ mastery to lift that data up into C++ objects. As part of your implementation, you'll need to binary search over the actor and movie offsets to find the actor or movie of interest.

> **注意：** 你的实现将是——实际上也应该是——C 和 C++ 的有趣混合。你将依靠你娴熟的 C 技巧来遍历这些二进制镜像，同时利用你的 C++ 水平将这些数据提升为 C++ 对象。作为实现的一部分，你需要对演员和电影的偏移量进行 binary search，以找到目标演员或电影。

I am requiring that you use the STL `lower_bound` algorithm to perform these binary searches, and that you use **C++11 lambdas** (also known as anonymous functions with capture clauses) to provide nameless comparison functions that `lower_bound` can use to guide its search.

> 我要求你使用 STL 的 `lower_bound` 算法来执行这些二分查找，并使用 **C++11 的 lambda 表达式**（也称为带有捕获子句的匿名函数）来提供无名比较函数，供 `lower_bound` 用于指导其搜索。

---

## Task II: Implementing Search
### 任务二：实现搜索

You're back in pure C++ mode. At this point, I'm assuming your imdb class just works, and the fact that there's some spectacularly shrewd pointer gymnastics going on in the `imdb.cc` file is completely disguised by the delightfully simple imdb interface. Use the services of your imdb and my `path` class (discussed below) to implement a **breadth-first search** for the shortest possible path. Leverage the STL containers as much as possible to get this done. Here are the STL classes I used in my solution:

> 现在回到纯 C++ 模式。此时，我假设你的 imdb 类已经可以正常工作，而 `imdb.cc` 文件中那些极其精妙的指针操作完全被简洁优雅的 imdb 接口所隐藏。利用你的 imdb 和我的 `path` 类（稍后讨论）的服务来实现 **breadth-first search**，找到可能的最短路径。尽可能利用 STL 容器来完成这项工作。以下是我在解决方案中使用的 STL 类：

- **vector:** there's no escaping this one, because the imdb requires we pull films and actors out of the binary images as vectors.

> - **vector：** 这个逃不掉，因为 imdb 要求我们从二进制镜像中以 vector 形式提取电影和演员。

- **list:** The list is a doubly-linked list that provides O(1) `push_back`, `front`, and `pop_front` operations. There's also a `queue` template, and you can use that if you want, but I'm so bugged that the STL queue calls its methods `push` and `pop` instead of `enqueue` and `dequeue` that I boycotted and used the list instead.

> - **list：** list 是一个双向链表，提供 O(1) 的 `push_back`、`front` 和 `pop_front` 操作。还有一个 `queue` 模板，如果你愿意也可以使用，但 STL queue 将其方法命名为 `push` 和 `pop` 而不是 `enqueue` 和 `dequeue` 让我非常不爽，所以我抵制它并改用 list。

- **set:** I used two sets to keep track of previously used actors and films. If you're implementing a breadth-first search and you encounter a movie or actor that you've seen before, there's no reason to use it/him/her a second time. You shouldn't need to use anything other than `set::insert`.

> - **set：** 我使用了两个 set 来跟踪已经访问过的演员和电影。如果你正在实现 breadth-first search，遇到之前见过的电影或演员时，没有理由再次使用。你应该只需要使用 `set::insert` 就足够了。

---

## Assignment 1 Files
### 作业一文件清单

| 文件 | 说明 |
|------|------|
| **imdb-utils.h** | film struct 的定义，以及一个为你查找数据文件的 inline 函数。你不应该需要修改这个文件。 |
| **imdb.h** | imdb 类的接口。你不应该更改此文件的 public 接口，但如果合理，你可以自由修改 private 部分。 |
| **imdb.cc** | imdb 构造函数、析构函数和方法的实现。这是你的 `getCast` 和 `getCredits` 代码所在的位置。 |
| **imdbtest.cc** | 我们提供的单元测试代码，用于帮助你练习 imdb。你不应该需要修改这个文件。 |
| **Makefile** | 通过输入 `make imdbtest`，你将仅编译构建 imdbtest 所需的文件。你完全不需要修改这个文件。 |
| **search.cc** | 你的任务二代码变更应该主要（甚至全部）在的文件。 |
| **path.h** | path 类的定义，这是一个自定义类，用于构建两位演员之间的路径。如果认为合理，你可以自由添加方法。 |
| **path.cc** | path 类的实现。同样，如果认为合理，你可以在此添加内容。 |

Everything from Task I (except `imdbtest.cc`) contributes to the overall search application. Type `make search` to build the search executable without building the imdbtest application (or you can just type `make` and build both). There's a sample executable at `./slink/search_soln` for you to play with. Understand that my sample application and yours aren't obligated to publish the same exact shortest path, but you should be sure that the path lengths themselves actually match.

> 任务一的所有内容（`imdbtest.cc` 除外）都有助于构建完整的搜索应用程序。输入 `make search` 可以仅构建 search 可执行文件，不构建 imdbtest（或者你也可以直接输入 `make` 同时构建两者）。`./slink/search_soln` 中有一个示例可执行文件供你试用。请理解，我的示例程序和你的程序不必输出完全相同的精确最短路径，但你应该确保路径长度本身是匹配的。

---

*翻译时间：2026-05-11 | 格式：Markdown 中英对照 | 关键术语保留原文*
