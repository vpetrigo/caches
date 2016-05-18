#ifndef LRU_CACHE_POLICY_HPP
#define LRU_CACHE_POLICY_HPP

#include "cache_policy.hpp"
#include <list>

namespace caches {
template <typename Key>
class FIFOCachePolicy : public ICachePolicy<Key> {
 public:
  ~FIFOCachePolicy() = default;
  
  void Insert(const Key& key) override {
    fifo_queue.emplace_front(key);
  }
  // handle request to the key-element in a cache
  void Touch(const Key& key) override {
    // nothing to do here in the FIFO strategy
  }
  // handle element deletion from a cache
  virtual void Erase(const Key& key) override {
    fifo_queue.pop_back();
  }

  // return a key of a displacement candidate
  virtual const Key& DispCandidate() const {
    return fifo_queue.back();
  }
 private:
  std::list<Key> fifo_queue;
};
} // namespace caches

#endif // LRU_CACHE_POLICY_HPP