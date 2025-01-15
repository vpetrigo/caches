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
    ASSERT_EQ(*cache.Get("Hello"), 1);
}

TEST(NoPolicyCache, Put_returns_shared_ptr)
{
    constexpr std::size_t cache_size = 1;
    caches::fixed_sized_cache<std::string, int> cache(cache_size);

    auto val = cache.Put("Hello", 1);
    ASSERT_EQ(*val, 1);
    ASSERT_EQ(*cache.Get("Hello"), 1);

    // update existing element
    auto val_updated = cache.Put("Hello", 2);
    ASSERT_EQ(*val_updated, 2);
}

TEST(NoPolicyCache, Add_delete_add_one_element)
{
    constexpr std::size_t cache_size = 1;
    caches::fixed_sized_cache<std::string, int> cache(cache_size);

    cache.Put("Hello", 1);
    cache.Put("World", 2);
    ASSERT_THROW(cache.Get("Hello"), std::range_error);
    ASSERT_EQ(*cache.Get("World"), 2);
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
        ASSERT_EQ(*cache.Get(std::to_string(i)), i);
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
        ASSERT_EQ(*cache.Get(temp_key), i);
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
        EXPECT_EQ(*element.first, i);
    }

    for (std::size_t i = TEST_CASE; i < TEST_CASE * 2; ++i)
    {
        auto element = cache.TryGet(std::to_string(i));
        EXPECT_FALSE(element.second);
    }
}

TEST(NoPolicyCache, GetWithReplacement)
{
    no_policy_cache_t<std::string, std::size_t> cache{2};

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

TEST(NoPolicyCache, InvalidSize)
{
    using test_type = no_policy_cache_t<std::string, int>;
    EXPECT_THROW(test_type cache{0}, std::invalid_argument);
}
