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

TEST_CASE("PolicyEraseNonExistentKey", "[coverage][branch]")
{
    {
        caches::LRU<std::string> policy;
        policy.Erase("nonexistent"); // Should not crash
        policy.Insert("key1");
        policy.Erase("other_nonexistent");
        policy.Erase("key1");
    }

    {
        caches::FIFO<std::string> policy;
        policy.Erase("nonexistent");
        policy.Insert("key1");
        policy.Erase("other_nonexistent");
        policy.Erase("key1");
    }

    {
        caches::LFU<std::string> policy;
        policy.Erase("nonexistent");
        policy.Insert("key1");
        policy.Erase("other_nonexistent");
        policy.Erase("key1");
    }

    {
        caches::NoEviction<std::string> policy;
        policy.Erase("nonexistent");
        policy.Insert("key1");
        policy.Erase("other_nonexistent");
        policy.Erase("key1");
    }
}

TEST_CASE("PolicyEraseNonExistentKeyIntType", "[coverage][branch]")
{
    // Same test but with int keys for different template instantiation
    {
        caches::LRU<int> policy;
        policy.Erase(999);
        policy.Insert(1);
        policy.Erase(888);
        policy.Erase(1);
    }

    {
        caches::FIFO<int> policy;
        policy.Erase(999);
        policy.Insert(1);
        policy.Erase(888);
        policy.Erase(1);
    }

    {
        caches::LFU<int> policy;
        policy.Erase(999);
        policy.Insert(1);
        policy.Erase(888);
        policy.Erase(1);
    }

    {
        caches::NoEviction<int> policy;
        policy.Erase(999);
        policy.Insert(1);
        policy.Erase(888);
        policy.Erase(1);
    }
}

TEST_CASE("ZeroSizeCacheThrowsIntKey", "[coverage][branch]")
{
    CHECK_THROWS_AS((caches::cache<int, int, caches::LRU>{0}), std::invalid_argument);
    CHECK_THROWS_AS((caches::cache<int, int, caches::FIFO>{0}), std::invalid_argument);
    CHECK_THROWS_AS((caches::cache<int, int, caches::LFU>{0}), std::invalid_argument);
    CHECK_THROWS_AS((caches::cache<int, int, caches::NoEviction>{0}), std::invalid_argument);
}

TEST_CASE("ZeroSizeCacheThrowsWithPolicyIntKey", "[coverage][branch]")
{
    {
        using cache_t = caches::cache<int, int, caches::LRU>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        CHECK_THROWS_AS(cache_t(0, std::move(policy)), std::invalid_argument);
    }
    {
        using cache_t = caches::cache<int, int, caches::FIFO>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        CHECK_THROWS_AS(cache_t(0, std::move(policy)), std::invalid_argument);
    }
    {
        using cache_t = caches::cache<int, int, caches::LFU>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        CHECK_THROWS_AS(cache_t(0, std::move(policy)), std::invalid_argument);
    }
    {
        using cache_t = caches::cache<int, int, caches::NoEviction>;
        using policy_t = cache_t::policy_type;
        policy_t policy;
        CHECK_THROWS_AS(cache_t(0, std::move(policy)), std::invalid_argument);
    }
}

TEST_CASE("UpdateExistingKeyAllPolicies", "[coverage][branch]")
{
    {
        caches::cache<int, int, caches::LRU> cache{5};
        cache.Put(1, 100);
        cache.Put(1, 200); // Update - triggers Touch and value update
        CHECK(*cache.Get(1) == 200);
        CHECK(cache.Size() == 1);
    }

    {
        caches::cache<int, int, caches::FIFO> cache{5};
        cache.Put(1, 100);
        cache.Put(1, 200);
        CHECK(*cache.Get(1) == 200);
        CHECK(cache.Size() == 1);
    }

    {
        caches::cache<int, int, caches::LFU> cache{5};
        cache.Put(1, 100);
        cache.Put(1, 200);
        CHECK(*cache.Get(1) == 200);
        CHECK(cache.Size() == 1);
    }

    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        cache.Put(1, 100);
        cache.Put(1, 200);
        CHECK(*cache.Get(1) == 200);
        CHECK(cache.Size() == 1);
    }
}

TEST_CASE("UpdateExistingKeyWithEviction", "[coverage][branch]")
{
    // Test updating a key when cache is at capacity (both branches in Put)

    {
        caches::cache<int, int, caches::LRU> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        // Cache full, now update existing key (should NOT evict)
        cache.Put(1, 150);
        CHECK(*cache.Get(1) == 150);
        CHECK(*cache.Get(2) == 200);
        CHECK(cache.Size() == 2);
    }

    {
        caches::cache<int, int, caches::LRU> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Get(1);      // Touch 1
        cache.Put(3, 300); // New key - evicts 2
        CHECK(cache.Cached(1));
        CHECK_FALSE(cache.Cached(2));
        CHECK(cache.Cached(3));
    }
}

TEST_CASE("GetTouchesKeyAllPolicies", "[coverage][branch]")
{
    // Covers the Get path that touches the key

    {
        caches::cache<int, int, caches::LRU> cache{3};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(3, 300);
        cache.Get(1);      // Touch key 1
        cache.Put(4, 400); // Evict key 2 (LRU)
        CHECK(cache.Cached(1));
        CHECK_FALSE(cache.Cached(2));
    }

    {
        caches::cache<int, int, caches::FIFO> cache{3};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(3, 300);
        cache.Get(1);      // Touch (no effect for FIFO)
        cache.Put(4, 400); // Evict key 1 (first in)
        CHECK_FALSE(cache.Cached(1));
        CHECK(cache.Cached(2));
    }

    {
        caches::cache<int, int, caches::LFU> cache{3};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(3, 300);
        cache.Get(1);
        cache.Get(1);      // Key 1 has high frequency
        cache.Put(4, 400); // Evict key 2 or 3 (low frequency)
        CHECK(cache.Cached(1));
    }
}

TEST_CASE("TryGetTouchesKeyAllPolicies", "[coverage][branch]")
{
    // Tests TryGet path for existing keys
    {
        caches::cache<int, int, caches::LRU> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        auto [ptr, found] = cache.TryGet(1); // Touch 1
        CHECK(found);
        CHECK(*ptr == 100);
        cache.Put(3, 300); // Should evict 2
        CHECK(cache.Cached(1));
        CHECK_FALSE(cache.Cached(2));
    }

    {
        caches::cache<int, int, caches::FIFO> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        auto [ptr, found] = cache.TryGet(1);
        CHECK(found);
        cache.Put(3, 300); // Should evict 1 (FIFO)
        CHECK_FALSE(cache.Cached(1));
    }

    {
        caches::cache<int, int, caches::LFU> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.TryGet(1);
        cache.TryGet(1);
        cache.TryGet(1);   // High frequency
        cache.Put(3, 300); // Evict 2
        CHECK(cache.Cached(1));
        CHECK_FALSE(cache.Cached(2));
    }
}

TEST_CASE("TryGetMissingKeyAllPolicies", "[coverage][branch]")
{
    // Tests TryGet path for missing keys (early return branch)
    {
        caches::cache<int, int, caches::LRU> cache{5};
        auto [ptr, found] = cache.TryGet(999);
        CHECK_FALSE(found);
        CHECK(ptr == nullptr);
    }
    {
        caches::cache<int, int, caches::FIFO> cache{5};
        auto [ptr, found] = cache.TryGet(999);
        CHECK_FALSE(found);
    }
    {
        caches::cache<int, int, caches::LFU> cache{5};
        auto [ptr, found] = cache.TryGet(999);
        CHECK_FALSE(found);
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        auto [ptr, found] = cache.TryGet(999);
        CHECK_FALSE(found);
    }
}

TEST_CASE("CachedCheckBothBranches", "[coverage][branch]")
{
    // Tests Cached() for both existing and non-existing keys

    caches::cache<int, int, caches::LRU> cache{5};

    // Key doesn't exist
    CHECK_FALSE(cache.Cached(1));

    // Key exists
    cache.Put(1, 100);
    CHECK(cache.Cached(1));

    // Another key doesn't exist
    CHECK_FALSE(cache.Cached(2));
}

TEST_CASE("RemoveKeyAllPolicies", "[coverage][branch]")
{
    // Tests Remove() for existing keys

    {
        caches::cache<int, int, caches::LRU> cache{5};
        cache.Put(1, 100);
        CHECK(cache.Remove(1));
        CHECK_FALSE(cache.Cached(1));
    }
    {
        caches::cache<int, int, caches::FIFO> cache{5};
        cache.Put(1, 100);
        CHECK(cache.Remove(1));
    }
    {
        caches::cache<int, int, caches::LFU> cache{5};
        cache.Put(1, 100);
        CHECK(cache.Remove(1));
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        cache.Put(1, 100);
        CHECK(cache.Remove(1));
    }
}

TEST_CASE("RemoveMissingKeyAllPolicies", "[coverage][branch]")
{
    // Tests Remove() for non-existing keys (line 290 return false branch)

    {
        caches::cache<int, int, caches::LRU> cache{5};
        CHECK_FALSE(cache.Remove(999));
    }
    {
        caches::cache<int, int, caches::FIFO> cache{5};
        CHECK_FALSE(cache.Remove(999));
    }
    {
        caches::cache<int, int, caches::LFU> cache{5};
        CHECK_FALSE(cache.Remove(999));
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        CHECK_FALSE(cache.Remove(999));
    }
}

TEST_CASE("ClearEmptyCacheAllPolicies", "[coverage][branch]")
{
    // Tests Clear() on empty cache (loop doesn't execute)

    {
        caches::cache<int, int, caches::LRU> cache{5};
        cache.Clear();
        CHECK(cache.Empty());
    }
    {
        caches::cache<int, int, caches::FIFO> cache{5};
        cache.Clear();
        CHECK(cache.Empty());
    }
    {
        caches::cache<int, int, caches::LFU> cache{5};
        cache.Clear();
        CHECK(cache.Empty());
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        cache.Clear();
        CHECK(cache.Empty());
    }
}

TEST_CASE("ClearNonEmptyCacheAllPolicies", "[coverage][branch]")
{
    // Tests Clear() on non-empty cache (loop executes)

    {
        caches::cache<int, int, caches::LRU> cache{5};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        CHECK(cache.Empty());
    }
    {
        caches::cache<int, int, caches::FIFO> cache{5};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        CHECK(cache.Empty());
    }
    {
        caches::cache<int, int, caches::LFU> cache{5};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        CHECK(cache.Empty());
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        CHECK(cache.Empty());
    }
}

TEST_CASE("EvictionCallbackWithIntKey", "[coverage][branch]")
{
    // Tests eviction callback invocation with int keys

    int count = 0;
    int last_key = -1;
    caches::cache<int, int, caches::LRU> cache{2, std::hash<int>{}, std::equal_to<int>{},
                                               std::allocator<int>{},
                                               [&](const int &key, const auto &)
                                               {
                                                   count++;
                                                   last_key = key;
                                               }};

    cache.Put(1, 100);
    cache.Put(2, 200);
    cache.Get(1);      // Touch 1
    cache.Put(3, 300); // Evict 2

    CHECK(count == 1);
    CHECK(last_key == 2);
}

TEST_CASE("EmptyCheckAllPolicies", "[coverage][branch]")
{
    // Tests Empty() for both empty and non-empty states

    {
        caches::cache<int, int, caches::LRU> cache{5};
        CHECK(cache.Empty());
        cache.Put(1, 100);
        CHECK_FALSE(cache.Empty());
    }
    {
        caches::cache<int, int, caches::FIFO> cache{5};
        CHECK(cache.Empty());
        cache.Put(1, 100);
        CHECK_FALSE(cache.Empty());
    }
    {
        caches::cache<int, int, caches::LFU> cache{5};
        CHECK(cache.Empty());
        cache.Put(1, 100);
        CHECK_FALSE(cache.Empty());
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        CHECK(cache.Empty());
        cache.Put(1, 100);
        CHECK_FALSE(cache.Empty());
    }
}

TEST_CASE("MaxSizeAllPolicies", "[coverage][branch]")
{
    // Tests MaxSize() returns correct value

    {
        caches::cache<int, int, caches::LRU> cache{42};
        CHECK(cache.MaxSize() == 42);
    }
    {
        caches::cache<int, int, caches::FIFO> cache{42};
        CHECK(cache.MaxSize() == 42);
    }
    {
        caches::cache<int, int, caches::LFU> cache{42};
        CHECK(cache.MaxSize() == 42);
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{42};
        CHECK(cache.MaxSize() == 42);
    }
}

TEST_CASE("MultipleEvictionsAllPolicies", "[coverage][branch]")
{
    // Tests multiple evictions in sequence

    {
        caches::cache<int, int, caches::LRU> cache{2};
        for (int i = 0; i < 10; i++)
        {
            cache.Put(i, i * 10);
        }
        CHECK(cache.Size() == 2);
        CHECK(cache.Cached(8));
        CHECK(cache.Cached(9));
    }

    {
        caches::cache<int, int, caches::FIFO> cache{2};
        for (int i = 0; i < 10; i++)
        {
            cache.Put(i, i * 10);
        }
        CHECK(cache.Size() == 2);
    }

    {
        caches::cache<int, int, caches::LFU> cache{2};
        for (int i = 0; i < 10; i++)
        {
            cache.Put(i, i * 10);
        }
        CHECK(cache.Size() == 2);
    }

    {
        caches::cache<int, int, caches::NoEviction> cache{2};
        for (int i = 0; i < 10; i++)
        {
            cache.Put(i, i * 10);
        }
        CHECK(cache.Size() == 2);
    }
}

// Additional edge case tests for branch coverage

TEST_CASE("CacheWithStringValueAllPolicies", "[coverage][branch]")
{
    // Tests with string values to cover more template instantiations
    {
        caches::cache<int, std::string, caches::LRU> cache{3};
        cache.Put(1, "one");
        cache.Put(2, "two");
        CHECK(*cache.Get(1) == "one");
        CHECK(*cache.Get(2) == "two");
        cache.Put(1, "ONE"); // Update
        CHECK(*cache.Get(1) == "ONE");
    }
    {
        caches::cache<int, std::string, caches::FIFO> cache{3};
        cache.Put(1, "one");
        cache.Put(2, "two");
        CHECK(*cache.Get(1) == "one");
        cache.Put(1, "ONE");
        CHECK(*cache.Get(1) == "ONE");
    }
    {
        caches::cache<int, std::string, caches::LFU> cache{3};
        cache.Put(1, "one");
        cache.Put(2, "two");
        CHECK(*cache.Get(1) == "one");
        cache.Put(1, "ONE");
        CHECK(*cache.Get(1) == "ONE");
    }
    {
        caches::cache<int, std::string, caches::NoEviction> cache{3};
        cache.Put(1, "one");
        cache.Put(2, "two");
        CHECK(*cache.Get(1) == "one");
        cache.Put(1, "ONE");
        CHECK(*cache.Get(1) == "ONE");
    }
}

TEST_CASE("GetThrowsAllPolicies", "[coverage][branch]")
{
    // Tests Get() throwing for missing keys across all policies
    {
        caches::cache<int, int, caches::LRU> cache{5};
        CHECK_THROWS_AS(cache.Get(999), std::range_error);
    }
    {
        caches::cache<int, int, caches::FIFO> cache{5};
        CHECK_THROWS_AS(cache.Get(999), std::range_error);
    }
    {
        caches::cache<int, int, caches::LFU> cache{5};
        CHECK_THROWS_AS(cache.Get(999), std::range_error);
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        CHECK_THROWS_AS(cache.Get(999), std::range_error);
    }
}

TEST_CASE("CacheFullThenUpdateAllPolicies", "[coverage][branch]")
{
    // Tests updating a key when cache is exactly at capacity
    {
        caches::cache<int, int, caches::LRU> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        CHECK(cache.Size() == 2);
        cache.Put(1, 150); // Update, not evict
        CHECK(cache.Size() == 2);
        CHECK(*cache.Get(1) == 150);
        CHECK(*cache.Get(2) == 200);
    }
    {
        caches::cache<int, int, caches::FIFO> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(1, 150);
        CHECK(cache.Size() == 2);
        CHECK(*cache.Get(1) == 150);
    }
    {
        caches::cache<int, int, caches::LFU> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(1, 150);
        CHECK(cache.Size() == 2);
        CHECK(*cache.Get(1) == 150);
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(1, 150);
        CHECK(cache.Size() == 2);
        CHECK(*cache.Get(1) == 150);
    }
}

TEST_CASE("SingleEntryOperationsAllPolicies", "[coverage][branch]")
{
    // Tests all operations on single-entry cache
    {
        caches::cache<int, int, caches::LRU> cache{1};
        CHECK(cache.Empty());
        cache.Put(1, 100);
        CHECK_FALSE(cache.Empty());
        CHECK(cache.Size() == 1);
        CHECK(*cache.Get(1) == 100);
        auto [ptr, found] = cache.TryGet(1);
        CHECK(found);
        cache.Put(1, 200);
        CHECK(*cache.Get(1) == 200);
        CHECK(cache.Remove(1));
        CHECK(cache.Empty());
    }
    {
        caches::cache<int, int, caches::FIFO> cache{1};
        cache.Put(1, 100);
        CHECK(*cache.Get(1) == 100);
        cache.Put(1, 200);
        CHECK(*cache.Get(1) == 200);
        CHECK(cache.Remove(1));
    }
    {
        caches::cache<int, int, caches::LFU> cache{1};
        cache.Put(1, 100);
        CHECK(*cache.Get(1) == 100);
        cache.Put(1, 200);
        CHECK(*cache.Get(1) == 200);
        CHECK(cache.Remove(1));
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{1};
        cache.Put(1, 100);
        CHECK(*cache.Get(1) == 100);
        cache.Put(1, 200);
        CHECK(*cache.Get(1) == 200);
        CHECK(cache.Remove(1));
    }
}

TEST_CASE("EvictThenInsertSameKeyAllPolicies", "[coverage][branch]")
{
    // Tests evicting a key then re-inserting it
    {
        caches::cache<int, int, caches::LRU> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(3, 300); // Evict 1
        CHECK_FALSE(cache.Cached(1));
        cache.Put(1, 101); // Re-insert 1
        CHECK(cache.Cached(1));
        CHECK(*cache.Get(1) == 101);
    }
    {
        caches::cache<int, int, caches::FIFO> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(3, 300);
        cache.Put(1, 101);
        CHECK(cache.Cached(1));
    }
    {
        caches::cache<int, int, caches::LFU> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(3, 300);
        cache.Put(1, 101);
        CHECK(cache.Cached(1));
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{2};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(3, 300);
        // NoEviction may evict any key
        cache.Put(1, 101);
    }
}

TEST_CASE("ClearThenReuseAllPolicies", "[coverage][branch]")
{
    // Tests clearing and reusing the cache
    {
        caches::cache<int, int, caches::LRU> cache{3};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        CHECK(cache.Empty());
        cache.Put(3, 300);
        CHECK(cache.Size() == 1);
        CHECK(*cache.Get(3) == 300);
    }
    {
        caches::cache<int, int, caches::FIFO> cache{3};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        cache.Put(3, 300);
        CHECK(*cache.Get(3) == 300);
    }
    {
        caches::cache<int, int, caches::LFU> cache{3};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        cache.Put(3, 300);
        CHECK(*cache.Get(3) == 300);
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{3};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        cache.Put(3, 300);
        CHECK(*cache.Get(3) == 300);
    }
}

TEST_CASE("TryGetAndGetOnSameKeyAllPolicies", "[coverage][branch]")
{
    // Tests both TryGet and Get on same key
    {
        caches::cache<int, int, caches::LRU> cache{5};
        cache.Put(1, 100);
        auto [ptr1, found1] = cache.TryGet(1);
        CHECK(found1);
        CHECK(*ptr1 == 100);
        auto ptr2 = cache.Get(1);
        CHECK(*ptr2 == 100);
    }
    {
        caches::cache<int, int, caches::FIFO> cache{5};
        cache.Put(1, 100);
        auto [ptr, found] = cache.TryGet(1);
        CHECK(found);
        CHECK(*cache.Get(1) == 100);
    }
    {
        caches::cache<int, int, caches::LFU> cache{5};
        cache.Put(1, 100);
        auto [ptr, found] = cache.TryGet(1);
        CHECK(found);
        CHECK(*cache.Get(1) == 100);
    }
    {
        caches::cache<int, int, caches::NoEviction> cache{5};
        cache.Put(1, 100);
        auto [ptr, found] = cache.TryGet(1);
        CHECK(found);
        CHECK(*cache.Get(1) == 100);
    }
}

TEST_CASE("CallbackInvokedOnClearAllPolicies", "[coverage][branch]")
{
    // Tests callback invocation during Clear()
    {
        int count = 0;
        caches::cache<int, int, caches::LRU> cache{5, std::hash<int>{}, std::equal_to<int>{},
                                                   std::allocator<int>{},
                                                   [&](const int &, const auto &) { count++; }};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Put(3, 300);
        cache.Clear();
        CHECK(count == 3);
    }
    {
        int count = 0;
        caches::cache<int, int, caches::FIFO> cache{5, std::hash<int>{}, std::equal_to<int>{},
                                                    std::allocator<int>{},
                                                    [&](const int &, const auto &) { count++; }};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        CHECK(count == 2);
    }
    {
        int count = 0;
        caches::cache<int, int, caches::LFU> cache{5, std::hash<int>{}, std::equal_to<int>{},
                                                   std::allocator<int>{},
                                                   [&](const int &, const auto &) { count++; }};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        CHECK(count == 2);
    }
    {
        int count = 0;
        caches::cache<int, int, caches::NoEviction> cache{
            5, std::hash<int>{}, std::equal_to<int>{}, std::allocator<int>{},
            [&](const int &, const auto &) { count++; }};
        cache.Put(1, 100);
        cache.Put(2, 200);
        cache.Clear();
        CHECK(count == 2);
    }
}

TEST_CASE("CallbackInvokedOnDestructorAllPolicies", "[coverage][branch]")
{
    // Tests callback invocation during destructor
    int count = 0;
    {
        caches::cache<int, int, caches::LRU> cache{5, std::hash<int>{}, std::equal_to<int>{},
                                                   std::allocator<int>{},
                                                   [&](const int &, const auto &) { count++; }};
        cache.Put(1, 100);
        cache.Put(2, 200);
    }
    CHECK(count == 2);

    count = 0;
    {
        caches::cache<int, int, caches::FIFO> cache{5, std::hash<int>{}, std::equal_to<int>{},
                                                    std::allocator<int>{},
                                                    [&](const int &, const auto &) { count++; }};
        cache.Put(1, 100);
    }
    CHECK(count == 1);

    count = 0;
    {
        caches::cache<int, int, caches::LFU> cache{5, std::hash<int>{}, std::equal_to<int>{},
                                                   std::allocator<int>{},
                                                   [&](const int &, const auto &) { count++; }};
        cache.Put(1, 100);
    }
    CHECK(count == 1);

    count = 0;
    {
        caches::cache<int, int, caches::NoEviction> cache{
            5, std::hash<int>{}, std::equal_to<int>{}, std::allocator<int>{},
            [&](const int &, const auto &) { count++; }};
        cache.Put(1, 100);
    }
    CHECK(count == 1);
}
