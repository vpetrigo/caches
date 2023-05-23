#include "cache.hpp"
#include "fifo_cache_policy.hpp"

#include <gtest/gtest.h>
#ifdef CUSTOM_HASHMAP
#include <parallel_hashmap/phmap.h>
#endif /* CUSTOM_HASHMAP */

#ifndef CUSTOM_HASHMAP
template <typename Key, typename Value>
using fifo_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::FIFOCachePolicy>;
#else
template <typename Key, typename Value>
using fifo_cache_t = typename caches::fixed_sized_cache<Key, Value, caches::FIFOCachePolicy,
                                                        phmap::node_hash_map<Key, Value>>;
#endif /* CUSTOM_HASHMAP */

TEST(FIFOCache, Simple_Test)
{
    fifo_cache_t<int, int> fc(2);

    fc.Put(1, 10);
    fc.Put(2, 20);

    EXPECT_EQ(fc.Size(), 2);
    EXPECT_EQ(fc.Get(1), 10);
    EXPECT_EQ(fc.Get(2), 20);

    fc.Put(1, 30);
    EXPECT_EQ(fc.Size(), 2);
    EXPECT_EQ(fc.Get(1), 30);

    fc.Put(3, 30);
    EXPECT_THROW(fc.Get(1), std::range_error);
    EXPECT_EQ(fc.Get(2), 20);
    EXPECT_EQ(fc.Get(3), 30);
}

TEST(FIFOCache, Missing_Value)
{
    fifo_cache_t<int, int> fc(2);

    fc.Put(1, 10);

    EXPECT_EQ(fc.Size(), 1);
    EXPECT_EQ(fc.Get(1), 10);
    EXPECT_THROW(fc.Get(2), std::range_error);
}

TEST(FIFOCache, Sequence_Test)
{
    constexpr int TEST_SIZE = 10;
    fifo_cache_t<std::string, int> fc(TEST_SIZE);

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        fc.Put(std::to_string('0' + i), static_cast<int>(i));
    }

    EXPECT_EQ(fc.Size(), TEST_SIZE);

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        EXPECT_EQ(fc.Get(std::to_string('0' + i)), i);
    }

    // replace a half
    for (size_t i = 0; i < TEST_SIZE / 2; ++i)
    {
        fc.Put(std::to_string('a' + i), static_cast<int>(i));
    }

    EXPECT_EQ(fc.Size(), TEST_SIZE);

    for (size_t i = 0; i < TEST_SIZE / 2; ++i)
    {
        EXPECT_THROW(fc.Get(std::to_string('0' + i)), std::range_error);
    }

    for (size_t i = 0; i < TEST_SIZE / 2; ++i)
    {
        EXPECT_EQ(fc.Get(std::to_string('a' + i)), i);
    }

    for (size_t i = TEST_SIZE / 2; i < TEST_SIZE; ++i)
    {
        EXPECT_EQ(fc.Get(std::to_string('0' + i)), i);
    }
}

TEST(FIFOCache, Remove_Test)
{
    constexpr std::size_t TEST_SIZE = 10;
    fifo_cache_t<std::string, std::size_t> fc(TEST_SIZE);

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

TEST(FIFOCache, TryGet)
{
    constexpr std::size_t TEST_CASE{10};
    fifo_cache_t<std::string, std::size_t> cache{TEST_CASE};

    for (std::size_t i = 0; i < TEST_CASE; ++i)
    {
        cache.Put(std::to_string(i), i);
    }

    for (std::size_t i = 0; i < TEST_CASE; ++i)
    {
        auto element = cache.TryGet(std::to_string(i));
        EXPECT_TRUE(element.second);
        EXPECT_EQ(element.first->second, i);
    }

    for (std::size_t i = TEST_CASE; i < TEST_CASE * 2; ++i)
    {
        auto element = cache.TryGet(std::to_string(i));
        EXPECT_FALSE(element.second);
    }
}
