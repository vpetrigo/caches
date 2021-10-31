---
title: caches::LRUCachePolicy
summary: LRU (Least Recently Used) cache policy. 

---

# caches::LRUCachePolicy



LRU (Least Recently Used) cache policy.  [More...](#detailed-description)


`#include <lru_cache_policy.hpp>`

Inherits from [caches::ICachePolicy< Key >](/caches/api/policy/cache_policy_interface/)

## Public Types

|                | Name           |
| -------------- | -------------- |
| using typename std::list< Key >::iterator | **[lru_iterator](/caches/api/policy/lru_cache_policy/#using-lru-iterator)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[LRUCachePolicy](/caches/api/policy/lru_cache_policy/#function-lrucachepolicy)**() =default |
| | **[~LRUCachePolicy](/caches/api/policy/lru_cache_policy/#function-~lrucachepolicy)**() =default |
| virtual void | **[Insert](/caches/api/policy/lru_cache_policy/#function-insert)**(const Key & key) override<br>Handle element insertion in a cache.  |
| virtual void | **[Touch](/caches/api/policy/lru_cache_policy/#function-touch)**(const Key & key) override<br>Handle request to the key-element in a cache.  |
| virtual void | **[Erase](/caches/api/policy/lru_cache_policy/#function-erase)**(const Key & key) override<br>Handle element deletion from a cache.  |
| virtual const Key & | **[ReplCandidate](/caches/api/policy/lru_cache_policy/#function-replcandidate)**() const override<br>Return a key of a replacement candidate.  |

## Additional inherited members

**Public Functions inherited from [caches::ICachePolicy< Key >](/caches/api/policy/cache_policy_interface/)**

|                | Name           |
| -------------- | -------------- |
| virtual | **[~ICachePolicy](/caches/api/policy/cache_policy_interface/#function-~icachepolicy)**() =default |


## Detailed Description

```cpp
template <typename Key >
class caches::LRUCachePolicy;
```

LRU (Least Recently Used) cache policy. 

**Template Parameters**: 

  * **Key** Type of a key a policy works with 


LRU policy in the case of replacement removes the least recently used element. That is, in the case of replacement necessity, that cache policy returns a key that has not been touched recently. For example, cache maximum size is 3 and 3 elements have been added - `A`, `B`, `C`. Then the following actions were made: 

```cpp
Cache placement order: A, B, C
Cache elements: A, B, C
# Cache access:
- A touched, B touched
# LRU element in the cache: C
# Cache access:
- B touched, C touched
# LRU element in the cache: A
# Put new element: D
# LRU replacement candidate: A

Cache elements: B, C, D
```

## Public Types Documentation

### using lru_iterator

```cpp
using caches::LRUCachePolicy< Key >::lru_iterator =  typename std::list<Key>::iterator;
```


## Public Functions Documentation

### function LRUCachePolicy

```cpp
LRUCachePolicy() =default
```


### function ~LRUCachePolicy

```cpp
~LRUCachePolicy() =default
```


### function Insert

```cpp
inline virtual void Insert(
    const Key & key
) override
```

Handle element insertion in a cache. 

**Parameters**: 

  * **key** Key that should be used by the policy 


**Reimplements**: [caches::ICachePolicy::Insert](/caches/api/policy/cache_policy_interface/#function-insert)


### function Touch

```cpp
inline virtual void Touch(
    const Key & key
) override
```

Handle request to the key-element in a cache. 

**Parameters**: 

  * **key** 


**Reimplements**: [caches::ICachePolicy::Touch](/caches/api/policy/cache_policy_interface/#function-touch)


### function Erase

```cpp
inline virtual void Erase(
    const Key & key
) override
```

Handle element deletion from a cache. 

**Parameters**: 

  * **key** Key that should be used by the policy 


**Reimplements**: [caches::ICachePolicy::Erase](/caches/api/policy/cache_policy_interface/#function-erase)


### function ReplCandidate

```cpp
inline virtual const Key & ReplCandidate() const override
```

Return a key of a replacement candidate. 

**Return**: Replacement candidate according to selected policy 

**Reimplements**: [caches::ICachePolicy::ReplCandidate](/caches/api/policy/cache_policy_interface/#function-replcandidate)
