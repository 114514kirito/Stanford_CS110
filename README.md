# Stanford_CS110

[Course website](https://web.stanford.edu/class/cs110/)

Get from [this Repo](https://github.com/sauronalexander/cs110-1), and extract the starter code.

- [x] Assignment 1

- [x] Assignment 2
- [ ] Assignment 3
- [ ] Assignment 4
- [ ] Assignment 5
- [ ] Assignment 6
- [ ] Assignment 7
- [ ] Assignment 8

## Promblem in Assignment 1

When I tried to run imdbtest_soln, it can not find data directory. 

The directory is defined in the `imdb-utils.h`. 

```c++
const std::string kIMDBDataDirectory("/usr/class/cs110/samples/assign1/");
```

Just change it to your own working directory，Everything will be fine.
