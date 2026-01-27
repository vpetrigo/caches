/**
 * \file
 * \brief Convenience header including all cache components
 *
 * This header provides access to the new simplified cache API.
 * Include this single header to get access to all cache functionality.
 *
 * \code
 * #include <caches/caches.hpp>
 *
 * // Simple LRU cache
 * caches::cache<std::string, int> c{100};
 *
 * // FIFO cache
 * caches::cache<int, std::string, caches::FIFO> fifo{50};
 *
 * // LFU cache
 * caches::cache<int, std::string, caches::LFU> lfu{50};
 *
 * // Custom key type with ADL
 * struct MyKey { int id; };
 * std::size_t hash_value(const MyKey& k) { return std::hash<int>{}(k.id); }
 * bool operator==(const MyKey& a, const MyKey& b) { return a.id == b.id; }
 * caches::cache<MyKey, int> custom{100};
 * \endcode
 */
#ifndef CACHES_CACHES_HPP
#define CACHES_CACHES_HPP

// Key traits for automatic hash/equality detection
#include "key_traits.hpp"
#include "wrapper_policy.hpp"

// Cache policies
#include "policies/fifo.hpp"
#include "policies/lfu.hpp"
#include "policies/lru.hpp"
#include "policies/no_eviction.hpp"
#include "policies/policy_interface.hpp"

// Main cache class
#include "core/cache.hpp"

#endif // CACHES_CACHES_HPP
