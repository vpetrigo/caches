#include "cache.hpp"
#include "lru_cache_policy.hpp"

#include <gtest/gtest.h>
#ifdef CUSTOM_HASHMAP
#include <parallel_hashmap/phmap.h>
#endif /* CUSTOM_HASHMAP */

#ifndef CUSTOM_HASHMAP
template <typename Key, typename Value>
using lru_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::LRUCachePolicy>;
#else
template <typename Key, typename Value>
using lru_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::LRUCachePolicy,
                                                       phmap::node_hash_map<Key, caches::WrappedValue<Value>>>;
#endif /* CUSTOM_HASHMAP */

TEST(CacheTest, SimplePut)
{
    lru_cache_t<std::string, int> cache(1);

    auto val = cache.Put("test", 666);
    EXPECT_EQ(*val, 666);

    EXPECT_EQ(*cache.Get("test"), 666);
}

TEST(CacheTest, PutWithUpdate)
{
    constexpr std::size_t TEST_CASE = 4;
    lru_cache_t<std::string, std::size_t> cache{TEST_CASE};

    for (size_t i = 0; i < TEST_CASE; ++i)
    {
        cache.Put(std::to_string(i), i);

        const auto value = cache.Get(std::to_string(i));
        ASSERT_EQ(i, *value);
    }

    for (size_t i = 0; i < TEST_CASE; ++i)
    {
        ASSERT_TRUE(cache.Cached(std::to_string(i)));
        cache.Put(std::to_string(i), i * 10);

        const auto value = cache.Get(std::to_string(i));
        ASSERT_EQ(i * 10, *value);
    }
}

TEST(CacheTest, MissingValue)
{
    lru_cache_t<std::string, int> cache(1);

    EXPECT_THROW(cache.Get("test"), std::range_error);
}

TEST(CacheTest, KeepsAllValuesWithinCapacity)
{
    constexpr int CACHE_CAP = 50;
    const int TEST_RECORDS = 100;
    lru_cache_t<int, int> cache(CACHE_CAP);

    for (int i = 0; i < TEST_RECORDS; ++i)
    {
        cache.Put(i, i);
    }

    for (int i = 0; i < TEST_RECORDS - CACHE_CAP; ++i)
    {
        EXPECT_THROW(cache.Get(i), std::range_error);
    }

    for (int i = TEST_RECORDS - CACHE_CAP; i < TEST_RECORDS; ++i)
    {
        EXPECT_EQ(i, *cache.Get(i));
    }
}

TEST(LRUCache, Remove_Test)
{
    constexpr std::size_t TEST_SIZE = 10;
    lru_cache_t<std::string, std::size_t> fc(TEST_SIZE);

    for (std::size_t i = 0; i < TEST_SIZE; ++i)
    {
        fc.Put(std::to_string(i), i);
    }

    EXPECT_EQ(fc.Size(), TEST_SIZE);

    for (std::size_t i = 0; i < TEST_SIZE; ++i)
    {
        EXPECT_TRUE(fc.Remove(std::to_string(i)));
    }

    EXPECT_EQ(fc.Size(), 0);

    for (std::size_t i = 0; i < TEST_SIZE; ++i)
    {
        EXPECT_FALSE(fc.Remove(std::to_string(i)));
    }
}

TEST(LRUCache, CachedCheck)
{
    constexpr std::size_t TEST_SUITE = 4;
    lru_cache_t<std::string, std::size_t> cache(TEST_SUITE);

    for (std::size_t i = 0; i < TEST_SUITE; ++i)
    {
        cache.Put(std::to_string(i), i);
    }

    for (std::size_t i = 0; i < TEST_SUITE; ++i)
    {
        EXPECT_TRUE(cache.Cached(std::to_string(i)));
    }

    for (std::size_t i = TEST_SUITE; i < TEST_SUITE * 2; ++i)
    {
        EXPECT_FALSE(cache.Cached(std::to_string(i)));
    }
}

TEST(LRUCache, ConstructCache)
{
    EXPECT_THROW((lru_cache_t<std::string, std::size_t>(0)), std::invalid_argument);
    EXPECT_NO_THROW((lru_cache_t<std::string, std::size_t>(1024)));
}

TEST(LRUCache, TryGet)
{
    constexpr std::size_t TEST_CASE{10};
    lru_cache_t<std::string, std::size_t> cache{TEST_CASE};

    for (std::size_t i = 0; i < TEST_CASE; ++i)
    {
        cache.Put(std::to_string(i), i);
    }

    for (std::size_t i = 0; i < TEST_CASE; ++i)
    {
        auto element = cache.TryGet(std::to_string(i));
        EXPECT_TRUE(element.second);
        EXPECT_EQ(*element.first, i);
    }

    for (std::size_t i = TEST_CASE; i < TEST_CASE * 2; ++i)
    {
        auto element = cache.TryGet(std::to_string(i));
        EXPECT_FALSE(element.second);
    }
}

TEST(LRUCache, GetWithReplacement)
{
    lru_cache_t<std::string, std::size_t> cache{2};

    cache.Put("1", 1);
    cache.Put("2", 2);

    auto element1 = cache.Get("1");
    auto element2 = cache.Get("2");
    EXPECT_EQ(*element1, 1);
    EXPECT_EQ(*element2, 2);
    cache.Put("3", 3);
    auto element3 = cache.Get("3");
    EXPECT_EQ(*element3, 3);

    std::string replaced_key;

    for (size_t i = 1; i <= 2; ++i) {
        const auto key = std::to_string(i);

        if (!cache.Cached(key)) {
            replaced_key = key;
        }
    }

    EXPECT_FALSE(cache.Cached(replaced_key));
    EXPECT_FALSE(cache.TryGet(replaced_key).second);
    EXPECT_THROW(cache.Get(replaced_key), std::range_error);
    EXPECT_EQ(*element1, 1);
    EXPECT_EQ(*element2, 2);
    EXPECT_EQ(*element3, 3);
}

TEST(LRUCache, InvalidSize)
{
    using test_type = lru_cache_t<std::string, int>;
    EXPECT_THROW(test_type cache{0}, std::invalid_argument);
}
