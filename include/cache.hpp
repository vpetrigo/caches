#ifndef CACHE_HPP
#define CACHE_HPP

#include <mutex>
#include <limits>
#include <cstddef>

namespace caches {

// Base class for caching algorithms
template <typename Key, typename Value>
class cache {
 public:
  virtual void Put(const Key& key, const Value& value) = 0;
  virtual const Value& Get(const Key& key) const = 0;

 protected:
   cache(size_t max_size) : max_cache_size{max_size} {
     if (max_cache_size == 0) {
       max_cache_size = std::numeric_limits<size_t>::max();
     }
   }

   
 private:
   using operation_guard = typename std::lock_guard<std::mutex>;

   mutable std::mutex safe_op;
   size_t max_cache_size;
};

}

#endif // CACHE_HPP
