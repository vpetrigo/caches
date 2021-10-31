---
title: caches::NoCachePolicy
summary: Basic no caching policy class. 

---

# caches::NoCachePolicy



Basic no caching policy class.  [More...](#detailed-description)


`#include <cache_policy.hpp>`

Inherits from [caches::ICachePolicy< Key >](/caches/api/policy/cache_policy_interface/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[NoCachePolicy](/caches/api/policy/no_cache_policy/#function-nocachepolicy)**() =default |
| | **[~NoCachePolicy](/caches/api/policy/no_cache_policy/#function-~nocachepolicy)**() override =default |
| virtual void | **[Insert](/caches/api/policy/no_cache_policy/#function-insert)**(const Key & key) override<br>Handle element insertion in a cache.  |
| virtual void | **[Touch](/caches/api/policy/no_cache_policy/#function-touch)**(const Key & key) override<br>Handle request to the key-element in a cache.  |
| virtual void | **[Erase](/caches/api/policy/no_cache_policy/#function-erase)**(const Key & key) override<br>Handle element deletion from a cache.  |
| virtual const Key & | **[ReplCandidate](/caches/api/policy/no_cache_policy/#function-replcandidate)**() const override<br>Return a key of a replacement candidate.  |

## Additional inherited members

**Public Functions inherited from [caches::ICachePolicy< Key >](/caches/api/policy/cache_policy_interface/)**

|                | Name           |
| -------------- | -------------- |
| virtual | **[~ICachePolicy](/caches/api/policy/cache_policy_interface/#function-~icachepolicy)**() =default |


## Detailed Description

```cpp
template <typename Key >
class caches::NoCachePolicy;
```

Basic no caching policy class. 

**Template Parameters**: 

  * **Key** Type of a key a policy works with 


Preserve any key provided. Erase procedure can get rid of any added keys without specific rules: a replacement candidate will be the first element in the underlying container. As unordered container can be used in the implementation there are no warranties that the first/last added key will be erased 

## Public Functions Documentation

### function NoCachePolicy

```cpp
NoCachePolicy() =default
```


### function ~NoCachePolicy

```cpp
~NoCachePolicy() override =default
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
