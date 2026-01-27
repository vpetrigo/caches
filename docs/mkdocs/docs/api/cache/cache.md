# caches::cache

`caches::cache<Key, Value, Policy, KeyTraits, WrapperPolicy, HashMap>` is a fixed-size, thread-safe cache.

Most users should include the convenience header:

```cpp
#include <caches/caches.hpp>
```

## Basic usage

```cpp
caches::cache<std::string, int> c{256};
c.Put("Hello", 1);

auto v = c.Get("Hello");
// v is a wrapper (default: std::shared_ptr<int>)
```

## Template parameters

- `Policy` is a template template parameter like `caches::LRU`, `caches::LFU`, `caches::FIFO`, `caches::NoEviction`.
- `KeyTraits` controls hashing and equality (see `api/key_traits.md`).
- `WrapperPolicy` controls the stored value wrapper type (see `api/wrapper_policy.md`).
- `HashMap` lets you plug in an `unordered_map`-compatible map type.

## Reference

For the full API (constructors, `Put`, `Get`, `TryGet`, iteration, etc.), use:

- [Doxygen API](../../doxygen/index.html)
