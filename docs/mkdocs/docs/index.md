---
hide:
  - toc
---
# Cache library with LRU/LFU/FIFO policies support

## Base class

- Cache class: [caches::fixed_sized_cache](/caches/api/cache/fixed_sized_cache/)

## Cache policy classes

- Cache policy abstract class: [caches::ICachePolicy](/caches/api/policy/cache_policy_interface/)
- LRU cache policy class: [caches::LRUCachePolicy](/caches/api/policy/lru_cache_policy/)
- LFU cache policy class: [caches::LFUCachePolicy](/caches/api/policy/lfu_cache_policy/)
- FIFO cache policy class: [caches::FIFOCachePolicy](/caches/api/policy/fifo_cache_policy/)
- No policy cache class: [caches::NoCachePolicy](/caches/api/policy/no_cache_policy/)

## Useful links

- [Cache replacement policies](https://en.wikipedia.org/wiki/Cache_replacement_policies)
