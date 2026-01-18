#include "caches/cache.hpp"
#include "caches/lfu_cache_policy.hpp"

#include <gtest/gtest.h>
#ifdef CUSTOM_HASHMAP
#include <parallel_hashmap/phmap.h>
#endif /* CUSTOM_HASHMAP */

#ifndef CUSTOM_HASHMAP
template <typename Key, typename Value>
using lfu_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::LFUCachePolicy>;
#else
template <typename Key, typename Value>
using lfu_cache_t =
    typename caches::fixed_sized_cache<Key, Value, caches::LFUCachePolicy,
                                       phmap::node_hash_map<Key, caches::WrappedValue<Value>>>;
#endif /* CUSTOM_HASHMAP */

TEST(LFUCache, Simple_Test)
{
    constexpr size_t FIRST_FREQ = 10;
    constexpr size_t SECOND_FREQ = 9;
    constexpr size_t THIRD_FREQ = 8;
    lfu_cache_t<std::string, int> cache(3);

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);

    for (size_t i = 0; i < FIRST_FREQ; ++i)
    {
        EXPECT_EQ(*cache.Get("B"), 2);
    }

    for (size_t i = 0; i < SECOND_FREQ; ++i)
    {
        EXPECT_EQ(*cache.Get("C"), 3);
    }

    for (size_t i = 0; i < THIRD_FREQ; ++i)
    {
        EXPECT_EQ(*cache.Get("A"), 1);
    }

    cache.Put("D", 4);

    EXPECT_EQ(*cache.Get("B"), 2);
    EXPECT_EQ(*cache.Get("C"), 3);
    EXPECT_EQ(*cache.Get("D"), 4);
    EXPECT_THROW(cache.Get("A"), std::range_error);
}

TEST(LFUCache, Single_Slot)
{
    constexpr size_t TEST_SIZE = 5;
    lfu_cache_t<int, int> cache(1);

    cache.Put(1, 10);

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        cache.Put(1, static_cast<int>(i));
    }

    EXPECT_EQ(*cache.Get(1), 4);

    cache.Put(2, 20);

    EXPECT_THROW(cache.Get(1), std::range_error);
    EXPECT_EQ(*cache.Get(2), 20);
}

TEST(LFUCache, FrequencyIssue)
{
    constexpr size_t TEST_SIZE = 50;
    lfu_cache_t<int, int> cache(3);

    cache.Put(1, 10);
    cache.Put(2, 1);
    cache.Put(3, 2);

    // cache value with key '1' will have the counter 50
    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        EXPECT_NO_THROW(cache.Get(1));
    }

    cache.Put(4, 3);
    cache.Put(5, 4);

    EXPECT_EQ(*cache.Get(1), 10);
    EXPECT_EQ(*cache.Get(2), 1);
    EXPECT_EQ(*cache.Get(5), 4);
    EXPECT_THROW(cache.Get(3), std::range_error);
    EXPECT_THROW(cache.Get(4), std::range_error);

    cache.Put(6, 5);
    cache.Put(7, 6);

    EXPECT_EQ(*cache.Get(1), 10);
    EXPECT_EQ(*cache.Get(5), 4);
    EXPECT_EQ(*cache.Get(7), 6);
    EXPECT_THROW(cache.Get(3), std::range_error);
    EXPECT_THROW(cache.Get(6), std::range_error);
}

TEST(LFUCache, Remove_Test)
{
    constexpr std::size_t TEST_SIZE = 10;
    lfu_cache_t<std::string, std::size_t> fc(TEST_SIZE);

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

TEST(LFUCache, TryGet)
{
    constexpr std::size_t TEST_CASE{10};
    lfu_cache_t<std::string, std::size_t> cache{TEST_CASE};

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

TEST(LFUCache, GetWithReplacement)
{
    lfu_cache_t<std::string, std::size_t> cache{2};

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

    for (size_t i = 1; i <= 2; ++i)
    {
        const auto key = std::to_string(i);

        if (!cache.Cached(key))
        {
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

TEST(LFUCache, InvalidSize)
{
    using test_type = lfu_cache_t<std::string, int>;
    EXPECT_THROW(test_type cache{0}, std::invalid_argument);
}
