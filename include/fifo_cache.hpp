#ifndef FIFO_CACHE_HPP
#define FIFO_CACHE_HPP

#include <deque>
#include <iterator>
#include <mutex>
#include <unordered_map>
#include <utility>

namespace caches {

template <typename Key, typename Value>
class fifo_cache {
 public:
  using value_type = typename std::pair<Key, Value>;
  using value_it = typename std::deque<value_type>::iterator;
  using operation_guard = typename std::lock_guard<std::mutex>;

  fifo_cache(size_t max_size) : max_cache_size{max_size} {}

  void Put(const Key& key, const Value& value) {
    operation_guard og{safe_op};
    auto it = cache_items_map.find(key);

    if (it == cache_items_map.end()) {
      if (cache_items_map.size() + 1 > max_cache_size) {
        // remove the last element from cache
        auto last = cache_items_deque.rbegin();

        cache_items_map.erase(last->first);
        cache_items_deque.pop_back();
      }

      cache_items_deque.push_front(std::make_pair(key, value));
      cache_items_map[key] = cache_items_deque.begin();
    }
    else {
      // just update value
      it->second->second = value;
    }
  }

  const Value& Get(const Key& key) {
    operation_guard og{safe_op};
    auto it = cache_items_map.find(key);

    if (it == cache_items_map.end()) {
      throw std::range_error("No such key in the cache");
    }

    return it->second->second;
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
  std::deque<value_type> cache_items_deque;
  std::unordered_map<Key, value_it> cache_items_map;
  size_t max_cache_size;
  mutable std::mutex safe_op;
};

}

#endif  // FIFO_CACHE_HPP
