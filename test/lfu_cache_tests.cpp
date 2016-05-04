#include <gtest/gtest.h>
#include "lfu_cache.hpp"

TEST(LFU_Cache, Simple_Test) {
  constexpr size_t FIRST_FREQ = 10;
  constexpr size_t SECOND_FREQ = 9;
  constexpr size_t THIRD_FREQ = 8;
  caches::lfu_cache<std::string, int> lfu(3);

  lfu.Put("A", 1);
  lfu.Put("B", 2);
  lfu.Put("C", 3);

  EXPECT_TRUE(lfu.Exists("A"));
  EXPECT_TRUE(lfu.Exists("B"));
  EXPECT_TRUE(lfu.Exists("C"));
  EXPECT_EQ(lfu.Size(), 3);

  for (size_t i = 0; i < FIRST_FREQ; ++i) {
    EXPECT_EQ(lfu.Get("B"), 2);
  }

  for (size_t i = 0; i < SECOND_FREQ; ++i) {
    EXPECT_EQ(lfu.Get("A"), 1);
  }

  for (size_t i = 0; i < THIRD_FREQ; ++i) {
    EXPECT_EQ(lfu.Get("C"), 3);
  }

  lfu.Put("D", 4);

  EXPECT_TRUE(lfu.Exists("A"));
  EXPECT_TRUE(lfu.Exists("B"));
  EXPECT_TRUE(lfu.Exists("D"));
  EXPECT_THROW(lfu.Get("C"), std::range_error);
  EXPECT_EQ(lfu.Size(), 3);
}

TEST(LFU_Cache, Single_Slot) {
  constexpr size_t TEST_SIZE = 5;
  caches::lfu_cache<int, int> lfu(1);

  lfu.Put(1, 10);

  for (size_t i = 0; i < TEST_SIZE; ++i) {
    lfu.Put(1, i);
  }

  EXPECT_EQ(lfu.Get(1), 4);

  lfu.Put(2, 20);

  EXPECT_THROW(lfu.Get(1), std::range_error);
  EXPECT_EQ(lfu.Get(2), 20);
  EXPECT_EQ(lfu.Size(), 1);
}
