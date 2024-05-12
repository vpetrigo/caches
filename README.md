[![CI](https://github.com/vpetrigo/caches/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/vpetrigo/caches/actions/workflows/ci.yml)
[![Build status](https://ci.appveyor.com/api/projects/status/kawd812e48065r7a?svg=true)](https://ci.appveyor.com/project/vpetrigo/caches)
[![codecov](https://codecov.io/gh/vpetrigo/caches/branch/master/graph/badge.svg?token=uExJPtyE0o)](https://codecov.io/gh/vpetrigo/caches)
[![version](https://img.shields.io/github/v/release/vpetrigo/caches)](https://github.com/vpetrigo/caches/releases)

# C++ Cache implementation

This project implements a simple thread-safe cache with several page replacement policies:

* Least Recently Used
* First-In/First-Out
* Least Frequently Used

More about cache algorithms and policy you could read on [Wikipedia](https://en.wikipedia.org/wiki/Cache_algorithms)

# Usage

Using this library is simple. It is necessary to include header with the cache implementation (`cache.hpp` file)
and appropriate header with the cache policy if it is needed. If not then the non-special algorithm will be used (it
removes the last element which key is the last in the internal container).

Currently, there is only three of them:

* `fifo_cache_policy.hpp`
* `lfu_cache_policy.hpp`
* `lru_cache_policy.hpp`

Example for the LRU policy:

```cpp
#include <string>
#include "cache.hpp"
#include "lru_cache_policy.hpp"

// alias for an easy class typing
template <typename Key, typename Value>
using lru_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::LRUCachePolicy>;

void foo() {
  constexpr std::size_t CACHE_SIZE = 256;
  lru_cache_t<std::string, int> cache(CACHE_SIZE);

  cache.Put("Hello", 1);
  cache.Put("world", 2);
  
  const auto hello_value = cache.Get("Hello");
  const auto world_value = cache.Get("world");

  std::cout << *hello_value << *world_value << '\n';
  // "12"
}
```

## Custom hashmap usage

You can use a custom hashmap implementation for the `caches::fixed_sized_cache` class which has the same interface
`std::unordered_map` has.

For example, you can declare LRU cache type like that:

```cpp
template <typename Key, typename Value>
using lru_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::LRUCachePolicy,
                                                       phmap::node_hash_map<Key, Value>>;
// ...
lru_cache_t<std::string, std::size_t> cache{16};
cache.Put("Hello", 1);
std::cout << *cache.Get("Hello") << '\n';
```

See `test` implementation which uses [`parallel-hashmap`](https://github.com/greg7mdp/parallel-hashmap).

# Requirements

The only requirement is a compatible C++11 compiler.

This project was tested in the environments listed below:

* MinGW64 ([MSYS2 project](https://msys2.github.io/))
    * Clang 13.0+
    * GCC 7+
* MSVC (VS 2015)

If you have any issues with the library building, please let me know.

# Contributing

Please fork this repository and contribute back using [pull requests](https://github.com/vpetrigo/caches/pulls).
Features can be requested using [issues](https://github.com/vpetrigo/caches/issues). All code, comments, and critiques
are greatly appreciated.
