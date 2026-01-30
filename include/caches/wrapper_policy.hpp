/**
 * \file
 * \brief Policy for customizing the wrapped value type in cache
 *
 * This header provides mechanisms to customize how cached values are wrapped.
 * By default, values are wrapped in std::shared_ptr using std::make_shared.
 *
 * \section overview Overview
 *
 * The cache stores values wrapped in a smart pointer to allow safe access
 * even after the value has been evicted. The wrapper_policy template controls:
 * - The wrapper type (e.g., std::shared_ptr, boost::shared_ptr)
 * - How wrapped values are created (e.g., make_shared, allocate_shared)
 *
 * \section customization Customization Options
 *
 * 1. Specialize wrapper_policy for specific value types
 * 2. Use pre-built policies: allocate_shared_wrapper, custom_deleter_wrapper
 * 3. Use make_wrapper_policy helper for inline custom policies
 */
#ifndef CACHES_WRAPPER_POLICY_HPP
#define CACHES_WRAPPER_POLICY_HPP

#include <memory>
#include <utility>

namespace caches
{

namespace detail
{
/**
 * \brief Allocator-aware deleter that destroys and deallocates
 *
 * Used by generic
 * smart_ptr_traits::allocate to ensure memory allocated
 * with a custom allocator is properly
 * deallocated using the same allocator.
 *
 * \tparam T The value type
 * \tparam Allocator The
 * allocator type
 */
template <typename T, typename Allocator>
struct allocator_destroy_deleter
{
    Allocator allocator;

    explicit allocator_destroy_deleter(Allocator a) : allocator{std::move(a)}
    {
    }

    void operator()(T *ptr)
    {
        if (ptr != nullptr)
        {
            using alloc_traits = std::allocator_traits<Allocator>;
            alloc_traits::destroy(allocator, ptr);
            alloc_traits::deallocate(allocator, ptr, 1);
        }
    }
};

/**
 * \brief Allocator-aware deleter wrapper for full_control_wrapper
 *
 * Wraps a user-provided deleter and an allocator. When invoked:
 * 1. Calls the user's deleter to destroy the object
 *
 * (destructor only)
 * 2. Uses the allocator to deallocate memory
 *
 * \tparam T The value type
 * \tparam Allocator The allocator type
 * \tparam Deleter The deleter type (destruction only)
 */
template <typename T, typename Allocator, typename Deleter>
struct allocator_deleter
{
    Allocator allocator;
    Deleter deleter;

    allocator_deleter(Allocator a, Deleter d) : allocator{std::move(a)}, deleter{std::move(d)}
    {
    }

    void operator()(T *ptr)
    {
        if (ptr != nullptr)
        {
            deleter(ptr); // User's deleter: destroy only
            using alloc_traits = std::allocator_traits<Allocator>;
            alloc_traits::deallocate(allocator, ptr, 1);
        }
    }
};
} // namespace detail

/**
 * \brief Traits for smart pointer creation functions
 *
 * Provides a uniform interface for creating smart pointers of different types.
 * Specialize this for custom smart pointer implementations.
 *
 * \tparam SmartPtr Smart pointer template (e.g., std::shared_ptr)
 */
template <template <typename> class SmartPtr>
struct smart_ptr_traits
{
    /**
     * \brief Create a smart pointer using default allocation
     */
    template <typename T, typename... Args>
    static SmartPtr<T> make(Args &&...args)
    {
        return SmartPtr<T>(new T(std::forward<Args>(args)...));
    }

    /**
     * \brief Create a smart pointer using custom allocator
     *
     * Uses
     * allocator_destroy_deleter to ensure proper deallocation
     * through the same allocator
     * that was used for allocation.
     */
    template <typename T, typename Alloc, typename... Args>
    static SmartPtr<T> allocate(Alloc alloc, Args &&...args)
    {
        using alloc_traits = std::allocator_traits<Alloc>;
        T *ptr = alloc_traits::allocate(alloc, 1);
        try
        {
            alloc_traits::construct(alloc, ptr, std::forward<Args>(args)...);
        }
        catch (...)
        {
            alloc_traits::deallocate(alloc, ptr, 1);
            throw;
        }

        return SmartPtr<T>(ptr, detail::allocator_destroy_deleter<T, Alloc>{alloc});
    }
};

/**
 * \brief Specialization for std::shared_ptr
 *
 * Uses std::make_shared and std::allocate_shared for optimal performance.
 */
template <>
struct smart_ptr_traits<std::shared_ptr>
{
    template <typename T, typename... Args>
    static std::shared_ptr<T> make(Args &&...args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename T, typename Alloc, typename... Args>
    static std::shared_ptr<T> allocate(Alloc alloc, Args &&...args)
    {
        return std::allocate_shared<T>(alloc, std::forward<Args>(args)...);
    }
};

/**
 * \brief Primary wrapper policy template with default std::shared_ptr behavior
 *
 * This is the main customization point for value wrapping. Users can:
 * 1. Specialize this template for specific value types
 * 2. Use pre-built policy templates (allocate_shared_wrapper, etc.)
 * 3. Define their own policy and pass it as a template argument to cache
 *
 * \b Example: Specialization for a custom type
 * \code
 * template<>
 * struct caches::wrapper_policy<MyValue> {
 *     using type = boost::shared_ptr<MyValue>;
 *     template <typename... Args>
 *     static type create(Args&&... args) {
 *         return boost::make_shared<MyValue>(std::forward<Args>(args)...);
 *     }
 * };
 * \endcode
 *
 * \tparam Value The value type to wrap
 */
template <typename Value>
struct wrapper_policy
{
    using type = std::shared_ptr<Value>;

    template <typename... Args>
    static type create(Args &&...args)
    {
        return std::make_shared<Value>(std::forward<Args>(args)...);
    }
};

/**
 * \brief Helper to create custom wrapper policies inline
 *
 * Use this when you have a template that defines a wrapper policy and want
 * to use it with the cache without specializing wrapper_policy.
 *
 * \b Example
 *
 * \code
 * template <typename T>
 * struct my_wrapper_policy {
 *     using type = std::shared_ptr<T>;
 *     template <typename... Args>
 *     static type create(Args&&... args) {
 *         // Custom creation logic
 *         return std::make_shared<T>(std::forward<Args>(args)...);
 *     }
 * };
 *
 * using my_helper = caches::make_wrapper_policy<my_wrapper_policy>;
 * caches::cache<Key, Value, Policy, KeyTraits, my_helper::policy<Value>> cache{100};
 * \endcode
 *
 * \tparam WrapperPolicyTemplate A template that takes a Value type
 */
template <template <typename> class WrapperPolicyTemplate>
struct make_wrapper_policy
{
    template <typename Value>
    using policy = WrapperPolicyTemplate<Value>;
};

/**
 * \brief Wrapper policy using std::allocate_shared with a custom allocator
 *
 * \tparam Value The value type to wrap
 * \tparam Allocator The allocator type to use
 * \tparam SmartPtr The smart pointer template to use (default: std::shared_ptr)
 */
template <typename Value, typename Allocator, template <typename> class SmartPtr = std::shared_ptr>
struct allocate_shared_wrapper
{
    using type = SmartPtr<Value>;
    using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<Value>;

    template <typename... Args>
    static type create(Args &&...args)
    {
        return smart_ptr_traits<SmartPtr>::template allocate<Value>(allocator_type{},
                                                                    std::forward<Args>(args)...);
    }
};

/**
 * \brief Wrapper policy using std::shared_ptr with a custom deleter
 *
 * \tparam Value The value type to wrap
 * \tparam Deleter The deleter type to use
 * \tparam SmartPtr The smart pointer template to use (default: std::shared_ptr)
 */
template <typename Value, typename Deleter, template <typename> class SmartPtr = std::shared_ptr>
struct custom_deleter_wrapper
{
    using type = SmartPtr<Value>;
    using deleter_type = Deleter;

    template <typename... Args>
    static type create(Args &&...args)
    {
        return type(new Value(std::forward<Args>(args)...), Deleter{});
    }
};

/**
 * \brief Wrapper policy combining custom allocator and deleter
 *
 * The allocator is used for
 * memory allocation and deallocation.
 * The deleter is responsible for object destruction only
 * (calling destructor
 * or custom cleanup logic) - it should NOT deallocate memory.
 *
 * \tparam
 * Value The value type to wrap
 * \tparam Allocator The allocator type to use for memory
 * management
 * \tparam Deleter The deleter type (destruction only, NOT deallocation)
 * \tparam
 * SmartPtr The smart pointer template to use (default: std::shared_ptr)
 */
template <typename Value, typename Allocator, typename Deleter,
          template <typename> class SmartPtr = std::shared_ptr>
struct full_control_wrapper
{
    using type = SmartPtr<Value>;
    using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<Value>;
    using deleter_type = Deleter;

  private:
    // RAII guard for allocated but not yet constructed memory
    struct alloc_guard
    {
        allocator_type &alloc;
        Value *ptr;
        bool released{false};

        alloc_guard(allocator_type &a, Value *p) : alloc{a}, ptr{p}
        {
        }

        ~alloc_guard()
        {
            if (!released && (ptr != nullptr))
            {
                using alloc_traits = std::allocator_traits<allocator_type>;
                alloc_traits::deallocate(alloc, ptr, 1);
            }
        }

        void release() noexcept
        {
            released = true;
        }
    };

  public:
    template <typename... Args>
    static type create(Args &&...args)
    {
        allocator_type alloc{};
        using alloc_traits = std::allocator_traits<allocator_type>;
        Value *ptr = alloc_traits::allocate(alloc, 1);
        alloc_guard guard(alloc, ptr);

        alloc_traits::construct(alloc, ptr, std::forward<Args>(args)...);
        guard.release();

        return type{
            ptr, detail::allocator_deleter<Value, allocator_type, deleter_type>{alloc, Deleter{}},
            alloc};
    }
};

} // namespace caches

#endif // CACHES_WRAPPER_POLICY_HPP
