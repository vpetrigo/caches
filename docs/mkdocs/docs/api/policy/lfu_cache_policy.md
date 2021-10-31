---
title: caches::LFUCachePolicy
summary: LFU (Least frequently used) cache policy. 

---

# caches::LFUCachePolicy



LFU (Least frequently used) cache policy.  [More...](#detailed-description)


`#include <lfu_cache_policy.hpp>`

Inherits from [caches::ICachePolicy< Key >](/caches/api/policy/lfu_cache_policy/cache_policy_interface/)

## Public Types

|                | Name           |
| -------------- | -------------- |
| using typename std::multimap< std::size_t, Key >::iterator | **[lfu_iterator](/caches/api/policy/lfu_cache_policy/#using-lfu-iterator)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[LFUCachePolicy](/caches/api/policy/lfu_cache_policy/#function-lfucachepolicy)**() =default |
| | **[~LFUCachePolicy](/caches/api/policy/lfu_cache_policy/#function-~lfucachepolicy)**() override =default |
| virtual void | **[Insert](/caches/api/policy/lfu_cache_policy/#function-insert)**(const Key & key) override<br>Handle element insertion in a cache.  |
| virtual void | **[Touch](/caches/api/policy/lfu_cache_policy/#function-touch)**(const Key & key) override<br>Handle request to the key-element in a cache.  |
| virtual void | **[Erase](/caches/api/policy/lfu_cache_policy/#function-erase)**(const Key & key) override<br>Handle element deletion from a cache.  |
| virtual const Key & | **[ReplCandidate](/caches/api/policy/lfu_cache_policy/#function-replcandidate)**() const override<br>Return a key of a replacement candidate.  |

## Additional inherited members

**Public Functions inherited from [caches::ICachePolicy< Key >](/caches/api/policy/cache_policy_interface/)**

|                | Name           |
| -------------- | -------------- |
| virtual | **[~ICachePolicy](/caches/api/policy/cache_policy_interface/#function-~icachepolicy)**() =default |


## Detailed Description

```cpp
template <typename Key >
class caches::LFUCachePolicy;
```

LFU (Least frequently used) cache policy. 

**Template Parameters**: 

  * **Key** Type of a key a policy works with 


LFU policy in the case of replacement removes the least frequently used element.

Each access to an element in the cache increments internal counter (frequency) that represents how many times that particular key has been accessed by someone. When a replacement has to occur the LFU policy just takes a look onto keys' frequencies and remove the least used one. E.g. cache of two elements where `A` has been accessed 10 times and `B` – only 2. When you want to add a key `C` the LFU policy returns `B` as a replacement candidate. 

## Public Types Documentation

### using lfu_iterator

```cpp
using caches::LFUCachePolicy< Key >::lfu_iterator =  typename std::multimap<std::size_t, Key>::iterator;
```


## Public Functions Documentation

### function LFUCachePolicy

```cpp
LFUCachePolicy() =default
```


### function ~LFUCachePolicy

```cpp
~LFUCachePolicy() override =default
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
