# caches::PolicyInterface

`caches::PolicyInterface<Key>` is the abstract interface implemented by eviction policies.

Policies are responsible for tracking key access/insert/erase events and selecting the next replacement candidate.

See also:

- `api/policy/lru.md`
- `api/policy/lfu.md`
- `api/policy/fifo.md`
- `api/policy/no_eviction.md`
- [Doxygen API](../../doxygen/index.html)
