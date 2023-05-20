---
title: caches::fixed_sized_cache
summary: Fixed sized cache that can be used with different policy types (e.g. LRU, FIFO, LFU) 

---

# caches::fixed_sized_cache



Fixed sized cache that can be used with different policy types (e.g. LRU, FIFO, LFU)  [More...](#detailed-description)


`#include <cache.hpp>`

## Public Types

|                | Name           |
| -------------- | -------------- |
| using typename std::unordered_map< Key, Value >::iterator | **[iterator](/caches/api/cache/fixed_sized_cache/#using-iterator)**  |
| using typename std::unordered_map< Key, Value >::const_iterator | **[const_iterator](/caches/api/cache/fixed_sized_cache/#using-const-iterator)**  |
| using typename std::lock_guard< std::mutex > | **[operation_guard](/caches/api/cache/fixed_sized_cache/#using-operation-guard)**  |
| using typename std::function< void(const Key &key, const Value &value)> | **[on_erase_cb](/caches/api/cache/fixed_sized_cache/#using-callback)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[fixed_sized_cache](/caches/api/cache/fixed_sized_cache/#function-fixed-sized-cache)**(size_t max_size, const Policy< Key > policy =Policy< Key >{}, on_erase_cb OnErase =[](const Key &, const Value &) {})<br>Fixed sized cache constructor.  |
| | **[~fixed_sized_cache](/caches/api/cache/fixed_sized_cache/#function-~fixed-sized-cache)**() |
| void | **[Put](/caches/api/cache/fixed_sized_cache/#function-put)**(const Key & key, const Value & value)<br>Put element into the cache.  |
| std::pair< const_iterator, bool > | **[TryGet](/caches/api/cache/fixed_sized_cache/#function-tryget)**(const Key & key) const<br>Try to get an element by the given key from the cache.  |
| const Value & | **[Get](/caches/api/cache/fixed_sized_cache/#function-get)**(const Key & key) const<br>Get element from the cache if present.  |
| bool | **[Cached](/caches/api/cache/fixed_sized_cache/#function-cached)**(const Key & key) const<br>Check whether the given key is presented in the cache.  |
| std::size_t | **[Size](/caches/api/cache/fixed_sized_cache/#function-size)**() const<br>Get number of elements in cache.  |
| bool | **[Remove](/caches/api/cache/fixed_sized_cache/#function-remove)**(const Key & key) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| void | **[Clear](/caches/api/cache/fixed_sized_cache/#function-clear)**() |
| const_iterator | **[begin](/caches/api/cache/fixed_sized_cache/#function-begin)**() const |
| const_iterator | **[end](/caches/api/cache/fixed_sized_cache/#function-end)**() const |
| void | **[Insert](/caches/api/cache/fixed_sized_cache/#function-insert)**(const Key & key, const Value & value) |
| void | **[Erase](/caches/api/cache/fixed_sized_cache/#function-erase)**(const_iterator elem) |
| void | **[Erase](/caches/api/cache/fixed_sized_cache/#function-erase)**(const Key & key) |
| void | **[Update](/caches/api/cache/fixed_sized_cache/#function-update)**(const Key & key, const Value & value) |
| const_iterator | **[FindElem](/caches/api/cache/fixed_sized_cache/#function-findelem)**(const Key & key) const |
| std::pair< const_iterator, bool > | **[GetInternal](/caches/api/cache/fixed_sized_cache/#function-getinternal)**(const Key & key) const |

## Detailed Description

```cpp
template <typename Key ,
typename Value ,
template< typename > class Policy =NoCachePolicy>
class caches::fixed_sized_cache;
```

Fixed sized cache that can be used with different policy types (e.g. LRU, FIFO, LFU) 

**Template Parameters**: 

  * **Key** Type of a key (should be hashable) 
  * **Value** Type of a value stored in the cache 
  * **Policy** Type of a policy to be used with the cache 

## Public Types Documentation

### using iterator

```cpp
using caches::fixed_sized_cache< Key, Value, Policy >::iterator =  typename std::unordered_map<Key, Value>::iterator;
```


### using const_iterator

```cpp
using caches::fixed_sized_cache< Key, Value, Policy >::const_iterator =  typename std::unordered_map<Key, Value>::const_iterator;
```


### using operation_guard

```cpp
using caches::fixed_sized_cache< Key, Value, Policy >::operation_guard =  typename std::lock_guard<std::mutex>;
```


### using on_erase_cb

```cpp
using caches::fixed_sized_cache< Key, Value, Policy >::on_erase_cb =  typename std::function<void(const Key &key, const Value &value)>;
```


## Public Functions Documentation

### function fixed_sized_cache

```cpp
inline explicit fixed_sized_cache(
    size_t max_size,
    const Policy< Key > policy =Policy< Key >{},
    on_erase_cb OnErase =[](const Key &, const Value &) {}
)
```

Fixed sized cache constructor. 

**Parameters**: 

  * **max_size** Maximum size of the cache 
  * **policy** Cache policy to use 
  * **OnErase** on_erase_cb function to be called when cache's element get erased 


**Exceptions**: 

  * **std::invalid_argument** 


### function ~fixed_sized_cache

```cpp
inline ~fixed_sized_cache()
```


### function Put

```cpp
inline void Put(
    const Key & key,
    const Value & value
)
```

Put element into the cache. 

**Parameters**: 

  * **key** Key value to use 
  * **value** Value to assign to the given key 


### function TryGet

```cpp
inline std::pair< const_iterator, bool > TryGet(
    const Key & key
) const
```

Try to get an element by the given key from the cache. 

**Parameters**: 

  * **key** Get element by key 


**Return**: Pair of iterator that points to the element and boolean value that shows whether get operation has been successful or not. If pair's boolean value is false, the element is not presented in the cache. If pair's boolean value is true, returned iterator can be used to get access to the element 

### function Get

```cpp
inline const Value & Get(
    const Key & key
) const
```

Get element from the cache if present. 

**Parameters**: 

  * **key** Get element by key 


**Exceptions**: 

  * **std::range_error** 


**Return**: Reference to the value stored by the specified key in the cache 

**Warning**: This method will change in the future with an optional class capabilities to avoid throwing exceptions 

### function Cached

```cpp
inline bool Cached(
    const Key & key
) const
```

Check whether the given key is presented in the cache. 

**Parameters**: 

  * **key** Element key to check 


**Returns**: 

  * **true** Element is presented in the case 
  * **false** Element is not presented in the case 


### function Size

```cpp
inline std::size_t Size() const
```

Get number of elements in cache. 

**Return**: Number of elements currently stored in the cache 

### function Remove

```cpp
inline bool Remove(
    const Key & key
)
```


**Parameters**: 

  * **key** Key parameter 


**Returns**: 

  * **true** if an element specified by key was found and deleted 
  * **false** if an element is not present in a cache 


Remove an element specified by key 


## Protected Functions Documentation

### function Clear

```cpp
inline void Clear()
```


### function begin

```cpp
inline const_iterator begin() const
```


### function end

```cpp
inline const_iterator end() const
```


### function Insert

```cpp
inline void Insert(
    const Key & key,
    const Value & value
)
```


### function Erase

```cpp
inline void Erase(
    const_iterator elem
)
```


### function Erase

```cpp
inline void Erase(
    const Key & key
)
```


### function Update

```cpp
inline void Update(
    const Key & key,
    const Value & value
)
```


### function FindElem

```cpp
inline const_iterator FindElem(
    const Key & key
) const
```


### function GetInternal

```cpp
inline std::pair< const_iterator, bool > GetInternal(
    const Key & key
) const
```
