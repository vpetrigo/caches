/**
 * \file
 * \brief LRU (Least Recently Used) cache policy
 */
#ifndef CACHES_POLICIES_LRU_HPP
#define CACHES_POLICIES_LRU_HPP

#include "../key_traits.hpp"
#include "policy_interface.hpp"

#include <list>
#include <memory>
#include <unordered_map>

namespace caches
{

/**
 * \brief LRU (Least Recently Used) cache eviction policy
 *
 * This policy evicts the element that has not been accessed for the longest time.
 * When an element is accessed (via Get or Put), it becomes the most recently used.
 *
 * Example behavior:
 * \code
 * Cache placement order: A, B, C
 * Cache access: A touched, B touched
 * LRU element (eviction candidate): C
 * Cache access: B touched, C touched
 * LRU element (eviction candidate): A
 * \endcode
 *
 * \tparam Key The type of keys being tracked
 * \tparam KeyTraits Traits class providing hash_type, equal_type, and allocator_type
 */
template <typename Key, typename KeyTraits = key_traits<Key>>
class LRU : public PolicyInterface<Key>
{
  public:
    using traits_type = KeyTraits;
    using hash_type = typename KeyTraits::hash_type;
    using equal_type = typename KeyTraits::equal_type;
    using allocator_type = typename KeyTraits::allocator_type;

  private:
    using list_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<Key>;
    using lru_list_type = std::list<Key, list_allocator_type>;
    using lru_iterator = typename lru_list_type::iterator;

    using map_value_type = std::pair<const Key, lru_iterator>;
    using map_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<map_value_type>;
    using map_type =
        std::unordered_map<Key, lru_iterator, hash_type, equal_type, map_allocator_type>;

  public:
    /**
     * \brief Construct an LRU policy with optional custom functors
     * \param hash Hash functor instance
     * \param equal Equality functor instance
     * \param allocator Allocator instance
     */
    explicit LRU(const hash_type &hash = hash_type{}, const equal_type &equal = equal_type{},
                 const allocator_type &allocator = allocator_type{})
        : lru_queue_(list_allocator_type{allocator}),
          key_finder_(0, hash, equal, map_allocator_type{allocator})
    {
    }

    ~LRU() override = default;

    LRU(const LRU &) = default;
    LRU(LRU &&) noexcept = default;
    LRU &operator=(const LRU &) = default;
    LRU &operator=(LRU &&) noexcept = default;

    void Insert(const Key &key) override
    {
        lru_queue_.emplace_front(key);
        key_finder_[key] = lru_queue_.begin();
    }

    void Touch(const Key &key) override
    {
        // Move the touched element to the front of the queue
        lru_queue_.splice(lru_queue_.begin(), lru_queue_, key_finder_[key]);
    }

    void Erase(const Key &key) noexcept override
    {
        auto it = key_finder_.find(key);
        if (it != key_finder_.end())
        {
            lru_queue_.erase(it->second);
            key_finder_.erase(it);
        }
    }

    const Key &ReplCandidate() const noexcept override
    {
        return lru_queue_.back();
    }

  private:
    lru_list_type lru_queue_;
    map_type key_finder_;
};

} // namespace caches

#endif // CACHES_POLICIES_LRU_HPP
