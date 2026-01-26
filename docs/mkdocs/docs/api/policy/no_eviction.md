# caches::NoEviction

No-eviction policy.

This policy never evicts existing entries. When the cache is full, inserts will fail.

```cpp
caches::cache<std::string, int, caches::NoEviction> c{256};
```

Full reference:

- [Doxygen API](../../doxygen/index.html)
