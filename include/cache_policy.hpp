#ifndef CACHE_POLICY_HPP
#define CACHE_POLICY_HPP

namespace caches {

template <typename Key>
class ICachePolicy {
 public:
  virtual ~ICachePolicy() {}
  // handle element insertion in a cache
  virtual void Insert(const Key& key) = 0;
  // handle request to the key-element in a cache
  virtual void Touch(const Key& key) = 0;
  // handle element deletion from a cache
  virtual void Erase(const Key& key) = 0;

  // return a key of a displacement candidate
  virtual const Key& DispCandidate() const = 0;
};

template <typename Key>
class NoCachePolicy : public ICachePolicy<Key> {
 public:
  NoCachePolicy() = default;
  ~NoCachePolicy() override = default;
  
  void Insert(const Key& key) override {
    
  }
  
  void Touch(const Key& key) override {
    
  }
  
  void Erase(const Key& key) {
    
  }

  // return a key of a displacement candidate
  virtual const Key& DispCandidate() const {
    
  }
};
}  // namespace caches

#endif  // CACHE_POLICY_HPP