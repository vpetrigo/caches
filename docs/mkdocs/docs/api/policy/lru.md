# caches::LRU

Least-recently-used policy.

Use it by passing `caches::LRU` as the policy template:

```cpp
caches::cache<std::string, int, caches::LRU> c{256};
```

Full reference:

- [Doxygen API](../../doxygen/index.html)
