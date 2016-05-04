#ifndef LFU_CACHE_HPP
#define LFU_CACHE_HPP

#include <algorithm>
#include <list>
#include <mutex>
#include <tuple>
#include <unordered_map>

namespace caches {

template <typename Key, typename Value>
class lfu_cache {
 public:
  using freq_type = unsigned;
  using value_type = typename std::tuple<Key, Value, freq_type>;
  using value_it = typename std::list<value_type>::iterator;
  using operation_guard = typename std::lock_guard<std::mutex>;

  enum VTFields { key_f = 0, value_f = 1, frequency_f = 2 };

  lfu_cache(size_t max_size) : max_cache_size{max_size} {}

  void Put(const Key& key, const Value& value) {
    constexpr unsigned INIT_FREQ = 1;
    operation_guard og{safe_op};
    auto it = cache_items_map.find(key);

    if (it == cache_items_map.end()) {
      if (cache_items_map.size() + 1 > max_cache_size) {
        // look for the element with the smallest frequency value
        auto least_fr =
            std::min_element(cache_items_list.cbegin(), cache_items_list.cend(),
                             [](const value_type& a, const value_type& b) {
                               return std::get<frequency_f>(a) <
                                      std::get<frequency_f>(b);
                             });

        cache_items_map.erase(std::get<key_f>(*least_fr));
        cache_items_list.erase(least_fr);
      }

      cache_items_list.emplace_front(std::make_tuple(key, value, INIT_FREQ));
      cache_items_map[key] = cache_items_list.begin();
    }
    else {
      // increase frequency of the existing value "key" and assigne new value
      std::get<value_f>(*it->second) = value;
      ++(std::get<frequency_f>(*it->second));
    }
  }

  const Value& Get(const Key& key) {
    operation_guard og{safe_op};
    auto it = cache_items_map.find(key);

    if (it == cache_items_map.end()) {
      throw std::range_error("No such key in the cache");
    }
    else {
      // increment the frequency of the "key"-element
      ++(std::get<frequency_f>(*it->second));

      return std::get<value_f>(*it->second);
    }
  }

  bool Exists(const Key& key) const {
    operation_guard og{safe_op};

    return cache_items_map.find(key) != cache_items_map.end();
  }

  size_t Size() const {
    operation_guard og{safe_op};

    return cache_items_map.size();
  }

 private:
  std::list<value_type> cache_items_list;
  std::unordered_map<Key, value_it> cache_items_map;
  size_t max_cache_size;
  mutable std::mutex safe_op;
};

}  // caches

#endif  // LFU_CACHE_HPP
