---
title: caches::FIFOCachePolicy
summary: FIFO (First in, first out) cache policy. 

---

# caches::FIFOCachePolicy



FIFO (First in, first out) cache policy.  [More...](#detailed-description)


`#include <fifo_cache_policy.hpp>`

Inherits from [caches::ICachePolicy< Key >](/caches/api/policy/cache_policy_interface/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[FIFOCachePolicy](/caches/api/policy/fifo_cache_policy/#function-fifocachepolicy)**() =default |
| | **[~FIFOCachePolicy](/caches/api/policy/fifo_cache_policy/#function-~fifocachepolicy)**() =default |
| virtual void | **[Insert](/caches/api/policy/fifo_cache_policy/#function-insert)**(const Key & key) override<br>Handle element insertion in a cache.  |
| virtual void | **[Touch](/caches/api/policy/fifo_cache_policy/#function-touch)**(const Key & key) override<br>Handle request to the key-element in a cache.  |
| virtual void | **[Erase](/caches/api/policy/fifo_cache_policy/#function-erase)**(const Key & key) override<br>Handle element deletion from a cache.  |
| virtual const Key & | **[ReplCandidate](/caches/api/policy/fifo_cache_policy/#function-replcandidate)**() const override<br>Return a key of a replacement candidate.  |

## Additional inherited members

**Public Functions inherited from [caches::ICachePolicy< Key >](/caches/api/policy/cache_policy_interface/)**

|                | Name           |
| -------------- | -------------- |
| virtual | **[~ICachePolicy](/caches/api/policy/cache_policy_interface/#function-~icachepolicy)**() =default |


## Detailed Description

```cpp
template <typename Key >
class caches::FIFOCachePolicy;
```

FIFO (First in, first out) cache policy. 

**Template Parameters**: 

  * **Key** Type of a key a policy works with 


FIFO policy in the case of replacement removes the first added element.

That is, consider the following key adding sequence: 

```cpp
A -> B -> C -> ...
```

In the case a cache reaches its capacity, the FIFO replacement candidate policy returns firstly added element `A`. To show that: 

```cpp
# New key: X
Initial state: A -> B -> C -> ...
Replacement candidate: A
Final state: B -> C -> ... -> X -> ...
```

An so on, the next candidate will be `B`, then `C`, etc. 

## Public Functions Documentation

### function FIFOCachePolicy

```cpp
FIFOCachePolicy() =default
```


### function ~FIFOCachePolicy

```cpp
~FIFOCachePolicy() =default
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
