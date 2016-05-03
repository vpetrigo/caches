#include <gtest/gtest.h>
#include "lru_cache.hpp"

TEST(CacheTest, SimplePut) { 
  caches::lru_cache<std::string, int> cache(1);
  
  cache.Put("test", 666);
  
  EXPECT_TRUE(cache.Exists("test"));
  EXPECT_EQ(cache.Get("test"), 666);
  EXPECT_EQ(cache.Size(), 1);
}

TEST(CacheTest, MissingValue) {
  caches::lru_cache<std::string, int> cache(1);
  
  EXPECT_THROW(cache.Get("test"), std::range_error);
}

TEST(CacheTest, KeepsAllValuesWithinCapacity) {
  constexpr int CACHE_CAP = 50;
  const int TEST_RECORDS = 100;
  
  caches::lru_cache<int, int> cache(CACHE_CAP);

  for (int i = 0; i < TEST_RECORDS; ++i) {
      cache.Put(i, i);
  }

  for (int i = 0; i < TEST_RECORDS - CACHE_CAP; ++i) {
      EXPECT_FALSE(cache.Exists(i));
  }

  for (int i = TEST_RECORDS - CACHE_CAP; i < TEST_RECORDS; ++i) {
      EXPECT_TRUE(cache.Exists(i));
      EXPECT_EQ(i, cache.Get(i));
  }
  
  size_t size = cache.Size();
  EXPECT_EQ(CACHE_CAP, size);
}