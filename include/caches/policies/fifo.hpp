/**
 * \file
 * \brief FIFO (First In, First Out) cache policy
 */
#ifndef CACHES_POLICIES_FIFO_HPP
#define CACHES_POLICIES_FIFO_HPP

#include "../key_traits.hpp"
#include "policy_interface.hpp"

#include <list>
#include <memory>
#include <unordered_map>

namespace caches
{

/**
 * \brief FIFO (First In, First Out) cache eviction policy
 *
 * This policy evicts the element that was inserted first, regardless of
 * how recently or frequently it has been accessed.
 *
 * Example behavior:
 * \code
 * Insert order: A -> B -> C
 * Eviction candidate: A (first inserted)
 * After evicting A and inserting D: B -> C -> D
 * Eviction candidate: B
 * \endcode
 *
 * \tparam Key The type of keys being tracked
 * \tparam KeyTraits Traits class providing hash_type, equal_type, and allocator_type
 */
template <typename Key, typename KeyTraits = key_traits<Key>>
class FIFO : public PolicyInterface<Key>
{
  public:
    using traits_type = KeyTraits;
    using hash_type = typename KeyTraits::hash_type;
    using equal_type = typename KeyTraits::equal_type;
    using allocator_type = typename KeyTraits::allocator_type;

  private:
    using list_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<Key>;
    using fifo_list_type = std::list<Key, list_allocator_type>;
    using fifo_iterator = typename fifo_list_type::const_iterator;

    using map_value_type = std::pair<const Key, fifo_iterator>;
    using map_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<map_value_type>;
    using map_type =
        std::unordered_map<Key, fifo_iterator, hash_type, equal_type, map_allocator_type>;

  public:
    /**
     * \brief Construct a FIFO policy with optional custom functors
     * \param hash Hash functor instance
     * \param equal Equality functor instance
     * \param allocator Allocator instance
     */
    explicit FIFO(const hash_type &hash = hash_type{}, const equal_type &equal = equal_type{},
                  const allocator_type &allocator = allocator_type{})
        : fifo_queue_(list_allocator_type{allocator}),
          key_lookup_(0, hash, equal, map_allocator_type{allocator})
    {
    }

    ~FIFO() override = default;

    FIFO(const FIFO &) = default;
    FIFO(FIFO &&) noexcept = default;
    FIFO &operator=(const FIFO &) = default;
    FIFO &operator=(FIFO &&) noexcept = default;

    void Insert(const Key &key) override
    {
        fifo_queue_.emplace_front(key);
        key_lookup_[key] = fifo_queue_.begin();
    }

    void Touch(const Key &key) noexcept override
    {
        // FIFO doesn't care about access patterns
        (void)key;
    }

    void Erase(const Key &key) noexcept override
    {
        auto it = key_lookup_.find(key);
        if (it != key_lookup_.end())
        {
            fifo_queue_.erase(it->second);
            key_lookup_.erase(it);
        }
    }

    const Key &ReplCandidate() const noexcept override
    {
        return fifo_queue_.back();
    }

  private:
    fifo_list_type fifo_queue_;
    map_type key_lookup_;
};

} // namespace caches

#endif // CACHES_POLICIES_FIFO_HPP
