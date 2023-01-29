---
title: caches::ICachePolicy
summary: Cache policy abstract base class. 

---

# caches::ICachePolicy



Cache policy abstract base class.  [More...](#detailed-description)


`#include <cache_policy.hpp>`

Inherited by [caches::FIFOCachePolicy< Key >](/caches/api/policy/fifo_cache_policy), [caches::LFUCachePolicy< Key >](/caches/api/policy/lfu_cache_policy), [caches::LRUCachePolicy< Key >](/caches/api/policy/lru_cache_policy), [caches::NoCachePolicy< Key >](/caches/api/policy/no_cache_policy)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual | **[~ICachePolicy](/caches/api/policy/cache_policy_interface/#function-~icachepolicy)**() =default |
| virtual void | **[Insert](/caches/api/policy/cache_policy_interface/#function-insert)**(const Key & key) =0<br>Handle element insertion in a cache.  |
| virtual void | **[Touch](/caches/api/policy/cache_policy_interface/#function-touch)**(const Key & key) =0<br>Handle request to the key-element in a cache.  |
| virtual void | **[Erase](/caches/api/policy/cache_policy_interface/#function-erase)**(const Key & key) =0<br>Handle element deletion from a cache.  |
| virtual const Key & | **[ReplCandidate](/caches/api/policy/cache_policy_interface/#function-replcandidate)**() const =0<br>Return a key of a replacement candidate.  |

## Detailed Description

```cpp
template <typename Key >
class caches::ICachePolicy;
```

Cache policy abstract base class. 

**Template Parameters**: 

  * **Key** Type of a key a policy works with 

## Public Functions Documentation

### function ~ICachePolicy

```cpp
virtual ~ICachePolicy() =default
```


### function Insert

```cpp
virtual void Insert(
    const Key & key
) =0
```

Handle element insertion in a cache. 

**Parameters**: 

  * **key** Key that should be used by the policy 


**Reimplemented by**: [caches::LFUCachePolicy::Insert](/caches/api/policy/lfu_cache_policy.html/#function-insert), [caches::FIFOCachePolicy::Insert](/caches/api/policy/fifo_cache_policy.html/#function-insert), [caches::LRUCachePolicy::Insert](/caches/api/policy/lru_cache_policy.html/#function-insert), [caches::NoCachePolicy::Insert](/caches/api/policy/no_cache_policy.html/#function-insert)


### function Touch

```cpp
virtual void Touch(
    const Key & key
) =0
```

Handle request to the key-element in a cache. 

**Parameters**: 

  * **key** 


**Reimplemented by**: [caches::FIFOCachePolicy::Touch](/caches/api/policy/fifo_cache_policy.html/#function-touch), [caches::NoCachePolicy::Touch](/caches/api/policy/no_cache_policy.html/#function-touch), [caches::LFUCachePolicy::Touch](/caches/api/policy/lfu_cache_policy.html/#function-touch), [caches::LRUCachePolicy::Touch](/caches/api/policy/lru_cache_policy.html/#function-touch)


### function Erase

```cpp
virtual void Erase(
    const Key & key
) =0
```

Handle element deletion from a cache. 

**Parameters**: 

  * **key** Key that should be used by the policy 


**Reimplemented by**: [caches::LRUCachePolicy::Erase](/caches/api/policy/lru_cache_policy.html/#function-erase), [caches::FIFOCachePolicy::Erase](/caches/api/policy/fifo_cache_policy.html/#function-erase), [caches::LFUCachePolicy::Erase](/caches/api/policy/lfu_cache_policy.html/#function-erase), [caches::NoCachePolicy::Erase](/caches/api/policy/no_cache_policy.html/#function-erase)


### function ReplCandidate

```cpp
virtual const Key & ReplCandidate() const =0
```

Return a key of a replacement candidate. 

**Return**: Replacement candidate according to selected policy 

**Reimplemented by**: [caches::FIFOCachePolicy::ReplCandidate](/caches/api/policy/fifo_cache_policy.html/#function-replcandidate), [caches::LFUCachePolicy::ReplCandidate](/caches/api/policy/lfu_cache_policy.html/#function-replcandidate), [caches::LRUCachePolicy::ReplCandidate](/caches/api/policy/lru_cache_policy.html/#function-replcandidate), [caches::NoCachePolicy::ReplCandidate](/caches/api/policy/no_cache_policy.html/#function-replcandidate)
