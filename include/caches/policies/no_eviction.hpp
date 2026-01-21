/**
 * \file
 * \brief NoEviction cache policy (no specific eviction order)
 */
#ifndef CACHES_POLICIES_NO_EVICTION_HPP
#define CACHES_POLICIES_NO_EVICTION_HPP

#include "../key_traits.hpp"
#include "policy_interface.hpp"

#include <memory>
#include <unordered_set>

namespace caches
{

/**
 * \brief NoEviction cache policy with undefined eviction order
 *
 * This policy simply tracks which keys are in the cache without maintaining
 * any specific order. When eviction is needed, an arbitrary key is selected.
 * This is useful when you don't care about eviction order or when the cache
 * is expected to stay within capacity.
 *
 * \note The eviction candidate is implementation-defined and may vary between
 * different standard library implementations or even between runs.
 *
 * \tparam Key The type of keys being tracked
 * \tparam KeyTraits Traits class providing hash_type, equal_type, and allocator_type
 */
template <typename Key, typename KeyTraits = key_traits<Key>>
class NoEviction : public PolicyInterface<Key>
{
  public:
    using traits_type = KeyTraits;
    using hash_type = typename KeyTraits::hash_type;
    using equal_type = typename KeyTraits::equal_type;
    using allocator_type = typename KeyTraits::allocator_type;

  private:
    using set_allocator_type =
        typename std::allocator_traits<allocator_type>::template rebind_alloc<Key>;
    using set_type = std::unordered_set<Key, hash_type, equal_type, set_allocator_type>;

  public:
    /**
     * \brief Construct a NoEviction policy with optional custom functors
     * \param hash Hash functor instance
     * \param equal Equality functor instance
     * \param allocator Allocator instance
     */
    explicit NoEviction(const hash_type &hash = hash_type{}, const equal_type &equal = equal_type{},
                        const allocator_type &allocator = allocator_type{})
        : key_storage_(0, hash, equal, set_allocator_type{allocator})
    {
    }

    ~NoEviction() noexcept override = default;

    NoEviction(const NoEviction &) = default;
    NoEviction(NoEviction &&) noexcept = default;
    NoEviction &operator=(const NoEviction &) = default;
    NoEviction &operator=(NoEviction &&) noexcept = default;

    void Insert(const Key &key) override
    {
        key_storage_.emplace(key);
    }

    void Touch(const Key &key) noexcept override
    {
        (void)key;
    }

    void Erase(const Key &key) noexcept override
    {
        key_storage_.erase(key);
    }

    const Key &ReplCandidate() const noexcept override
    {
        return *key_storage_.cbegin();
    }

  private:
    set_type key_storage_;
};

} // namespace caches

#endif // CACHES_POLICIES_NO_EVICTION_HPP
