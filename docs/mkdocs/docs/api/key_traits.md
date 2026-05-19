# Key traits

`caches::key_traits<Key>` controls hashing and equality.

By default the library will use:

- `std::hash<Key>` if it exists
- otherwise, an ADL-discoverable `hash_value(const Key&)`

## ADL hash_value example

```cpp
struct MyKey { int id; };

std::size_t hash_value(const MyKey& k) { return std::hash<int>{}(k.id); }
bool operator==(const MyKey& a, const MyKey& b) { return a.id == b.id; }

caches::cache<MyKey, int> c{100};
```

## Explicit functors

Use `caches::make_traits<Hash, Equal>`:

```cpp
using my_traits = caches::make_traits<MyHash, MyEqual>;
caches::cache<MyKey, int, caches::LRU, my_traits> c{100};
```

Full reference:

- [Doxygen API](../doxygen/index.html)
