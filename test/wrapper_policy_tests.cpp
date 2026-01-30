/**
 * \file
 * \brief Tests for wrapper_policy customization
 */
#include "caches/caches.hpp"
#include "caches/wrapper_policy.hpp"

#include "test_helper.hpp"

#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>

namespace
{
template <typename Backend>
struct BackendTraits;

template <>
struct BackendTraits<StdBackend>
{
    template <typename K, typename V, typename H, typename E, typename A>
    using MapType = std::unordered_map<K, V, H, E, A>;
};

template <>
struct BackendTraits<PhmapBackend>
{
    template <typename K, typename V, typename H, typename E, typename A>
    using MapType = phmap_node_hash_map<K, V, H, E, A>;
};

template <typename T>
struct Identity
{
    using type = T;
};
} // namespace

namespace test_utils
{

struct tracking_allocator_state
{
    static std::atomic<int> allocation_count;
    static std::atomic<int> deallocation_count;

    static void reset_counts()
    {
        allocation_count.store(0, std::memory_order_relaxed);
        deallocation_count.store(0, std::memory_order_relaxed);
    }
};

std::atomic<int> tracking_allocator_state::allocation_count{0};
std::atomic<int> tracking_allocator_state::deallocation_count{0};

// Tracking allocator to verify allocate_shared usage. Note that allocate_shared
// uses the allocator for the control block as well, so we track allocations
// across all rebinds via a shared state.
template <typename T>
class tracking_allocator
{
  public:
    using value_type = T;

    tracking_allocator() = default;

    template <typename U>
    tracking_allocator(const tracking_allocator<U> &) noexcept
    {
    }

    T *allocate(std::size_t n)
    {
        tracking_allocator_state::allocation_count.fetch_add(1, std::memory_order_relaxed);
        return static_cast<T *>(::operator new(n * sizeof(T)));
    }

    void deallocate(T *p, std::size_t) noexcept
    {
        tracking_allocator_state::deallocation_count.fetch_add(1, std::memory_order_relaxed);
        ::operator delete(p);
    }

    static void reset_counts()
    {
        tracking_allocator_state::reset_counts();
    }

    template <typename U>
    bool operator==(const tracking_allocator<U> &) const noexcept
    {
        return true;
    }

    template <typename U>
    bool operator!=(const tracking_allocator<U> &) const noexcept
    {
        return false;
    }
};

// Tracking deleter to verify custom_deleter_wrapper usage
template <typename T>
struct tracking_deleter
{
    static std::atomic<int> delete_count;

    void operator()(T *p) const
    {
        delete_count.fetch_add(1, std::memory_order_relaxed);
        delete p;
    }

    static void reset_count()
    {
        delete_count.store(0, std::memory_order_relaxed);
    }
};

template <typename T>
std::atomic<int> tracking_deleter<T>::delete_count{0};

// Value type for testing
struct TestValue
{
    int data;
    std::string name;

    explicit TestValue(int d = 0, std::string n = "") : data(d), name(std::move(n))
    {
    }
};

} // namespace test_utils

TEST_CASE("WrapperValueTraits: Default traits use shared_ptr", "[wrapper_policy]")
{
    using policy = caches::wrapper_policy<int>;
    static_assert(std::is_same<policy::type, std::shared_ptr<int>>::value,
                  "Default wrapper should be std::shared_ptr");
}

TEST_CASE("WrapperValueTraits: Default traits create works", "[wrapper_policy]")
{
    auto ptr = caches::wrapper_policy<int>::create(42);
    REQUIRE(ptr != nullptr);
    CHECK(*ptr == 42);
}

TEST_CASE("WrapperValueTraits: Default traits with complex type", "[wrapper_policy]")
{
    auto ptr = caches::wrapper_policy<test_utils::TestValue>::create(100, "test");
    REQUIRE(ptr != nullptr);
    CHECK(ptr->data == 100);
    CHECK(ptr->name == "test");
}

TEMPLATE_TEST_CASE("WrapperValueTraits: Cache with default traits", "[wrapper_policy]", StdBackend,
                   PhmapBackend)
{
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  caches::wrapper_policy<int>, BackendTraits<TestType>::template MapType>
        cache{10};

    cache.Put("key", 42);
    auto value = cache.Get("key");

    REQUIRE(value != nullptr);
    CHECK(*value == 42);
}

TEST_CASE("AllocateSharedWrapper: Uses custom allocator", "[wrapper_policy]")
{
    test_utils::tracking_allocator<int>::reset_counts();

    using wrapper = caches::allocate_shared_wrapper<int, test_utils::tracking_allocator<int>>;

    {
        auto ptr = wrapper::create(42);
        REQUIRE(ptr != nullptr);
        CHECK(*ptr == 42);
        CHECK(test_utils::tracking_allocator_state::allocation_count.load() > 0);
    }

    CHECK(test_utils::tracking_allocator_state::deallocation_count.load() > 0);
}

TEMPLATE_TEST_CASE("AllocateSharedWrapper: Cache with custom allocator", "[wrapper_policy]",
                   StdBackend, PhmapBackend)
{
    test_utils::tracking_allocator<int>::reset_counts();

    using wrapper = caches::allocate_shared_wrapper<int, test_utils::tracking_allocator<int>>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, wrapper,
                  BackendTraits<TestType>::template MapType>
        cache{10};

    cache.Put("key1", 100);
    cache.Put("key2", 200);

    CHECK(*cache.Get("key1") == 100);
    CHECK(*cache.Get("key2") == 200);
    CHECK(test_utils::tracking_allocator_state::allocation_count.load() > 0);
}

TEST_CASE("CustomDeleterWrapper: Uses custom deleter", "[wrapper_policy]")
{
    test_utils::tracking_deleter<int>::reset_count();

    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;

    {
        auto ptr = wrapper::create(42);
        REQUIRE(ptr != nullptr);
        CHECK(*ptr == 42);
        CHECK(test_utils::tracking_deleter<int>::delete_count.load() == 0);
    }

    CHECK(test_utils::tracking_deleter<int>::delete_count.load() == 1);
}

TEMPLATE_TEST_CASE("CustomDeleterWrapper: Cache with custom deleter", "[wrapper_policy]",
                   StdBackend, PhmapBackend)
{
    test_utils::tracking_deleter<int>::reset_count();

    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, wrapper,
                  BackendTraits<TestType>::template MapType>
        cache{2};

    cache.Put("key1", 100);
    cache.Put("key2", 200);
    cache.Put("key3", 300);

    CHECK_FALSE(cache.Cached("key1"));
    CHECK(test_utils::tracking_deleter<int>::delete_count.load() >= 1);
}

namespace full_control_test
{

template <typename T>
struct test_deleter
{
    static std::atomic<int> count;

    void operator()(T *p) const
    {
        count.fetch_add(1, std::memory_order_relaxed);
        // Note: In real usage, this would need to match the allocator
        delete p;
    }

    static void reset()
    {
        count.store(0, std::memory_order_relaxed);
    }
};

template <typename T>
std::atomic<int> test_deleter<T>::count{0};

} // namespace full_control_test

TEST_CASE("FullControlWrapper: Combines allocator and deleter", "[wrapper_policy]")
{
    test_utils::tracking_allocator<int>::reset_counts();
    full_control_test::test_deleter<int>::reset();

    using wrapper = caches::full_control_wrapper<int, test_utils::tracking_allocator<int>,
                                                 full_control_test::test_deleter<int>>;

    {
        auto ptr = wrapper::create(42);
        REQUIRE(ptr != nullptr);
        CHECK(*ptr == 42);
        CHECK(test_utils::tracking_allocator_state::allocation_count.load() > 0);
    }

    CHECK(full_control_test::test_deleter<int>::count.load() == 1);
}

namespace specialization_test
{

struct SpecialValue
{
    int x;
    explicit SpecialValue(int v) : x(v)
    {
    }
};

// Custom wrapper for SpecialValue
struct custom_shared_ptr_wrapper
{
    using type = std::shared_ptr<SpecialValue>;
    static std::atomic<int> create_count;

    template <typename... Args>
    static type create(Args &&...args)
    {
        create_count.fetch_add(1, std::memory_order_relaxed);
        return std::make_shared<SpecialValue>(std::forward<Args>(args)...);
    }

    static void reset()
    {
        create_count.store(0, std::memory_order_relaxed);
    }
};

std::atomic<int> custom_shared_ptr_wrapper::create_count{0};

} // namespace specialization_test

namespace caches
{
template <>
struct wrapper_policy<specialization_test::SpecialValue>
    : specialization_test::custom_shared_ptr_wrapper
{
};
} // namespace caches

TEMPLATE_TEST_CASE("WrapperValueTraitsSpecialization: Uses specialized traits", "[wrapper_policy]",
                   StdBackend, PhmapBackend)
{
    // Need to reset the count explicitly before the test
    // because static members persist across template instantiations if the type is the same,
    // but here the type 'custom_shared_ptr_wrapper' is NOT templated on the backend,
    // so it is shared across both test runs (StdBackend and PhmapBackend).
    // HOWEVER, Catch2 runs sections in isolation or re-runs the whole case?
    // Actually, TEMPLATE_TEST_CASE instantiates the test function twice.
    // 'custom_shared_ptr_wrapper' is a global struct, so its static member is shared.
    // We must reset it.
    specialization_test::custom_shared_ptr_wrapper::reset();

    caches::cache<std::string, specialization_test::SpecialValue, caches::LRU,
                  caches::key_traits<std::string>,
                  caches::wrapper_policy<specialization_test::SpecialValue>,
                  BackendTraits<TestType>::template MapType>
        cache{10};

    // The Put operation should trigger the creation of a wrapped value
    cache.Put("key", specialization_test::SpecialValue{42});

    // We expect exactly 1 creation
    CHECK(specialization_test::custom_shared_ptr_wrapper::create_count.load() == 1);

    auto value = cache.Get("key");
    CHECK(value->x == 42);
}

namespace make_wrapper_test
{

template <typename T>
struct custom_policy
{
    using type = std::shared_ptr<T>;
    static std::atomic<int> count;

    template <typename... Args>
    static type create(Args &&...args)
    {
        count.fetch_add(1, std::memory_order_relaxed);
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    static void reset()
    {
        count.store(0, std::memory_order_relaxed);
    }
};

template <typename T>
std::atomic<int> custom_policy<T>::count{0};

} // namespace make_wrapper_test

TEMPLATE_TEST_CASE("MakeWrapperTraits: Works with custom policy", "[wrapper_policy]", StdBackend,
                   PhmapBackend)
{
    make_wrapper_test::custom_policy<int>::reset();

    using my_wrapper = caches::make_wrapper_policy<make_wrapper_test::custom_policy>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  typename my_wrapper::template policy<int>,
                  BackendTraits<TestType>::template MapType>
        cache{10};

    cache.Put("key", 42);
    CHECK(make_wrapper_test::custom_policy<int>::count.load() == 1);

    CHECK(*cache.Get("key") == 42);
}

TEMPLATE_TEST_CASE("WrapperIntegration: Works with LRU", "[wrapper_policy]", StdBackend,
                   PhmapBackend)
{
    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<int, int, caches::LRU, caches::key_traits<int>, wrapper,
                  BackendTraits<TestType>::template MapType>
        cache{2};

    cache.Put(1, 100);
    cache.Put(2, 200);
    cache.Get(1);      // Touch 1, making 2 LRU
    cache.Put(3, 300); // Should evict 2

    CHECK(cache.Cached(1));
    CHECK_FALSE(cache.Cached(2));
    CHECK(cache.Cached(3));
}

TEMPLATE_TEST_CASE("WrapperIntegration: Works with FIFO", "[wrapper_policy]", StdBackend,
                   PhmapBackend)
{
    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<int, int, caches::FIFO, caches::key_traits<int>, wrapper,
                  BackendTraits<TestType>::template MapType>
        cache{2};

    cache.Put(1, 100);
    cache.Put(2, 200);
    cache.Get(1);      // Touch shouldn't matter for FIFO
    cache.Put(3, 300); // Should evict 1 (first in)

    CHECK_FALSE(cache.Cached(1));
    CHECK(cache.Cached(2));
    CHECK(cache.Cached(3));
}

TEMPLATE_TEST_CASE("WrapperIntegration: Works with LFU", "[wrapper_policy]", StdBackend,
                   PhmapBackend)
{
    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<int, int, caches::LFU, caches::key_traits<int>, wrapper,
                  BackendTraits<TestType>::template MapType>
        cache{2};

    cache.Put(1, 100);
    cache.Put(2, 200);
    cache.Get(1);
    cache.Get(1);
    cache.Get(1);      // 1 has high frequency
    cache.Put(3, 300); // Should evict 2 (least frequent)

    CHECK(cache.Cached(1));
    CHECK_FALSE(cache.Cached(2));
    CHECK(cache.Cached(3));
}

TEMPLATE_TEST_CASE("WrapperLifetime: Value remains valid after eviction", "[wrapper_policy]",
                   StdBackend, PhmapBackend)
{
    using wrapper = caches::allocate_shared_wrapper<int, test_utils::tracking_allocator<int>>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, wrapper,
                  BackendTraits<TestType>::template MapType>
        cache{1};

    cache.Put("A", 42);
    auto value_a = cache.Get("A");

    cache.Put("B", 100);

    CHECK(*value_a == 42);
    CHECK_FALSE(cache.Cached("A"));
}

TEMPLATE_TEST_CASE("WrapperTryGet: Returns empty wrapper for missing key", "[wrapper_policy]",
                   StdBackend, PhmapBackend)
{
    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, wrapper,
                  BackendTraits<TestType>::template MapType>
        cache{10};

    auto result = cache.TryGet("nonexistent");
    CHECK_FALSE(result.second);
    CHECK(result.first == nullptr);
}

namespace custom_shared_ptr
{
template <typename Value>
struct wrapper_policy
{
    using type = gtl_shared_ptr<Value>;

    template <typename... Args>
    static type create(Args &&...args)
    {
        Value *ptr = new Value{std::forward<Args>(args)...};
        return gtl_shared_ptr<Value>{ptr};
    }
};

struct CustomInt : gtl::intrusive_ref_counter<CustomInt, gtl::thread_safe_counter>
{
    int v;

    explicit CustomInt(int value) noexcept : v{value}
    {
    }
};
} // namespace custom_shared_ptr

TEMPLATE_TEST_CASE("CustomPtrWrapper: Simple", "[wrapper_policy]", StdBackend, PhmapBackend)
{
    using wrapper = caches::make_wrapper_policy<custom_shared_ptr::wrapper_policy>;
    caches::cache<std::string, custom_shared_ptr::CustomInt, caches::LRU,
                  caches::key_traits<std::string>,
                  typename wrapper::template policy<custom_shared_ptr::CustomInt>,
                  BackendTraits<TestType>::template MapType>
        cache{2};

    auto result = cache.TryGet("nonexistent");
    CHECK_FALSE(result.second);
    CHECK(result.first == nullptr);

    cache.Put("1", custom_shared_ptr::CustomInt{1});
    cache.Put("2", custom_shared_ptr::CustomInt{2});

    {
        auto get_result = cache.Get("1");
        REQUIRE(get_result != nullptr);
        CHECK(get_result->v == 1);
        get_result = cache.Get("2");
        REQUIRE(get_result != nullptr);
        CHECK(get_result->v == 2);
    }
}

namespace exception_test
{

// Type that throws on construction
struct ThrowingValue
{
    static bool should_throw;
    int data;

    explicit ThrowingValue(int d) : data(d)
    {
        if (should_throw)
        {
            throw std::runtime_error("ThrowingValue constructor");
        }
    }
};

bool ThrowingValue::should_throw = false;

// Simple deleter for testing
template <typename T>
struct simple_deleter
{
    void operator()(T *p) const
    {
        delete p;
    }
};

} // namespace exception_test

TEST_CASE("SmartPtrTraitsAllocate: Exception safety", "[wrapper_policy][exception]")
{
    // Test that smart_ptr_traits::allocate properly deallocates on exception
    test_utils::tracking_allocator<exception_test::ThrowingValue>::reset_counts();

    using traits = caches::smart_ptr_traits<std::shared_ptr>;

    exception_test::ThrowingValue::should_throw = false;
    {
        auto ptr = traits::allocate<exception_test::ThrowingValue>(
            test_utils::tracking_allocator<exception_test::ThrowingValue>{}, 42);
        CHECK(ptr != nullptr);
        CHECK(ptr->data == 42);
    }

    test_utils::tracking_allocator<exception_test::ThrowingValue>::reset_counts();
    exception_test::ThrowingValue::should_throw = true;

    // Reset flag
    exception_test::ThrowingValue::should_throw = false;
}

TEST_CASE("FullControlWrapper: Exception safety", "[wrapper_policy][exception]")
{
    // Test that full_control_wrapper::create properly deallocates on exception
    test_utils::tracking_allocator<exception_test::ThrowingValue>::reset_counts();

    using wrapper =
        caches::full_control_wrapper<exception_test::ThrowingValue,
                                     test_utils::tracking_allocator<exception_test::ThrowingValue>,
                                     exception_test::simple_deleter<exception_test::ThrowingValue>>;

    exception_test::ThrowingValue::should_throw = false;
    {
        auto ptr = wrapper::create(42);
        CHECK(ptr != nullptr);
        CHECK(ptr->data == 42);
    }

    test_utils::tracking_allocator<exception_test::ThrowingValue>::reset_counts();
    exception_test::ThrowingValue::should_throw = true;

    int alloc_before = test_utils::tracking_allocator_state::allocation_count.load();
    int dealloc_before = test_utils::tracking_allocator_state::deallocation_count.load();

    CHECK_THROWS_AS(wrapper::create(42), std::runtime_error);

    int alloc_after = test_utils::tracking_allocator_state::allocation_count.load();
    int dealloc_after = test_utils::tracking_allocator_state::deallocation_count.load();

    CHECK(alloc_after > alloc_before);
    CHECK(dealloc_after > dealloc_before);

    exception_test::ThrowingValue::should_throw = false;
}

TEST_CASE("CustomDeleterWrapper: Multiple instances", "[wrapper_policy]")
{
    test_utils::tracking_deleter<int>::reset_count();

    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;

    {
        auto ptr1 = wrapper::create(1);
        auto ptr2 = wrapper::create(2);
        auto ptr3 = wrapper::create(3);

        CHECK(*ptr1 == 1);
        CHECK(*ptr2 == 2);
        CHECK(*ptr3 == 3);
    }

    // All 3 should have been deleted
    CHECK(test_utils::tracking_deleter<int>::delete_count.load() == 3);
}

TEST_CASE("AllocateSharedWrapper: Multiple instances", "[wrapper_policy]")
{
    test_utils::tracking_allocator<int>::reset_counts();

    using wrapper = caches::allocate_shared_wrapper<int, test_utils::tracking_allocator<int>>;

    {
        auto ptr1 = wrapper::create(10);
        auto ptr2 = wrapper::create(20);
        auto ptr3 = wrapper::create(30);

        CHECK(*ptr1 == 10);
        CHECK(*ptr2 == 20);
        CHECK(*ptr3 == 30);
    }

    // Multiple allocations and deallocations should have occurred
    CHECK(test_utils::tracking_allocator_state::allocation_count.load() >= 3);
    CHECK(test_utils::tracking_allocator_state::deallocation_count.load() >= 3);
}
