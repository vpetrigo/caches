// This header exposes two backends for GoogleTest typed tests.
#pragma once

#include "caches/caches.hpp"
#include "caches/wrapper_policy.hpp"

#include "test_helper.hpp"

struct StdBackend
{
    template <template <typename, typename> class Policy, typename Key, typename Value>
    using cache_t = caches::cache<Key, Value, Policy>;
};

struct PhmapBackend
{
    template <template <typename, typename> class Policy, typename Key, typename Value>
    using cache_t = caches::cache<Key, Value, Policy, caches::key_traits<Key>,
                                  caches::wrapper_policy<Value>, phmap_node_hash_map>;
};
