/**
 * \file
 * \brief Tests for the new simplified cache API
 */
#include "caches/caches.hpp"

#include <gtest/gtest.h>
#ifdef CUSTOM_HASHMAP
#include <parallel_hashmap/phmap.h>
#endif /* CUSTOM_HASHMAP */

#include <cstddef>
#include <functional>
#include <string>

TEST(BasicCacheTest, SimplePutAndGet)
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key1", 100);
    cache.Put("key2", 200);

    EXPECT_EQ(*cache.Get("key1"), 100);
    EXPECT_EQ(*cache.Get("key2"), 200);
}

TEST(BasicCacheTest, PutUpdatesExistingKey)
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key", 100);
    EXPECT_EQ(*cache.Get("key"), 100);

    cache.Put("key", 200);
    EXPECT_EQ(*cache.Get("key"), 200);
    EXPECT_EQ(cache.Size(), 1);
}

TEST(BasicCacheTest, GetThrowsForMissingKey)
{
    caches::cache<std::string, int> cache{10};
    EXPECT_THROW(cache.Get("nonexistent"), std::range_error);
}

TEST(BasicCacheTest, TryGetReturnsNullptrForMissingKey)
{
    caches::cache<std::string, int> cache{10};

    auto result = cache.TryGet("nonexistent");
    EXPECT_FALSE(result.second);
    EXPECT_EQ(result.first, nullptr);
}

TEST(BasicCacheTest, TryGetReturnsValueForExistingKey)
{
    caches::cache<std::string, int> cache{10};
    cache.Put("key", 42);

    auto result = cache.TryGet("key");
    EXPECT_TRUE(result.second);
    EXPECT_EQ(*result.first, 42);
}

TEST(BasicCacheTest, CachedReturnsCorrectStatus)
{
    caches::cache<std::string, int> cache{10};

    EXPECT_FALSE(cache.Cached("key"));
    cache.Put("key", 100);
    EXPECT_TRUE(cache.Cached("key"));
}

TEST(BasicCacheTest, RemoveDeletesEntry)
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key", 100);
    EXPECT_TRUE(cache.Cached("key"));

    EXPECT_TRUE(cache.Remove("key"));
    EXPECT_FALSE(cache.Cached("key"));
}

TEST(BasicCacheTest, RemoveReturnsFalseForMissingKey)
{
    caches::cache<std::string, int> cache{10};
    EXPECT_FALSE(cache.Remove("nonexistent"));
}

TEST(BasicCacheTest, SizeReturnsCorrectCount)
{
    caches::cache<std::string, int> cache{10};

    EXPECT_EQ(cache.Size(), 0);
    cache.Put("key1", 1);
    EXPECT_EQ(cache.Size(), 1);
    cache.Put("key2", 2);
    EXPECT_EQ(cache.Size(), 2);
    cache.Remove("key1");
    EXPECT_EQ(cache.Size(), 1);
}

TEST(BasicCacheTest, MaxSizeReturnsCapacity)
{
    caches::cache<std::string, int> cache{42};
    EXPECT_EQ(cache.MaxSize(), 42);
}

TEST(BasicCacheTest, EmptyReturnsCorrectStatus)
{
    caches::cache<std::string, int> cache{10};

    EXPECT_TRUE(cache.Empty());
    cache.Put("key", 100);
    EXPECT_FALSE(cache.Empty());
}

TEST(BasicCacheTest, ClearRemovesAllEntries)
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key1", 1);
    cache.Put("key2", 2);
    cache.Put("key3", 3);

    cache.Clear();

    EXPECT_TRUE(cache.Empty());
    EXPECT_EQ(cache.Size(), 0);
}

TEST(BasicCacheTest, ZeroSizeThrows)
{
    EXPECT_THROW((caches::cache<std::string, int>{0}), std::invalid_argument);
}

TEST(LRUCacheTest, EvictsLeastRecentlyUsed)
{
    caches::cache<std::string, int, caches::LRU> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);

    // Access A to make B the LRU
    cache.Get("A");

    // Insert C, should evict B
    cache.Put("C", 3);

    EXPECT_TRUE(cache.Cached("A"));
    EXPECT_FALSE(cache.Cached("B"));
    EXPECT_TRUE(cache.Cached("C"));
}

TEST(LRUCacheTest, UpdateRefreshesAccessTime)
{
    caches::cache<std::string, int, caches::LRU> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);

    // Update A (should refresh its access time)
    cache.Put("A", 10);

    // Insert C, should evict B (not A)
    cache.Put("C", 3);

    EXPECT_TRUE(cache.Cached("A"));
    EXPECT_FALSE(cache.Cached("B"));
    EXPECT_TRUE(cache.Cached("C"));
    EXPECT_EQ(*cache.Get("A"), 10);
}

TEST(FIFOCacheTest, EvictsFirstInserted)
{
    caches::cache<std::string, int, caches::FIFO> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);

    // Access A (should not affect FIFO order)
    cache.Get("A");

    // Insert C, should evict A (first inserted)
    cache.Put("C", 3);

    EXPECT_FALSE(cache.Cached("A"));
    EXPECT_TRUE(cache.Cached("B"));
    EXPECT_TRUE(cache.Cached("C"));
}

TEST(LFUCacheTest, EvictsLeastFrequentlyUsed)
{
    caches::cache<std::string, int, caches::LFU> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);

    // Access A multiple times
    cache.Get("A");
    cache.Get("A");
    cache.Get("A");

    // B only accessed once (during Put)
    // Insert C, should evict B (least frequently used)
    cache.Put("C", 3);

    EXPECT_TRUE(cache.Cached("A"));
    EXPECT_FALSE(cache.Cached("B"));
    EXPECT_TRUE(cache.Cached("C"));
}

TEST(NoEvictionCacheTest, EvictsSomeEntry)
{
    caches::cache<std::string, int, caches::NoEviction> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);

    // One of A or B should be evicted
    EXPECT_EQ(cache.Size(), 2);
    EXPECT_TRUE(cache.Cached("C"));
    // Either A or B is evicted (implementation-defined)
    EXPECT_TRUE(cache.Cached("A") || cache.Cached("B"));
}

namespace custom_types
{

struct MyKey
{
    int id;
    std::string name;

    explicit MyKey(int i, std::string n = "") : id(i), name(std::move(n))
    {
    }
};

// ADL-discoverable hash function
std::size_t hash_value(const MyKey &key)
{
    return std::hash<int>{}(key.id) ^ (std::hash<std::string>{}(key.name) << 1);
}

// ADL-discoverable equality operator
bool operator==(const MyKey &lhs, const MyKey &rhs)
{
    return lhs.id == rhs.id && lhs.name == rhs.name;
}

} // namespace custom_types

TEST(CustomKeyADLTest, WorksWithADLHashAndEqual)
{
    // This should compile and work because hash_value() and operator== are
    // discoverable via ADL in the custom_types namespace
    caches::cache<custom_types::MyKey, std::string> cache{10};

    cache.Put(custom_types::MyKey{1, "one"}, "value1");
    cache.Put(custom_types::MyKey{2, "two"}, "value2");

    EXPECT_EQ(*cache.Get(custom_types::MyKey{1, "one"}), "value1");
    EXPECT_EQ(*cache.Get(custom_types::MyKey{2, "two"}), "value2");

    EXPECT_TRUE(cache.Cached(custom_types::MyKey{1, "one"}));
    EXPECT_FALSE(cache.Cached(custom_types::MyKey{3, "three"}));
}

namespace explicit_traits_test
{

struct ExternalKey
{
    double value;

    explicit ExternalKey(double v) : value(v)
    {
    }
};

struct ExternalKeyHash
{
    std::size_t operator()(const ExternalKey &key) const noexcept
    {
        return std::hash<double>{}(key.value);
    }
};

struct ExternalKeyEqual
{
    bool operator()(const ExternalKey &lhs, const ExternalKey &rhs) const noexcept
    {
        return lhs.value == rhs.value;
    }
};

} // namespace explicit_traits_test

TEST(CustomKeyExplicitTraitsTest, WorksWithMakeTraits)
{
    using my_traits = caches::make_traits<explicit_traits_test::ExternalKeyHash,
                                          explicit_traits_test::ExternalKeyEqual>;

    caches::cache<explicit_traits_test::ExternalKey, int, caches::LRU, my_traits> cache{10};

    cache.Put(explicit_traits_test::ExternalKey{1.5}, 100);
    cache.Put(explicit_traits_test::ExternalKey{2.5}, 200);

    EXPECT_EQ(*cache.Get(explicit_traits_test::ExternalKey{1.5}), 100);
    EXPECT_EQ(*cache.Get(explicit_traits_test::ExternalKey{2.5}), 200);
}

namespace specialized_traits_test
{

struct SpecialKey
{
    unsigned long long id;

    explicit SpecialKey(unsigned long long i) : id(i)
    {
    }
};

struct SpecialKeyHash
{
    std::size_t operator()(const SpecialKey &key) const noexcept
    {
        return static_cast<std::size_t>(key.id);
    }
};

struct SpecialKeyEqual
{
    bool operator()(const SpecialKey &lhs, const SpecialKey &rhs) const noexcept
    {
        return lhs.id == rhs.id;
    }
};

} // namespace specialized_traits_test

// Specialize key_traits for SpecialKey
template <>
struct caches::key_traits<specialized_traits_test::SpecialKey>
{
    using hash_type = specialized_traits_test::SpecialKeyHash;
    using equal_type = specialized_traits_test::SpecialKeyEqual;
    using allocator_type = std::allocator<specialized_traits_test::SpecialKey>;
};

TEST(CustomKeySpecializedTraitsTest, WorksWithSpecializedKeyTraits)
{
    // No need to specify traits - they're automatically picked up from specialization
    caches::cache<specialized_traits_test::SpecialKey, std::string> cache{10};

    cache.Put(specialized_traits_test::SpecialKey{100ULL}, "hundred");
    cache.Put(specialized_traits_test::SpecialKey{200ULL}, "two hundred");

    EXPECT_EQ(*cache.Get(specialized_traits_test::SpecialKey{100ULL}), "hundred");
    EXPECT_EQ(*cache.Get(specialized_traits_test::SpecialKey{200ULL}), "two hundred");
}

TEST(OnEraseCallbackTest, CallbackInvokedOnEviction)
{
    int callback_count = 0;
    std::string last_evicted_key;

    caches::cache<std::string, int> cache{2, std::hash<std::string>{}, std::equal_to<std::string>{},
                                          std::allocator<std::string>{},
                                          [&](const std::string &key, const auto &)
                                          {
                                              callback_count++;
                                              last_evicted_key = key;
                                          }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    EXPECT_EQ(callback_count, 0);

    // This should trigger eviction
    cache.Put("C", 3);
    EXPECT_EQ(callback_count, 1);
}

TEST(OnEraseCallbackTest, CallbackInvokedOnRemove)
{
    bool callback_called = false;

    caches::cache<std::string, int> cache{
        10, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { callback_called = true; }};

    cache.Put("key", 100);
    cache.Remove("key");

    EXPECT_TRUE(callback_called);
}

TEST(OnEraseCallbackTest, CallbackInvokedOnClear)
{
    int callback_count = 0;

    caches::cache<std::string, int> cache{
        10, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { callback_count++; }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);

    cache.Clear();

    EXPECT_EQ(callback_count, 3);
}

//==============================================================================
// Value Lifetime Tests
//==============================================================================

TEST(ValueLifetimeTest, ValueRemainsValidAfterEviction)
{
    caches::cache<std::string, int> cache{1};

    cache.Put("A", 42);
    auto value_a = cache.Get("A");

    // Evict A by inserting B
    cache.Put("B", 100);

    // A is evicted but the shared_ptr should still be valid
    EXPECT_EQ(*value_a, 42);
    EXPECT_FALSE(cache.Cached("A"));
}

//==============================================================================
// Different Policy Type Combinations
//==============================================================================

TEST(PolicyCombinationsTest, AllPoliciesWithIntKey)
{
    {
        caches::cache<int, int, caches::LRU> lru{5};
        lru.Put(1, 100);
        EXPECT_EQ(*lru.Get(1), 100);
    }
    {
        caches::cache<int, int, caches::FIFO> fifo{5};
        fifo.Put(1, 100);
        EXPECT_EQ(*fifo.Get(1), 100);
    }
    {
        caches::cache<int, int, caches::LFU> lfu{5};
        lfu.Put(1, 100);
        EXPECT_EQ(*lfu.Get(1), 100);
    }
    {
        caches::cache<int, int, caches::NoEviction> no_eviction{5};
        no_eviction.Put(1, 100);
        EXPECT_EQ(*no_eviction.Get(1), 100);
    }
}

//==============================================================================
// Capacity Boundary Tests
//==============================================================================

TEST(CapacityTest, SingleElementCache)
{
    caches::cache<int, int> cache{1};

    cache.Put(1, 100);
    EXPECT_EQ(*cache.Get(1), 100);

    cache.Put(2, 200);
    EXPECT_FALSE(cache.Cached(1));
    EXPECT_EQ(*cache.Get(2), 200);
}

TEST(CapacityTest, LargeCacheStaysWithinCapacity)
{
    constexpr std::size_t CAPACITY = 100;
    caches::cache<int, int> cache{CAPACITY};

    for (int i = 0; i < 1000; ++i)
    {
        cache.Put(i, i * 10);
        EXPECT_LE(cache.Size(), CAPACITY);
    }

    EXPECT_EQ(cache.Size(), CAPACITY);
}

#ifdef CUSTOM_HASHMAP
// Define a template alias for phmap::node_hash_map that matches the expected signature
template <typename Key, typename Value, typename Hash, typename Equal, typename Allocator>
using phmap_node_hash_map = phmap::node_hash_map<Key, Value, Hash, Equal, Allocator>;

TEST(CustomHashMapTest, WorksWithPhmapNodeHashMap)
{
    // Use phmap::node_hash_map as the underlying hash map
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{10};

    cache.Put("key1", 100);
    cache.Put("key2", 200);

    EXPECT_EQ(*cache.Get("key1"), 100);
    EXPECT_EQ(*cache.Get("key2"), 200);
    EXPECT_EQ(cache.Size(), 2);
}

TEST(CustomHashMapTest, LRUEvictionWithPhmap)
{
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);

    // Access A to make B the LRU
    cache.Get("A");

    // Insert C, should evict B
    cache.Put("C", 3);

    EXPECT_TRUE(cache.Cached("A"));
    EXPECT_FALSE(cache.Cached("B"));
    EXPECT_TRUE(cache.Cached("C"));
}

TEST(CustomHashMapTest, FIFOEvictionWithPhmap)
{
    caches::cache<std::string, int, caches::FIFO, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);

    // Access A (should not affect FIFO order)
    cache.Get("A");

    // Insert C, should evict A (first inserted)
    cache.Put("C", 3);

    EXPECT_FALSE(cache.Cached("A"));
    EXPECT_TRUE(cache.Cached("B"));
    EXPECT_TRUE(cache.Cached("C"));
}

TEST(CustomHashMapTest, LFUEvictionWithPhmap)
{
    caches::cache<std::string, int, caches::LFU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);

    // Access A multiple times
    cache.Get("A");
    cache.Get("A");
    cache.Get("A");

    // B only accessed once (during Put)
    // Insert C, should evict B (least frequently used)
    cache.Put("C", 3);

    EXPECT_TRUE(cache.Cached("A"));
    EXPECT_FALSE(cache.Cached("B"));
    EXPECT_TRUE(cache.Cached("C"));
}

TEST(CustomHashMapTest, CustomKeyWithPhmap)
{
    using my_traits = caches::make_traits<explicit_traits_test::ExternalKeyHash,
                                          explicit_traits_test::ExternalKeyEqual>;

    caches::cache<explicit_traits_test::ExternalKey, int, caches::LRU, my_traits,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{10};

    cache.Put(explicit_traits_test::ExternalKey{1.5}, 100);
    cache.Put(explicit_traits_test::ExternalKey{2.5}, 200);

    EXPECT_EQ(*cache.Get(explicit_traits_test::ExternalKey{1.5}), 100);
    EXPECT_EQ(*cache.Get(explicit_traits_test::ExternalKey{2.5}), 200);
}

TEST(CustomHashMapTest, AllOperationsWithPhmap)
{
    caches::cache<int, std::string, caches::LRU, caches::key_traits<int>,
                  caches::default_wrapper<std::string>, phmap_node_hash_map>
        cache{5};

    // Test Put and Get
    cache.Put(1, "one");
    cache.Put(2, "two");
    cache.Put(3, "three");

    EXPECT_EQ(*cache.Get(1), "one");
    EXPECT_EQ(*cache.Get(2), "two");
    EXPECT_EQ(*cache.Get(3), "three");

    // Test TryGet
    auto result = cache.TryGet(1);
    EXPECT_TRUE(result.second);
    EXPECT_EQ(*result.first, "one");

    auto missing = cache.TryGet(99);
    EXPECT_FALSE(missing.second);

    // Test Cached
    EXPECT_TRUE(cache.Cached(1));
    EXPECT_FALSE(cache.Cached(99));

    // Test Size
    EXPECT_EQ(cache.Size(), 3);

    // Test Remove
    EXPECT_TRUE(cache.Remove(2));
    EXPECT_FALSE(cache.Cached(2));
    EXPECT_EQ(cache.Size(), 2);

    // Test Clear
    cache.Clear();
    EXPECT_TRUE(cache.Empty());
    EXPECT_EQ(cache.Size(), 0);
}

// Define a template alias for phmap::flat_hash_map
template <typename Key, typename Value, typename Hash, typename Equal, typename Allocator>
using phmap_flat_hash_map = phmap::flat_hash_map<Key, Value, Hash, Equal, Allocator>;

TEST(CustomHashMapTest, WorksWithPhmapFlatHashMap)
{
    // Use phmap::flat_hash_map as the underlying hash map
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_flat_hash_map>
        cache{10};

    cache.Put("key1", 100);
    cache.Put("key2", 200);

    EXPECT_EQ(*cache.Get("key1"), 100);
    EXPECT_EQ(*cache.Get("key2"), 200);
    EXPECT_EQ(cache.Size(), 2);
}

TEST(CustomHashMapTest, CapacityWithPhmap)
{
    constexpr std::size_t CAPACITY = 50;
    caches::cache<int, int, caches::LRU, caches::key_traits<int>, caches::default_wrapper<int>,
                  phmap_node_hash_map>
        cache{CAPACITY};

    for (int i = 0; i < 500; ++i)
    {
        cache.Put(i, i * 10);
        EXPECT_LE(cache.Size(), CAPACITY);
    }

    EXPECT_EQ(cache.Size(), CAPACITY);
}

#endif // CUSTOM_HASHMAP
