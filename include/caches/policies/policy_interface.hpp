/**
 * \file
 * \brief Cache policy interface for the new API
 */
#ifndef CACHES_POLICIES_POLICY_INTERFACE_HPP
#define CACHES_POLICIES_POLICY_INTERFACE_HPP

namespace caches
{

/**
 * \brief Abstract base class for cache eviction policies
 *
 * All cache policies must implement this interface. The policy is responsible
 * for tracking key access patterns and determining which key should be evicted
 * when the cache reaches capacity.
 *
 * \tparam Key The type of keys being tracked
 */
template <typename Key>
class PolicyInterface
{
  public:
    virtual ~PolicyInterface() = default;

    /**
     * \brief Called when a new key is inserted into the cache
     * \param key The key being inserted
     */
    virtual void Insert(const Key &key) = 0;

    /**
     * \brief Called when an existing key is accessed (read or updated)
     * \param key The key being accessed
     */
    virtual void Touch(const Key &key) = 0;

    /**
     * \brief Called when a key is removed from the cache
     * \param key The key being removed
     */
    virtual void Erase(const Key &key) = 0;

    /**
     * \brief Get the key that should be evicted next
     * \return Reference to the eviction candidate key
     */
    virtual const Key &ReplCandidate() const = 0;
};

} // namespace caches

#endif // CACHES_POLICIES_POLICY_INTERFACE_HPP
