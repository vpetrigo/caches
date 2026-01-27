/**
 * \file
 * \brief LFU (Least Frequently Used) cache policy
 */
#ifndef CACHES_POLICIES_LFU_HPP
#define CACHES_POLICIES_LFU_HPP

#include "caches/key_traits.hpp"
#include "policy_interface.hpp"

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <unordered_map>

namespace caches
{

/**
 * \brief LFU (Least Frequently Used) cache eviction policy
 *
 * This policy evicts the element that has been accessed the fewest times.
 * Each access increments an internal counter for that key.
 *
 * Example behavior:
 * \code
 * Cache with A (accessed 10 times) and B (accessed 2 times)
 * Insert new element C
 * Eviction candidate: B (least frequently accessed)
 * \endcode
 *
 * \tparam Key The type of keys being tracked
 * \tparam KeyTraits Traits class providing hash_type, equal_type, and allocator_type
 */
template <typename Key, typename KeyTraits = key_traits<Key>>
class LFU : public PolicyInterface<Key>
{
  public:
    using traits_type = KeyTraits;
    using hash_type = typename KeyTraits::hash_type;
    using equal_type = typename KeyTraits::equal_type;
    using allocator_type = typename KeyTraits::allocator_type;

  private:
    using frequency_value_type = std::pair<const std::size_t, Key>;
    using frequency_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<frequency_value_type>;
    using frequency_storage_type =
        std::multimap<std::size_t, Key, std::less<std::size_t>, frequency_allocator_type>;
    using lfu_iterator = typename frequency_storage_type::iterator;

    using map_value_type = std::pair<const Key, lfu_iterator>;
    using map_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<map_value_type>;
    using map_type =
        std::unordered_map<Key, lfu_iterator, hash_type, equal_type, map_allocator_type>;

  public:
    /**
     * \brief Construct an LFU policy with optional custom functors
     * \param hash Hash functor instance
     * \param equal Equality functor instance
     * \param allocator Allocator instance
     */
    explicit LFU(const hash_type &hash = hash_type{}, const equal_type &equal = equal_type{},
                 const allocator_type &allocator = allocator_type{})
        : frequency_storage_(std::less<std::size_t>{}, frequency_allocator_type{allocator}),
          lfu_storage_(0, hash, equal, map_allocator_type{allocator})
    {
    }

    ~LFU() override = default;

    LFU(const LFU &) = default;
    LFU(LFU &&) noexcept = default;
    LFU &operator=(const LFU &) = default;
    LFU &operator=(LFU &&) noexcept = default;

    void Insert(const Key &key) override
    {
        constexpr std::size_t INIT_FREQUENCY = 1;
        lfu_storage_[key] =
            frequency_storage_.emplace_hint(frequency_storage_.cbegin(), INIT_FREQUENCY, key);
    }

    void Touch(const Key &key) override
    {
        auto elem_it = lfu_storage_[key];
        auto updated = std::make_pair(elem_it->first + 1, elem_it->second);
        frequency_storage_.erase(elem_it);
        lfu_storage_[key] =
            frequency_storage_.emplace_hint(frequency_storage_.cend(), std::move(updated));
    }

    void Erase(const Key &key) noexcept override
    {
        auto it = lfu_storage_.find(key);
        if (it != lfu_storage_.end())
        {
            frequency_storage_.erase(it->second);
            lfu_storage_.erase(it);
        }
    }

    const Key &ReplCandidate() const noexcept override
    {
        return frequency_storage_.cbegin()->second;
    }

  private:
    frequency_storage_type frequency_storage_;
    map_type lfu_storage_;
};

} // namespace caches

#endif // CACHES_POLICIES_LFU_HPP
