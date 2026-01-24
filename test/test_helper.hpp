/**
 * \file
 * \brief Test helper functions implementation
 */
#pragma once

#include <gtl/intrusive.hpp>
#include <gtl/phmap.hpp>

template <typename Key, typename Value, typename Hash, typename Equal, typename Allocator>
using phmap_node_hash_map = gtl::node_hash_map<Key, Value, Hash, Equal, Allocator>;

template <typename Key, typename Value, typename Hash, typename Equal, typename Allocator>
using phmap_flat_hash_map = gtl::flat_hash_map<Key, Value, Hash, Equal, Allocator>;

template <typename Value>
using gtl_shared_ptr = gtl::intrusive_ptr<Value>;
