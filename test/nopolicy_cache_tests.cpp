#include "cache.hpp"
#include "cache_policy.hpp"

#include <gtest/gtest.h>
#include <stdexcept>

TEST(NoPolicyCache, Add_one_element) {
  constexpr std::size_t cache_size = 1;
  caches::fixed_sized_cache<std::string, int> cache(cache_size);

  cache.Put("Hello", 1);
  ASSERT_EQ(cache.Get("Hello"), 1);
}

TEST(NoPolicyCache, Add_delete_add_one_element) {
  constexpr std::size_t cache_size = 1;
  caches::fixed_sized_cache<std::string, int> cache(cache_size);

  cache.Put("Hello", 1);
  cache.Put("World", 2);
  ASSERT_THROW(cache.Get("Hello"), std::range_error);
  ASSERT_EQ(cache.Get("World"), 2);
}

TEST(NoPolicyCache, Add_many_elements) {
  constexpr std::size_t cache_size = 1024;
  caches::fixed_sized_cache<std::string, int> cache(cache_size);

  for (std::size_t i = 0; i < cache_size; ++i) {
    cache.Put(std::to_string(i), i);
  }

  ASSERT_EQ(cache.Size(), cache_size);

  for (std::size_t i = 0; i < cache_size; ++i) {
    ASSERT_EQ(cache.Get(std::to_string(i)), i);
  }
}

TEST(NoPolicyCache, Small_cache_many_elements) {
  constexpr std::size_t cache_size = 1;
  caches::fixed_sized_cache<std::string, int> cache(cache_size);

  for (std::size_t i = 0; i < cache_size; ++i) {
    std::string temp_key = std::to_string(i);
    cache.Put(temp_key, i);
    ASSERT_EQ(cache.Get(temp_key), i);
  }

  ASSERT_EQ(cache.Size(), cache_size);
}
