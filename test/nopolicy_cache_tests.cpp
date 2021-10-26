#include "cache.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

template <typename K, typename V>
using no_policy_cache_t = typename caches::fixed_sized_cache<K, V, caches::NoCachePolicy>;

TEST(NoPolicyCache, Add_one_element)
{
    constexpr std::size_t cache_size = 1;
    caches::fixed_sized_cache<std::string, int> cache(cache_size);

    cache.Put("Hello", 1);
    ASSERT_EQ(cache.Get("Hello"), 1);
}

TEST(NoPolicyCache, Add_delete_add_one_element)
{
    constexpr std::size_t cache_size = 1;
    caches::fixed_sized_cache<std::string, int> cache(cache_size);

    cache.Put("Hello", 1);
    cache.Put("World", 2);
    ASSERT_THROW(cache.Get("Hello"), std::range_error);
    ASSERT_EQ(cache.Get("World"), 2);
}

TEST(NoPolicyCache, Add_many_elements)
{
    constexpr std::size_t cache_size = 1024;
    caches::fixed_sized_cache<std::string, std::size_t> cache(cache_size);

    for (std::size_t i = 0; i < cache_size; ++i)
    {
        cache.Put(std::to_string(i), i);
    }

    ASSERT_EQ(cache.Size(), cache_size);

    for (std::size_t i = 0; i < cache_size; ++i)
    {
        ASSERT_EQ(cache.Get(std::to_string(i)), i);
    }
}

TEST(NoPolicyCache, Small_cache_many_elements)
{
    constexpr std::size_t cache_size = 1;
    caches::fixed_sized_cache<std::string, std::size_t> cache(cache_size);

    for (std::size_t i = 0; i < cache_size; ++i)
    {
        std::string temp_key = std::to_string(i);
        cache.Put(temp_key, i);
        ASSERT_EQ(cache.Get(temp_key), i);
    }

    ASSERT_EQ(cache.Size(), cache_size);
}

TEST(NoPolicyCache, Remove_Test)
{
    constexpr std::size_t TEST_SIZE = 10;
    caches::fixed_sized_cache<std::string, std::size_t> fc(TEST_SIZE);

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

TEST(NoPolicyCache, TryGet)
{
    constexpr std::size_t TEST_CASE{10};
    no_policy_cache_t<std::string, std::size_t> cache{TEST_CASE};

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
