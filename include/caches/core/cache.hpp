/**
 * \file
 * \brief Generic cache implementation
 */
#ifndef CACHES_CORE_CACHE_HPP
#define CACHES_CORE_CACHE_HPP

#include "caches/key_traits.hpp"
#include "caches/policies/lru.hpp"
#include "caches/wrapper_policy.hpp"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace caches
{

namespace detail
{
// Default hash map type alias
template <typename Key, typename Value, typename Hash, typename Equal, typename Allocator>
using default_hash_map = std::unordered_map<Key, Value, Hash, Equal, Allocator>;
} // namespace detail

/**
 * \brief Fixed-size cache with configurable eviction policy
 *
 * A thread-safe, fixed-size cache that automatically evicts entries when
 * capacity is reached. The eviction strategy is determined by the Policy
 * template parameter.
 *
 * \par Basic Usage
 * \code
 * // Simple LRU cache (default)
 * caches::cache<std::string, int> c{100};
 * c.Put("key", 42);
 * auto value = c.Get("key");  // returns wrapper type (default: shared_ptr<int>)
 * \endcode
 *
 * \par Custom Key Types
 * For types without std::hash specialization, you can either:
 *
 * 1. Define ADL-discoverable hash_value() and operator==:
 * \code
 * struct MyKey { int id; };
 * std::size_t hash_value(const MyKey& k) { return std::hash<int>{}(k.id); }
 * bool operator==(const MyKey& a, const MyKey& b) { return a.id == b.id; }
 *
 * caches::cache<MyKey, int> c{100};  // Automatically detected!
 * \endcode
 *
 * 2. Use make_traits to specify custom functors:
 * \code
 * struct MyHash { size_t operator()(const MyKey&) const; };
 * struct MyEqual { bool operator()(const MyKey&, const MyKey&) const; };
 *
 * using my_traits = caches::make_traits<MyHash, MyEqual>;
 * caches::cache<MyKey, int, caches::LRU, my_traits> c{100};
 * \endcode
 *
 * 3. Specialize key_traits for reusable configuration:
 * \code
 * template<>
 * struct caches::key_traits<MyKey> {
 *     using hash_type = MyHash;
 *     using equal_type = MyEqual;
 *     using allocator_type = std::allocator<MyKey>;
 * };
 *
 * caches::cache<MyKey, int> c{100};  // Uses specialized traits
 * \endcode
 *
 * \par Custom HashMap
 * You can also use a custom hash map implementation (e.g., parallel-hashmap):
 * \code
 * #include <parallel_hashmap/phmap.h>
 *
 * template <typename K, typename V, typename H, typename E, typename A>
 * using my_hash_map = phmap::node_hash_map<K, V, H, E, A>;
 *
 * caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, my_hash_map>
 * c{100};
 * \endcode
 *
 * \tparam Key Type of cache keys
 * \tparam Value Type of cached values
 * \tparam Policy Eviction policy template (default: LRU)
 * \tparam KeyTraits Traits providing hash_type, equal_type, allocator_type
 * \tparam WrapperPolicy Policy providing wrapper `type` and `create()` (default:
 * wrapper_policy<Value>)
 * \tparam HashMap Template for the underlying hash map (default: std::unordered_map)
 */
template <typename Key, typename Value, template <typename, typename> class Policy = LRU,
          typename KeyTraits = key_traits<Key>, typename WrapperPolicy = wrapper_policy<Value>,
          template <typename, typename, typename, typename, typename> class HashMap =
              detail::default_hash_map>
class cache
{
  public:
    using key_type = Key;
    using mapped_type = Value;
    using wrapper_policy_type = WrapperPolicy;
    using value_type = typename WrapperPolicy::type;
    using traits_type = KeyTraits;
    using hash_type = typename KeyTraits::hash_type;
    using equal_type = typename KeyTraits::equal_type;
    using allocator_type = typename KeyTraits::allocator_type;
    using policy_type = Policy<Key, KeyTraits>;

  private:
    using map_value_type = std::pair<const Key, value_type>;
    using map_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<map_value_type>;
    using map_type = HashMap<Key, value_type, hash_type, equal_type, map_allocator_type>;
    using iterator = typename map_type::iterator;
    using const_iterator = typename map_type::const_iterator;
    using lock_guard = std::lock_guard<std::mutex>;

  public:
    /**
     * \brief Callback type for erase notifications
     */
    using on_erase_callback = std::function<void(const Key &, const value_type &)>;

    /**
     * \brief Construct a cache with the specified maximum size
     *
     * \param max_size Maximum number of entries the cache can hold
     * \param hash Hash functor instance (optional)
     * \param equal Equality functor instance (optional)
     * \param allocator Allocator instance (optional)
     * \param on_erase Callback invoked when an entry is evicted (optional)
     *
     * \throw std::invalid_argument if max_size is 0
     */
    explicit cache(
        std::size_t max_size, const hash_type &hash = hash_type{},
        const equal_type &equal = equal_type{}, const allocator_type &allocator = allocator_type{},
        on_erase_callback on_erase = [](const Key &, const value_type &) {})
        : cache_map_{0, hash, equal, map_allocator_type{allocator}},
          policy_{hash, equal, allocator}, max_size_{max_size}, on_erase_{std::move(on_erase)}
    {
        if (max_size_ == 0)
        {
            throw std::invalid_argument{"Cache size must be greater than 0"};
        }
    }

    /**
     * \brief Construct a cache with a pre-configured policy instance
     *
     * \param max_size Maximum number of entries
     * \param policy Policy instance to use
     * \param hash Hash functor instance
     * \param equal Equality functor instance
     * \param allocator Allocator instance
     * \param on_erase Callback invoked when an entry is evicted
     *
     * \throw std::invalid_argument if max_size is 0
     */
    explicit cache(
        std::size_t max_size, policy_type policy, const hash_type &hash = hash_type{},
        const equal_type &equal = equal_type{}, const allocator_type &allocator = allocator_type{},
        on_erase_callback on_erase = [](const Key &, const value_type &) {})
        : cache_map_{0, hash, equal, map_allocator_type{allocator}}, policy_{std::move(policy)},
          max_size_{max_size}, on_erase_{std::move(on_erase)}
    {
        if (max_size_ == 0)
        {
            throw std::invalid_argument{"Cache size must be greater than 0"};
        }
    }

    ~cache() noexcept
    {
        Clear();
    }

    // Non-copyable, non-movable due to mutex
    cache(const cache &) = delete;
    cache &operator=(const cache &) = delete;
    cache(cache &&) = delete;
    cache &operator=(cache &&) = delete;

    /**
     * \brief Insert or update an entry in the cache
     *
     * If the key already exists, its value is updated.
     * If the cache is full, the entry selected by the policy is evicted first.
     *
     * \param key Key to insert/update
     * \param value Value to associate with the key
     */
    void Put(const Key &key, const Value &value) noexcept
    {
        lock_guard lock{mutex_};

        auto it = cache_map_.find(key);
        if (it == cache_map_.end())
        {
            // Evict if necessary
            if (cache_map_.size() >= max_size_)
            {
                EvictOne();
            }
            // Insert new entry
            policy_.Insert(key);
            cache_map_.emplace(key, WrapperPolicy::create(value));
        }
        else
        {
            // Update existing entry
            policy_.Touch(key);
            it->second = WrapperPolicy::create(value);
        }
    }

    /**
     * \brief Get an entry from the cache
     *
     * \param key Key to look up
     * \return Wrapped value (throws if not found)
     * \throw std::range_error if key is not in the cache
     */
    value_type Get(const Key &key) const
    {
        lock_guard lock{mutex_};

        auto it = cache_map_.find(key);
        if (it != cache_map_.end())
        {
            policy_.Touch(key);
            return it->second;
        }
        throw std::range_error{"Key not found in cache"};
    }

    /**
     * \brief Try to get an entry from the cache without throwing
     *
     * \param key Key to look up
     * \return Pair of (value, found). If found is false, value is default-constructed.
     */
    std::pair<value_type, bool> TryGet(const Key &key) const noexcept
    {
        lock_guard lock{mutex_};

        auto it = cache_map_.find(key);
        if (it != cache_map_.end())
        {
            policy_.Touch(key);
            return {it->second, true};
        }
        return {value_type{}, false};
    }

    /**
     * \brief Check if a key exists in the cache
     *
     * \note This does not update the access pattern for the policy.
     *
     * \param key Key to check
     * \return true if the key exists in the cache
     */
    bool Cached(const Key &key) const noexcept
    {
        lock_guard lock{mutex_};
        return cache_map_.find(key) != cache_map_.end();
    }

    /**
     * \brief Remove an entry from the cache
     *
     * \param key Key to remove
     * \return true if the key was found and removed
     */
    bool Remove(const Key &key)
    {
        lock_guard lock{mutex_};

        auto it = cache_map_.find(key);
        if (it == cache_map_.end())
        {
            return false;
        }

        policy_.Erase(key);
        on_erase_(key, it->second);
        cache_map_.erase(it);
        return true;
    }

    /**
     * \brief Get the number of entries in the cache
     * \return Current number of cached entries
     */
    std::size_t Size() const noexcept
    {
        lock_guard lock{mutex_};
        return cache_map_.size();
    }

    /**
     * \brief Get the maximum capacity of the cache
     * \return Maximum number of entries the cache can hold
     */
    std::size_t MaxSize() const noexcept
    {
        return max_size_;
    }

    /**
     * \brief Check if the cache is empty
     * \return true if the cache contains no entries
     */
    bool Empty() const noexcept
    {
        lock_guard lock{mutex_};
        return cache_map_.empty();
    }

    /**
     * \brief Remove all entries from the cache
     */
    void Clear() noexcept
    {
        lock_guard lock{mutex_};
        for (const auto &entry : cache_map_)
        {
            policy_.Erase(entry.first);
            on_erase_(entry.first, entry.second);
        }
        cache_map_.clear();
    }

  private:
    void EvictOne()
    {
        const Key &victim = policy_.ReplCandidate();
        auto it = cache_map_.find(victim);
        if (it != cache_map_.end())
        {
            on_erase_(it->first, it->second);
            cache_map_.erase(it);
            policy_.Erase(victim);
        }
    }

    map_type cache_map_;
    mutable policy_type policy_;
    mutable std::mutex mutex_;
    std::size_t max_size_;
    on_erase_callback on_erase_;
};

} // namespace caches

#endif // CACHES_CORE_CACHE_HPP
