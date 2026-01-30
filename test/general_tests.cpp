/**
 * \file
 * \brief Tests for the new simplified cache API
 */
#include "caches/caches.hpp"

#include "test_helper.hpp"

#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <functional>
#include <string>

TEST_CASE("SimplePutAndGet", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key1", 100);
    cache.Put("key2", 200);

    CHECK(*cache.Get("key1") == 100);
    CHECK(*cache.Get("key2") == 200);
}

TEST_CASE("PutUpdatesExistingKey", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key", 100);
    CHECK(*cache.Get("key") == 100);

    cache.Put("key", 200);
    CHECK(*cache.Get("key") == 200);
    CHECK(cache.Size() == 1);
}

TEST_CASE("GetThrowsForMissingKey", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};
    CHECK_THROWS_AS(cache.Get("nonexistent"), std::range_error);
}

TEST_CASE("TryGetReturnsNullptrForMissingKey", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};

    auto result = cache.TryGet("nonexistent");
    CHECK_FALSE(result.second);
    CHECK(result.first == nullptr);
}

TEST_CASE("TryGetReturnsValueForExistingKey", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};
    cache.Put("key", 42);

    auto result = cache.TryGet("key");
    CHECK(result.second);
    CHECK(*result.first == 42);
}

TEST_CASE("CachedReturnsCorrectStatus", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};

    CHECK_FALSE(cache.Cached("key"));
    cache.Put("key", 100);
    CHECK(cache.Cached("key"));
}

TEST_CASE("RemoveDeletesEntry", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key", 100);
    CHECK(cache.Cached("key"));

    CHECK(cache.Remove("key"));
    CHECK_FALSE(cache.Cached("key"));
}

TEST_CASE("RemoveReturnsFalseForMissingKey", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};
    CHECK_FALSE(cache.Remove("nonexistent"));
}

TEST_CASE("SizeReturnsCorrectCount", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};

    CHECK(cache.Size() == 0);
    cache.Put("key1", 1);
    CHECK(cache.Size() == 1);
    cache.Put("key2", 2);
    CHECK(cache.Size() == 2);
    cache.Remove("key1");
    CHECK(cache.Size() == 1);
}

TEST_CASE("MaxSizeReturnsCapacity", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{42};
    CHECK(cache.MaxSize() == 42);
}

TEST_CASE("EmptyReturnsCorrectStatus", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};

    CHECK(cache.Empty());
    cache.Put("key", 100);
    CHECK_FALSE(cache.Empty());
}

TEST_CASE("ClearRemovesAllEntries", "[BasicCacheTest]")
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key1", 1);
    cache.Put("key2", 2);
    cache.Put("key3", 3);

    cache.Clear();

    CHECK(cache.Empty());
    CHECK(cache.Size() == 0);
}

TEST_CASE("ZeroSizeThrows", "[BasicCacheTest]")
{
    CHECK_THROWS_AS((caches::cache<std::string, int>{0}), std::invalid_argument);
}

TEST_CASE("EvictsLeastRecentlyUsed", "[LRUCacheTest]")
{
    caches::cache<std::string, int, caches::LRU> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A");
    cache.Put("C", 3);

    CHECK(cache.Cached("A"));
    CHECK_FALSE(cache.Cached("B"));
    CHECK(cache.Cached("C"));
}

TEST_CASE("UpdateRefreshesAccessTime", "[LRUCacheTest]")
{
    caches::cache<std::string, int, caches::LRU> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("A", 10);
    cache.Put("C", 3);

    CHECK(cache.Cached("A"));
    CHECK_FALSE(cache.Cached("B"));
    CHECK(cache.Cached("C"));
    CHECK(*cache.Get("A") == 10);
}

TEST_CASE("EvictsFirstInserted", "[FIFOCacheTest]")
{
    caches::cache<std::string, int, caches::FIFO> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A");
    cache.Put("C", 3);

    CHECK_FALSE(cache.Cached("A"));
    CHECK(cache.Cached("B"));
    CHECK(cache.Cached("C"));
}

TEST_CASE("EvictsLeastFrequentlyUsed", "[LFUCacheTest]")
{
    caches::cache<std::string, int, caches::LFU> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A");
    cache.Get("A");
    cache.Get("A");
    cache.Put("C", 3);

    CHECK(cache.Cached("A"));
    CHECK_FALSE(cache.Cached("B"));
    CHECK(cache.Cached("C"));
}

TEST_CASE("EvictsSomeEntry", "[NoEvictionCacheTest]")
{
    caches::cache<std::string, int, caches::NoEviction> cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);

    CHECK(cache.Size() == 2);
    CHECK(cache.Cached("C"));
    CHECK((cache.Cached("A") || cache.Cached("B")));
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

std::size_t hash_value(const MyKey &key)
{
    return std::hash<int>{}(key.id) ^ (std::hash<std::string>{}(key.name) << 1);
}

bool operator==(const MyKey &lhs, const MyKey &rhs)
{
    return lhs.id == rhs.id && lhs.name == rhs.name;
}

} // namespace custom_types

TEST_CASE("WorksWithADLHashAndEqual", "[CustomKeyADLTest]")
{
    caches::cache<custom_types::MyKey, std::string> cache{10};

    cache.Put(custom_types::MyKey{1, "one"}, "value1");
    cache.Put(custom_types::MyKey{2, "two"}, "value2");

    CHECK(*cache.Get(custom_types::MyKey{1, "one"}) == "value1");
    CHECK(*cache.Get(custom_types::MyKey{2, "two"}) == "value2");

    CHECK(cache.Cached(custom_types::MyKey{1, "one"}));
    CHECK_FALSE(cache.Cached(custom_types::MyKey{3, "three"}));
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

TEST_CASE("WorksWithMakeTraits", "[CustomKeyExplicitTraitsTest]")
{
    using my_traits = caches::make_traits<explicit_traits_test::ExternalKeyHash,
                                          explicit_traits_test::ExternalKeyEqual>;

    caches::cache<explicit_traits_test::ExternalKey, int, caches::LRU, my_traits> cache{10};

    cache.Put(explicit_traits_test::ExternalKey{1.5}, 100);
    cache.Put(explicit_traits_test::ExternalKey{2.5}, 200);

    CHECK(*cache.Get(explicit_traits_test::ExternalKey{1.5}) == 100);
    CHECK(*cache.Get(explicit_traits_test::ExternalKey{2.5}) == 200);
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

template <>
struct caches::key_traits<specialized_traits_test::SpecialKey>
{
    using hash_type = specialized_traits_test::SpecialKeyHash;
    using equal_type = specialized_traits_test::SpecialKeyEqual;
    using allocator_type = std::allocator<specialized_traits_test::SpecialKey>;
};

TEST_CASE("WorksWithSpecializedKeyTraits", "[CustomKeySpecializedTraitsTest]")
{
    caches::cache<specialized_traits_test::SpecialKey, std::string> cache{10};

    cache.Put(specialized_traits_test::SpecialKey{100ULL}, "hundred");
    cache.Put(specialized_traits_test::SpecialKey{200ULL}, "two hundred");

    CHECK(*cache.Get(specialized_traits_test::SpecialKey{100ULL}) == "hundred");
    CHECK(*cache.Get(specialized_traits_test::SpecialKey{200ULL}) == "two hundred");
}

TEST_CASE("CallbackInvokedOnEviction", "[OnEraseCallbackTest]")
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
    CHECK(callback_count == 0);
    cache.Put("C", 3);
    CHECK(callback_count == 1);
}

TEST_CASE("CallbackInvokedOnRemove", "[OnEraseCallbackTest]")
{
    bool callback_called = false;

    caches::cache<std::string, int> cache{
        10, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { callback_called = true; }};

    cache.Put("key", 100);
    cache.Remove("key");

    CHECK(callback_called);
}

TEST_CASE("CallbackInvokedOnClear", "[OnEraseCallbackTest]")
{
    int callback_count = 0;

    caches::cache<std::string, int> cache{
        10, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { callback_count++; }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);

    cache.Clear();

    CHECK(callback_count == 3);
}

TEST_CASE("CallbackInvokedOnEvictionFIFO", "[OnEraseCallbackTest]")
{
    int callback_count = 0;
    std::string last_evicted;

    caches::cache<std::string, int, caches::FIFO> cache{
        2, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &key, const auto &)
        {
            callback_count++;
            last_evicted = key;
        }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    CHECK(callback_count == 0);

    cache.Put("C", 3); // Should evict A (FIFO)
    CHECK(callback_count == 1);
    CHECK(last_evicted == "A");
}

TEST_CASE("CallbackInvokedOnEvictionLFU", "[OnEraseCallbackTest]")
{
    int callback_count = 0;

    caches::cache<std::string, int, caches::LFU> cache{
        2, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { callback_count++; }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A"); // Increase A's frequency
    cache.Get("A");

    cache.Put("C", 3); // Should evict B (LFU)
    CHECK(callback_count == 1);
    CHECK_FALSE(cache.Cached("B"));
}

TEST_CASE("CallbackInvokedOnEvictionNoEviction", "[OnEraseCallbackTest]")
{
    int callback_count = 0;

    caches::cache<std::string, int, caches::NoEviction> cache{
        2, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { callback_count++; }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3); // Evicts some entry
    CHECK(callback_count == 1);
    CHECK(cache.Size() == 2);
}

TEST_CASE("ClearWithCallbackFIFO", "[OnEraseCallbackTest]")
{
    int count = 0;
    caches::cache<std::string, int, caches::FIFO> cache{
        10, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { count++; }};
    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);
    cache.Clear();
    CHECK(count == 3);
}

TEST_CASE("ClearWithCallbackLFU", "[OnEraseCallbackTest]")
{
    int count = 0;
    caches::cache<std::string, int, caches::LFU> cache{
        10, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { count++; }};
    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);
    cache.Clear();
    CHECK(count == 3);
}

TEST_CASE("ClearWithCallbackNoEviction", "[OnEraseCallbackTest]")
{
    int count = 0;
    caches::cache<std::string, int, caches::NoEviction> cache{
        10, std::hash<std::string>{}, std::equal_to<std::string>{}, std::allocator<std::string>{},
        [&](const std::string &, const auto &) { count++; }};
    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);
    cache.Clear();
    CHECK(count == 3);
}

TEST_CASE("RemoveWithCallbackAllPolicies", "[OnEraseCallbackTest]")
{
    {
        int count = 0;
        caches::cache<std::string, int, caches::LRU> cache{
            10, std::hash<std::string>{}, std::equal_to<std::string>{},
            std::allocator<std::string>{}, [&](const std::string &, const auto &) { count++; }};
        cache.Put("key", 100);
        cache.Remove("key");
        CHECK(count == 1);
    }
    {
        int count = 0;
        caches::cache<std::string, int, caches::FIFO> cache{
            10, std::hash<std::string>{}, std::equal_to<std::string>{},
            std::allocator<std::string>{}, [&](const std::string &, const auto &) { count++; }};
        cache.Put("key", 100);
        cache.Remove("key");
        CHECK(count == 1);
    }
    {
        int count = 0;
        caches::cache<std::string, int, caches::LFU> cache{
            10, std::hash<std::string>{}, std::equal_to<std::string>{},
            std::allocator<std::string>{}, [&](const std::string &, const auto &) { count++; }};
        cache.Put("key", 100);
        cache.Remove("key");
        CHECK(count == 1);
    }
    {
        int count = 0;
        caches::cache<std::string, int, caches::NoEviction> cache{
            10, std::hash<std::string>{}, std::equal_to<std::string>{},
            std::allocator<std::string>{}, [&](const std::string &, const auto &) { count++; }};
        cache.Put("key", 100);
        cache.Remove("key");
        CHECK(count == 1);
    }
}

TEST_CASE("ValueRemainsValidAfterEviction", "[ValueLifetimeTest]")
{
    caches::cache<std::string, int> cache{1};

    cache.Put("A", 42);
    auto value_a = cache.Get("A");

    cache.Put("B", 100);
    CHECK(*value_a == 42);
    CHECK_FALSE(cache.Cached("A"));
}

TEST_CASE("AllPoliciesWithIntKey", "[PolicyCombinationsTest]")
{
    {
        caches::cache<int, int, caches::LRU> lru{5};
        lru.Put(1, 100);
        CHECK(*lru.Get(1) == 100);
    }
    {
        caches::cache<int, int, caches::FIFO> fifo{5};
        fifo.Put(1, 100);
        CHECK(*fifo.Get(1) == 100);
    }
    {
        caches::cache<int, int, caches::LFU> lfu{5};
        lfu.Put(1, 100);
        CHECK(*lfu.Get(1) == 100);
    }
    {
        caches::cache<int, int, caches::NoEviction> no_eviction{5};
        no_eviction.Put(1, 100);
        CHECK(*no_eviction.Get(1) == 100);
    }
}

TEST_CASE("SingleElementCache", "[CapacityTest]")
{
    caches::cache<int, int> cache{1};

    cache.Put(1, 100);
    CHECK(*cache.Get(1) == 100);

    cache.Put(2, 200);
    CHECK_FALSE(cache.Cached(1));
    CHECK(*cache.Get(2) == 200);
}

TEST_CASE("LargeCacheStaysWithinCapacity", "[CapacityTest]")
{
    constexpr std::size_t CAPACITY = 100;
    caches::cache<int, int> cache{CAPACITY};

    for (int i = 0; i < 1000; ++i)
    {
        cache.Put(i, i * 10);
        CHECK(cache.Size() <= CAPACITY);
    }

    CHECK(cache.Size() == CAPACITY);
}

TEST_CASE("ConstructorWithPolicy", "[coverage]")
{
    using cache_t = caches::cache<std::string, int, caches::LRU>;
    using policy_t = cache_t::policy_type;

    policy_t policy;
    cache_t cache{10, std::move(policy)};

    REQUIRE(cache.MaxSize() == 10);
    REQUIRE(cache.Size() == 0);

    cache.Put("test", 1);
    REQUIRE(*cache.Get("test") == 1);
}

TEST_CASE("ConstructorWithPolicyThrows", "[coverage]")
{
    using cache_t = caches::cache<std::string, int, caches::LRU>;
    using policy_t = cache_t::policy_type;

    policy_t policy;
    REQUIRE_THROWS_AS(cache_t(0, std::move(policy)), std::invalid_argument);
}

TEST_CASE("ConstructorWithPolicyAllPolicies", "[coverage]")
{
    // Test FIFO with policy constructor
    {
        using cache_t = caches::cache<std::string, int, caches::FIFO>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        cache_t cache{10, std::move(policy)};
        REQUIRE(cache.MaxSize() == 10);
        REQUIRE(cache.Size() == 0);
        cache.Put("test", 1);
        REQUIRE(*cache.Get("test") == 1);
    }
    // Test LFU with policy constructor
    {
        using cache_t = caches::cache<std::string, int, caches::LFU>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        cache_t cache{10, std::move(policy)};
        REQUIRE(cache.MaxSize() == 10);
        REQUIRE(cache.Size() == 0);
        cache.Put("test", 1);
        REQUIRE(*cache.Get("test") == 1);
    }
    // Test NoEviction with policy constructor
    {
        using cache_t = caches::cache<std::string, int, caches::NoEviction>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        cache_t cache{10, std::move(policy)};
        REQUIRE(cache.MaxSize() == 10);
        REQUIRE(cache.Size() == 0);
        cache.Put("test", 1);
        REQUIRE(*cache.Get("test") == 1);
    }
}

TEST_CASE("ConstructorWithPolicyThrowsAllPolicies", "[coverage]")
{
    // FIFO
    {
        using cache_t = caches::cache<std::string, int, caches::FIFO>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        REQUIRE_THROWS_AS(cache_t(0, std::move(policy)), std::invalid_argument);
    }
    // LFU
    {
        using cache_t = caches::cache<std::string, int, caches::LFU>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        REQUIRE_THROWS_AS(cache_t(0, std::move(policy)), std::invalid_argument);
    }
    // NoEviction
    {
        using cache_t = caches::cache<std::string, int, caches::NoEviction>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        REQUIRE_THROWS_AS(cache_t(0, std::move(policy)), std::invalid_argument);
    }
}

template <template <typename, typename> class Policy>
void TestPolicyRuleOfFive()
{
    using key_type = std::string;
    using traits_type = caches::key_traits<key_type>;
    using policy_t = Policy<key_type, traits_type>;

    // Default construction
    policy_t p1;

    // Copy construction
    policy_t p2(p1);

    // Copy assignment
    policy_t p3;
    p3 = p1;

    // Move construction
    policy_t p4(std::move(p2));

    // Move assignment
    policy_t p5;
    p5 = std::move(p3);

    // Basic operation to ensure valid state after moves
    p5.Insert("test");
    p5.Touch("test");
    p5.Erase("test");
}

TEST_CASE("PolicyRuleOfFive", "[coverage]")
{
    TestPolicyRuleOfFive<caches::LRU>();
    TestPolicyRuleOfFive<caches::FIFO>();
    TestPolicyRuleOfFive<caches::LFU>();
    TestPolicyRuleOfFive<caches::NoEviction>();
}

TEST_CASE("LRUEvictionWithPhmap", "[CustomHashMapTest]")
{
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A");
    cache.Put("C", 3);

    CHECK(cache.Cached("A"));
    CHECK_FALSE(cache.Cached("B"));
    CHECK(cache.Cached("C"));
}

TEST_CASE("FIFOEvictionWithPhmap", "[CustomHashMapTest]")
{
    caches::cache<std::string, int, caches::FIFO, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A");
    cache.Put("C", 3);

    CHECK_FALSE(cache.Cached("A"));
    CHECK(cache.Cached("B"));
    CHECK(cache.Cached("C"));
}

TEST_CASE("LFUEvictionWithPhmap", "[CustomHashMapTest]")
{
    caches::cache<std::string, int, caches::LFU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A");
    cache.Get("A");
    cache.Get("A");
    cache.Put("C", 3);

    CHECK(cache.Cached("A"));
    CHECK_FALSE(cache.Cached("B"));
    CHECK(cache.Cached("C"));
}

TEST_CASE("CustomKeyWithPhmap", "[CustomHashMapTest]")
{
    using my_traits = caches::make_traits<explicit_traits_test::ExternalKeyHash,
                                          explicit_traits_test::ExternalKeyEqual>;

    caches::cache<explicit_traits_test::ExternalKey, int, caches::LRU, my_traits,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{10};

    cache.Put(explicit_traits_test::ExternalKey{1.5}, 100);
    cache.Put(explicit_traits_test::ExternalKey{2.5}, 200);

    CHECK(*cache.Get(explicit_traits_test::ExternalKey{1.5}) == 100);
    CHECK(*cache.Get(explicit_traits_test::ExternalKey{2.5}) == 200);
}

TEST_CASE("AllOperationsWithPhmap", "[CustomHashMapTest]")
{
    caches::cache<int, std::string, caches::LRU, caches::key_traits<int>,
                  caches::default_wrapper<std::string>, phmap_node_hash_map>
        cache{5};

    cache.Put(1, "one");
    cache.Put(2, "two");
    cache.Put(3, "three");

    CHECK(*cache.Get(1) == "one");
    CHECK(*cache.Get(2) == "two");
    CHECK(*cache.Get(3) == "three");

    auto result = cache.TryGet(1);
    CHECK(result.second);
    CHECK(*result.first == "one");

    auto missing = cache.TryGet(99);
    CHECK_FALSE(missing.second);
    CHECK(cache.Cached(1));
    CHECK_FALSE(cache.Cached(99));
    CHECK(cache.Size() == 3);
    CHECK(cache.Remove(2));
    CHECK_FALSE(cache.Cached(2));
    CHECK(cache.Size() == 2);
    cache.Clear();
    CHECK(cache.Empty());
    CHECK(cache.Size() == 0);
}

TEST_CASE("WorksWithPhmapFlatHashMap", "[CustomHashMapTest]")
{
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_flat_hash_map>
        cache{10};

    cache.Put("key1", 100);
    cache.Put("key2", 200);

    CHECK(*cache.Get("key1") == 100);
    CHECK(*cache.Get("key2") == 200);
    CHECK(cache.Size() == 2);
}

TEST_CASE("CapacityWithPhmap", "[CustomHashMapTest]")
{
    constexpr std::size_t CAPACITY = 50;
    caches::cache<int, int, caches::LRU, caches::key_traits<int>, caches::default_wrapper<int>,
                  phmap_node_hash_map>
        cache{CAPACITY};

    for (int i = 0; i < 500; ++i)
    {
        cache.Put(i, i * 10);
        CHECK(cache.Size() <= CAPACITY);
    }

    CHECK(cache.Size() == CAPACITY);
}

TEST_CASE("CallbackWithPhmapBackendLRU", "[CustomHashMapTest][OnEraseCallbackTest]")
{
    int callback_count = 0;
    std::string last_evicted;

    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2, std::hash<std::string>{}, std::equal_to<std::string>{},
              std::allocator<std::string>{}, [&](const std::string &key, const auto &)
              {
                  callback_count++;
                  last_evicted = key;
              }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A");    // Touch A
    cache.Put("C", 3); // Should evict B

    CHECK(callback_count == 1);
    CHECK(last_evicted == "B");
}

TEST_CASE("CallbackWithPhmapBackendFIFO", "[CustomHashMapTest][OnEraseCallbackTest]")
{
    int callback_count = 0;
    std::string last_evicted;

    caches::cache<std::string, int, caches::FIFO, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2, std::hash<std::string>{}, std::equal_to<std::string>{},
              std::allocator<std::string>{}, [&](const std::string &key, const auto &)
              {
                  callback_count++;
                  last_evicted = key;
              }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3); // Should evict A (FIFO)

    CHECK(callback_count == 1);
    CHECK(last_evicted == "A");
}

TEST_CASE("CallbackWithPhmapBackendLFU", "[CustomHashMapTest][OnEraseCallbackTest]")
{
    int callback_count = 0;

    caches::cache<std::string, int, caches::LFU, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2, std::hash<std::string>{}, std::equal_to<std::string>{},
              std::allocator<std::string>{},
              [&](const std::string &, const auto &) { callback_count++; }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Get("A");
    cache.Get("A");
    cache.Put("C", 3); // Should evict B (LFU)

    CHECK(callback_count == 1);
    CHECK_FALSE(cache.Cached("B"));
}

TEST_CASE("CallbackWithPhmapBackendNoEviction", "[CustomHashMapTest][OnEraseCallbackTest]")
{
    int callback_count = 0;

    caches::cache<std::string, int, caches::NoEviction, caches::key_traits<std::string>,
                  caches::default_wrapper<int>, phmap_node_hash_map>
        cache{2, std::hash<std::string>{}, std::equal_to<std::string>{},
              std::allocator<std::string>{},
              [&](const std::string &, const auto &) { callback_count++; }};

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3); // Evicts some entry

    CHECK(callback_count == 1);
    CHECK(cache.Size() == 2);
}
