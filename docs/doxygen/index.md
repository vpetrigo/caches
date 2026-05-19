Cache Library
-------------

These pages contain the API documentation of Caches, a header-only C++ cache library.

Prefer including the convenience header:

\code
#include <caches/caches.hpp>
\endcode

# Contents

- Core
  - \link caches::cache \endlink -- fixed-size, thread-safe cache
- Policies
  - \link caches::PolicyInterface \endlink -- policy interface
  - \link caches::LRU \endlink -- least recently used
  - \link caches::LFU \endlink -- least frequently used
  - \link caches::FIFO \endlink -- first in, first out
  - \link caches::NoEviction \endlink -- never evict (inserts fail when full)
- Customization
  - \link caches::key_traits \endlink -- hash/equality traits (std::hash or ADL hash_value)
  - \link caches::make_traits \endlink -- helper to define custom traits inline
  - \link caches::wrapper_policy \endlink -- customize cached value wrapper type

\author [Vladimir Petrigo](https://rs-stuff.dev/about/)

\see https://github.com/vpetrigo/caches to download the source code

\version 0.2.0
