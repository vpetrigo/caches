#include "caches/caches.hpp"
#include "test_helper.hpp"

#include <array>
#include <gtest/gtest.h>

template <typename Backend>
class FIFOCache : public ::testing::Test
{
  public:
    template <typename K, typename V>
    using cache_t = typename Backend::template cache_t<caches::FIFO, K, V>;
};

using Backends = ::testing::Types<StdBackend, PhmapBackend>;
TYPED_TEST_SUITE(FIFOCache, Backends);

TYPED_TEST(FIFOCache, Simple_Test)
{
    using cache_t = typename TestFixture::template cache_t<int, int>;
    cache_t fc(2);

    fc.Put(1, 10);
    fc.Put(2, 20);

    EXPECT_EQ(fc.Size(), 2);
    EXPECT_EQ(*fc.Get(1), 10);
    EXPECT_EQ(*fc.Get(2), 20);

    fc.Put(1, 30);
    EXPECT_EQ(fc.Size(), 2);
    EXPECT_EQ(*fc.Get(1), 30);

    fc.Put(3, 30);
    EXPECT_THROW(fc.Get(1), std::range_error);
    EXPECT_EQ(*fc.Get(2), 20);
    EXPECT_EQ(*fc.Get(3), 30);
}

TYPED_TEST(FIFOCache, Missing_Value)
{
    using cache_t = typename TestFixture::template cache_t<int, int>;
    cache_t fc(2);

    fc.Put(1, 10);

    EXPECT_EQ(fc.Size(), 1);
    EXPECT_EQ(*fc.Get(1), 10);
    EXPECT_THROW(fc.Get(2), std::range_error);
}

TYPED_TEST(FIFOCache, Sequence_Test)
{
    constexpr int TEST_SIZE = 10;
    using cache_t = typename TestFixture::template cache_t<std::string, int>;
    cache_t fc(TEST_SIZE);

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        fc.Put(std::to_string('0' + i), static_cast<int>(i));
    }

    EXPECT_EQ(fc.Size(), TEST_SIZE);

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        EXPECT_EQ(*fc.Get(std::to_string('0' + i)), i);
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
        EXPECT_EQ(*fc.Get(std::to_string('a' + i)), i);
    }

    for (size_t i = TEST_SIZE / 2; i < TEST_SIZE; ++i)
    {
        EXPECT_EQ(*fc.Get(std::to_string('0' + i)), i);
    }
}

TYPED_TEST(FIFOCache, Remove_Test)
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

TYPED_TEST(FIFOCache, Partial_Remove_Test)
{
    using cache_t = typename TestFixture::template cache_t<std::string, int>;
    cache_t cache{5};

    for (int i = 0; i < 5; ++i)
    {
        cache.Put("key" + std::to_string(i), i);
    }

    constexpr std::array access_order = {
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

    constexpr std::array access_order3 = {
        "key5", "key6", "key1", "key2", "key4",
    };

    for (const auto &key : access_order3)
    {
        EXPECT_TRUE(cache.Cached(key));
        EXPECT_NO_THROW(cache.Get(key));
    }
}

TYPED_TEST(FIFOCache, TryGet)
{
    constexpr std::size_t TEST_CASE{10};
    using cache_t = typename TestFixture::template cache_t<std::string, std::size_t>;
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

TYPED_TEST(FIFOCache, GetWithReplacement)
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

TYPED_TEST(FIFOCache, InvalidSize)
{
    using test_type = typename TestFixture::template cache_t<std::string, int>;
    EXPECT_THROW(test_type cache{0}, std::invalid_argument);
}
