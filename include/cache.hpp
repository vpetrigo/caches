#ifndef CACHE_HPP
#define CACHE_HPP

#include "cache_policy.hpp"

#include <cstddef>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace caches
{

// Base class for caching algorithms
template <typename Key, typename Value, template <typename ...> class Policy = NoCachePolicy>
class fixed_sized_cache
{
  public:
    using iterator = typename std::unordered_map<Key, Value>::iterator;
    using const_iterator = typename std::unordered_map<Key, Value>::const_iterator;
    using operation_guard = typename std::lock_guard<std::mutex>;
    using Callback = typename std::function<void(const Key &key, const Value &value)>;

    explicit fixed_sized_cache(
        size_t max_size, const Policy<Key> policy = Policy<Key>{},
        Callback OnErase = [](const Key &, const Value &) {})
        : cache_policy{policy}, max_cache_size{max_size}, OnEraseCallback{OnErase}
    {
        if (max_cache_size == 0)
        {
            throw std::invalid_argument{"Size of the cache should be non-zero"};
        }
    }

    ~fixed_sized_cache() noexcept
    {
        Clear();
    }

    void Put(const Key &key, const Value &value) noexcept
    {
        operation_guard lock{safe_op};
        auto elem_it = FindElem(key);

        if (elem_it == cache_items_map.end())
        {
            // add new element to the cache
            if (cache_items_map.size() + 1 > max_cache_size)
            {
                auto disp_candidate_key = cache_policy.ReplCandidate();

                Erase(disp_candidate_key);
            }

            Insert(key, value);
        }
        else
        {
            // update previous value
            Update(key, value);
        }
    }

    const Value &Get(const Key &key) const
    {
        operation_guard lock{safe_op};
        auto elem_it = FindElem(key);

        if (elem_it != end())
        {
            cache_policy.Touch(key);

            return elem_it->second;
        }
        else
        {
            throw std::range_error{"No such element in the cache"};
        }
    }

    bool Cached(const Key &key) const noexcept
    {
        operation_guard lock{safe_op};
        return FindElem(key) != cache_items_map.cend();
    }

    std::size_t Size() const
    {
        operation_guard lock{safe_op};

        return cache_items_map.size();
    }

    /**
     * Remove an element specified by key
     * @param key Key parameter
     * @retval true if an element specified by key was found and deleted
     * @retval false if an element is not present in a cache
     */
    bool Remove(const Key &key)
    {
        operation_guard lock{safe_op};

        auto elem = FindElem(key);

        if (elem == cache_items_map.end())
        {
            return false;
        }

        Erase(elem);

        return true;
    }

  protected:
    void Clear()
    {
        operation_guard lock{safe_op};

        for (auto it = begin(); it != end(); ++it)
        {
            cache_policy.Erase(it->first);
        }

        cache_items_map.clear();
    }

    const_iterator begin() const noexcept
    {
        return cache_items_map.cbegin();
    }

    const_iterator end() const noexcept
    {
        return cache_items_map.cend();
    }

  protected:
    void Insert(const Key &key, const Value &value)
    {
        cache_policy.Insert(key);
        cache_items_map.emplace(std::make_pair(key, value));
    }

    void Erase(const_iterator elem)
    {
        cache_policy.Erase(elem->first);
        OnEraseCallback(elem->first, elem->second);
        cache_items_map.erase(elem);
    }

    void Erase(const Key &key)
    {
        auto elem_it = FindElem(key);

        Erase(elem_it);
    }

    void Update(const Key &key, const Value &value)
    {
        cache_policy.Touch(key);
        cache_items_map[key] = value;
    }

    const_iterator FindElem(const Key &key) const
    {
        return cache_items_map.find(key);
    }

  private:
    std::unordered_map<Key, Value> cache_items_map;
    mutable Policy<Key> cache_policy;
    mutable std::mutex safe_op;
    size_t max_cache_size;
    Callback OnEraseCallback;
};
} // namespace caches

#endif // CACHE_HPP
