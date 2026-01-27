[![CI](https://github.com/vpetrigo/caches/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/vpetrigo/caches/actions/workflows/ci.yml)
[![Build status](https://ci.appveyor.com/api/projects/status/kawd812e48065r7a?svg=true)](https://ci.appveyor.com/project/vpetrigo/caches)
[![codecov](https://codecov.io/gh/vpetrigo/caches/branch/master/graph/badge.svg?token=uExJPtyE0o)](https://codecov.io/gh/vpetrigo/caches)
[![version](https://img.shields.io/github/v/release/vpetrigo/caches)](https://github.com/vpetrigo/caches/releases)

# C++ Cache implementation

This project implements a simple thread-safe cache with several page replacement policies:

- Least Recently Used
- First-In/First-Out
- Least Frequently Used

More about cache algorithms and policy you could read on [Wikipedia](https://en.wikipedia.org/wiki/Cache_algorithms)

# Usage

Include the convenience header:

```cpp
#include <caches/caches.hpp>
```

By default, `caches::cache` uses an LRU eviction policy and wraps values in `std::shared_ptr`.

```cpp
#include <caches/caches.hpp>

#include <iostream>
#include <string>

int main() {
  caches::cache<std::string, int> c{256};
  c.Put("Hello", 1);
  c.Put("world", 2);

  auto hello = c.Get("Hello");
  auto world = c.Get("world");
  std::cout << *hello << *world << "\n";
}
```

## Policies

Select a policy by passing it as the third template parameter:

```cpp
caches::cache<int, std::string, caches::FIFO> fifo{50};
caches::cache<int, std::string, caches::LFU> lfu{50};
caches::cache<int, std::string, caches::LRU> lru{50};
caches::cache<int, std::string, caches::NoEviction> no_evict{50};
```

## Custom key hashing/equality

For custom key types you can either provide `std::hash<Key>` or define an ADL-discoverable `hash_value(const Key&)`.

```cpp
struct MyKey { int id; };

std::size_t hash_value(const MyKey& k) { return std::hash<int>{}(k.id); }
bool operator==(const MyKey& a, const MyKey& b) { return a.id == b.id; }

caches::cache<MyKey, int> c{100};
```

If you want to explicitly provide functors, use `caches::make_traits`:

```cpp
struct MyHash { std::size_t operator()(const MyKey&) const; };
struct MyEqual { bool operator()(const MyKey&, const MyKey&) const; };

using my_traits = caches::make_traits<MyHash, MyEqual>;
caches::cache<MyKey, int, caches::LRU, my_traits> c{100};
```

## Custom hashmap

You can override the underlying hashmap template as long as it is compatible with `std::unordered_map`'s template
parameters.

```cpp
template <typename K, typename V, typename H, typename E, typename A>
using my_hash_map = std::unordered_map<K, V, H, E, A>;

caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
              caches::wrapper_policy<int>, my_hash_map>
    c{100};
```

## Value wrapper policy

Values are wrapped to allow safe access even if an entry is evicted. By default this is `std::shared_ptr<Value>`.
You can customize it via the `WrapperPolicy` template parameter.

# Requirements

The only requirement is a compatible C++11 compiler.

This project was tested in the environments listed below:

- MinGW64 ([MSYS2 project](https://msys2.github.io/))
  - Clang 13.0+
  - GCC 7+
- MSVC (VS 2015)

If you have any issues with the library building, please let me know.

# Contributing

Please fork this repository and contribute back using [pull requests](https://github.com/vpetrigo/caches/pulls).
Features can be requested using [issues](https://github.com/vpetrigo/caches/issues). All code, comments, and critiques
are greatly appreciated.
