#include "caches/caches.hpp"
#include "test_helper.hpp"

#include <array>
#include <gtest/gtest.h>

template <typename Backend>
class LRUCache : public ::testing::Test
{
  public:
    template <typename K, typename V>
    using cache_t = typename Backend::template cache_t<caches::LRU, K, V>;
};

using Backends = ::testing::Types<StdBackend, PhmapBackend>;
TYPED_TEST_SUITE(LRUCache, Backends);

TYPED_TEST(LRUCache, SimplePut)
{
    using cache_t = typename TestFixture::template cache_t<std::string, int>;
    cache_t cache(1);

    cache.Put("test", 666);

    EXPECT_EQ(*cache.Get("test"), 666);
}

TYPED_TEST(LRUCache, PutWithUpdate)
{
    using cache_t = typename TestFixture::template cache_t<std::string, std::size_t>;
    constexpr std::size_t TEST_CASE = 4;
    cache_t cache{TEST_CASE};

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

TYPED_TEST(LRUCache, MissingValue)
{
    using cache_t = typename TestFixture::template cache_t<std::string, int>;
    cache_t cache(1);

    EXPECT_THROW(cache.Get("test"), std::range_error);
}

TYPED_TEST(LRUCache, KeepsAllValuesWithinCapacity)
{
    using cache_t = typename TestFixture::template cache_t<int, int>;
    constexpr int CACHE_CAP = 50;
    const int TEST_RECORDS = 100;
    cache_t cache(CACHE_CAP);

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

TYPED_TEST(LRUCache, Remove_Test)
{
    constexpr std::size_t TEST_SIZE = 10;
    using cache_t = typename TestFixture::template cache_t<std::string, std::size_t>;
    cache_t fc(TEST_SIZE);

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

TYPED_TEST(LRUCache, Partial_Remove_Test)
{
    using cache_t = typename TestFixture::template cache_t<std::string, int>;
    cache_t cache{5};

    for (int i = 0; i < 5; ++i)
    {
        cache.Put("key" + std::to_string(i), i);
    }

    const std::array<const char *, 5> access_order = {
        "key1", "key3", "key0", "key4", "key2",
    };

    for (const auto &key : access_order)
    {
        EXPECT_NE(cache.Get(key), nullptr);
    }

    cache.Remove("key3");

    for (int i = 0; i < 5; ++i)
    {
        if (const auto key = "key" + std::to_string(i); key != "key3")
        {
            EXPECT_TRUE(cache.Cached(key));
        }
        else
        {
            EXPECT_FALSE(cache.Cached(key));
        }
    }

    cache.Put("key5", 5);
    cache.Put("key6", 6);

    const std::array<const char *, 5> access_order3 = {
        "key5", "key6", "key0", "key2", "key4",
    };

    for (const auto &key : access_order3)
    {
        EXPECT_TRUE(cache.Cached(key));
        EXPECT_NO_THROW(cache.Get(key));
    }
}

TYPED_TEST(LRUCache, CachedCheck)
{
    using cache_t = typename TestFixture::template cache_t<std::string, std::size_t>;
    constexpr std::size_t TEST_SUITE = 4;
    cache_t cache(TEST_SUITE);

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

TYPED_TEST(LRUCache, ConstructCache)
{
    using cache_t = typename TestFixture::template cache_t<std::string, std::size_t>;
    EXPECT_THROW((cache_t(0)), std::invalid_argument);
    EXPECT_NO_THROW((cache_t(1024)));
}

TYPED_TEST(LRUCache, TryGet)
{
    using cache_t = typename TestFixture::template cache_t<std::string, std::size_t>;
    constexpr std::size_t TEST_CASE{10};
    cache_t cache{TEST_CASE};

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

TYPED_TEST(LRUCache, GetWithReplacement)
{
    using cache_t = typename TestFixture::template cache_t<std::string, std::size_t>;
    cache_t cache{2};

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

TYPED_TEST(LRUCache, InvalidSize)
{
    using test_type = typename TestFixture::template cache_t<std::string, int>;
    EXPECT_THROW(test_type cache{0}, std::invalid_argument);
}
