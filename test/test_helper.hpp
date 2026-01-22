/**
 * \file
 * \brief Test helper functions implementation
 */
#pragma once

#ifdef CUSTOM_HASHMAP
#include <parallel_hashmap/phmap.h>

template <typename Key, typename Value, typename Hash, typename Equal, typename Allocator>
using phmap_node_hash_map = phmap::node_hash_map<Key, Value, Hash, Equal, Allocator>;
#endif
