#include "cache.hpp"
#include "lru_cache_policy.hpp"
#include <gtest/gtest.h>

template <typename Key, typename Value>
using lru_cache_t =
    typename caches::fixed_sized_cache<Key, Value, caches::LRUCachePolicy<Key>>;

TEST(CacheTest, SimplePut)
{
    lru_cache_t<std::string, int> cache(1);

    cache.Put("test", 666);

    EXPECT_EQ(cache.Get("test"), 666);
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
        EXPECT_EQ(i, cache.Get(i));
    }
}

TEST(LRUCache, Remove_Test) {
  constexpr std::size_t TEST_SIZE = 10;
  lru_cache_t <std::string, std::size_t> fc(TEST_SIZE);

  for (std::size_t i = 0; i < TEST_SIZE; ++i) {
    fc.Put(std::to_string(i), i);
  }

  EXPECT_EQ(fc.Size(), TEST_SIZE);

  for (std::size_t i = 0; i < TEST_SIZE; ++i) {
    EXPECT_TRUE(fc.Remove(std::to_string(i)));
  }

  EXPECT_EQ(fc.Size(), 0);

  for (std::size_t i = 0; i < TEST_SIZE; ++i) {
    EXPECT_FALSE(fc.Remove(std::to_string(i)));
  }
}
