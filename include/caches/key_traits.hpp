/**
 * \file
 * \brief Key traits for automatic hash/equality detection via ADL
 */
#ifndef CACHES_KEY_TRAITS_HPP
#define CACHES_KEY_TRAITS_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <type_traits>

namespace caches
{

namespace detail
{

// void_t implementation for C++11 compatibility
template <typename...>
struct voider
{
    using type = void;
};

template <typename... Ts>
using void_t = typename voider<Ts...>::type;

// Detection idiom for ADL hash_value function
template <typename T, typename = void>
struct has_adl_hash : std::false_type
{
};

template <typename T>
struct has_adl_hash<T, void_t<decltype(hash_value(std::declval<const T &>()))>> : std::true_type
{
};

// Detection idiom for ADL operator==
template <typename T, typename = void>
struct has_adl_equal : std::false_type
{
};

template <typename T>
struct has_adl_equal<T, void_t<decltype(std::declval<const T &>() == std::declval<const T &>())>>
    : std::true_type
{
};

// Detection for std::hash specialization
template <typename T, typename = void>
struct has_std_hash : std::false_type
{
};

template <typename T>
struct has_std_hash<T, void_t<decltype(std::declval<std::hash<T>>()(std::declval<const T &>()))>>
    : std::true_type
{
};

// ADL-based hash functor that calls hash_value(key)
template <typename T>
struct adl_hash
{
    std::size_t operator()(const T &value) const noexcept(noexcept(hash_value(value)))
    {
        return hash_value(value);
    }
};

// ADL-based equality functor that calls operator==
template <typename T>
struct adl_equal
{
    bool operator()(const T &lhs, const T &rhs) const
        noexcept(noexcept(std::declval<const T &>() == std::declval<const T &>()))
    {
        return lhs == rhs;
    }
};

// Select hash type: prefer std::hash if available, then ADL hash_value
template <typename T, bool HasStdHash = has_std_hash<T>::value,
          bool HasAdlHash = has_adl_hash<T>::value>
struct select_hash_type
{
    // Fallback: no hash available - will cause compile error if used
    using type = std::hash<T>;
};

template <typename T, bool HasAdlHash>
struct select_hash_type<T, true, HasAdlHash>
{
    // std::hash is available
    using type = std::hash<T>;
};

template <typename T>
struct select_hash_type<T, false, true>
{
    // Only ADL hash_value is available
    using type = adl_hash<T>;
};

// Select equality type: prefer std::equal_to, but use ADL operator== if needed
template <typename T, bool HasAdlEqual = has_adl_equal<T>::value>
struct select_equal_type
{
    using type = std::equal_to<T>;
};

} // namespace detail

/**
 * \brief Primary key traits template with automatic hash/equality detection
 *
 * This template automatically detects the appropriate hash and equality
 * functors for a key type using the following priority:
 *
 * For hashing:
 * 1. std::hash<Key> if a valid specialization exists
 * 2. ADL-discovered hash_value(const Key&) function
 *
 * For equality:
 * 1. std::equal_to<Key> (uses operator== which is ADL-discoverable)
 *
 * Users can specialize this template for their own types:
 * \code
 * template<>
 * struct caches::key_traits<MyType> {
 *     using hash_type = MyTypeHash;
 *     using equal_type = MyTypeEqual;
 *     using allocator_type = std::allocator<MyType>;
 * };
 * \endcode
 *
 * \tparam Key The key type
 */
template <typename Key>
struct key_traits
{
    using hash_type = typename detail::select_hash_type<Key>::type;
    using equal_type = typename detail::select_equal_type<Key>::type;
    using allocator_type = std::allocator<Key>;
};

/**
 * \brief Helper to create custom key traits with explicit hash and equality functors
 *
 * Use this when you have a type that doesn't have std::hash or ADL hash_value,
 * or when you want to use custom hash/equality logic.
 *
 * \code
 * struct MyHash { size_t operator()(const MyKey&) const; };
 * struct MyEqual { bool operator()(const MyKey&, const MyKey&) const; };
 *
 * using my_traits = caches::make_traits<MyHash, MyEqual>;
 * caches::cache<MyKey, int, caches::LRU, my_traits> cache{100};
 * \endcode
 *
 * \tparam Hash Hash functor type
 * \tparam Equal Equality functor type
 * \tparam Allocator Allocator type (defaults to std::allocator<char> as placeholder)
 */
template <typename Hash, typename Equal, typename Allocator = std::allocator<char>>
struct make_traits
{
    using hash_type = Hash;
    using equal_type = Equal;
    using allocator_type = Allocator;
};

} // namespace caches

#endif // CACHES_KEY_TRAITS_HPP
