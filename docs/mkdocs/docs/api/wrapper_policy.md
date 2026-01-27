# Wrapper policy

`caches::wrapper_policy<Value>` controls how cached values are wrapped.

The default wrapper is `std::shared_ptr<Value>`. This allows callers to keep a value alive even if the cache later
evicts that entry.

You can pass a custom wrapper policy as the `WrapperPolicy` template parameter of `caches::cache`.

Full reference:

- [Doxygen API](../doxygen/index.html)
