/**
 * \file
 * \brief Tests for wrapper_policy customization
 */
#include "caches/caches.hpp"
#include "caches/wrapper_policy.hpp"

#include "test_helper.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <type_traits>

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

TEST(WrapperValueTraitsTest, DefaultTraitsUsesSharedPtr)
{
    using policy = caches::wrapper_policy<int>;
    static_assert(std::is_same<policy::type, std::shared_ptr<int>>::value,
                  "Default wrapper should be std::shared_ptr");
}

TEST(WrapperValueTraitsTest, DefaultTraitsCreateWorks)
{
    auto ptr = caches::wrapper_policy<int>::create(42);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(*ptr, 42);
}

TEST(WrapperValueTraitsTest, DefaultTraitsWithComplexType)
{
    auto ptr = caches::wrapper_policy<test_utils::TestValue>::create(100, "test");
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(ptr->data, 100);
    EXPECT_EQ(ptr->name, "test");
}

TEST(WrapperValueTraitsTest, CacheWithDefaultTraits)
{
    caches::cache<std::string, int> cache{10};

    cache.Put("key", 42);
    auto value = cache.Get("key");

    ASSERT_NE(value, nullptr);
    EXPECT_EQ(*value, 42);
}

TEST(AllocateSharedWrapperTest, UsesCustomAllocator)
{
    test_utils::tracking_allocator<int>::reset_counts();

    using wrapper = caches::allocate_shared_wrapper<int, test_utils::tracking_allocator<int>>;

    {
        auto ptr = wrapper::create(42);
        ASSERT_NE(ptr, nullptr);
        EXPECT_EQ(*ptr, 42);
        EXPECT_GT(test_utils::tracking_allocator_state::allocation_count.load(), 0);
    }

    EXPECT_GT(test_utils::tracking_allocator_state::deallocation_count.load(), 0);
}

TEST(AllocateSharedWrapperTest, CacheWithCustomAllocator)
{
    test_utils::tracking_allocator<int>::reset_counts();

    using wrapper = caches::allocate_shared_wrapper<int, test_utils::tracking_allocator<int>>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, wrapper> cache{
        10};

    cache.Put("key1", 100);
    cache.Put("key2", 200);

    EXPECT_EQ(*cache.Get("key1"), 100);
    EXPECT_EQ(*cache.Get("key2"), 200);
    EXPECT_GT(test_utils::tracking_allocator_state::allocation_count.load(), 0);
}

TEST(CustomDeleterWrapperTest, UsesCustomDeleter)
{
    test_utils::tracking_deleter<int>::reset_count();

    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;

    {
        auto ptr = wrapper::create(42);
        ASSERT_NE(ptr, nullptr);
        EXPECT_EQ(*ptr, 42);
        EXPECT_EQ(test_utils::tracking_deleter<int>::delete_count.load(), 0);
    }

    EXPECT_EQ(test_utils::tracking_deleter<int>::delete_count.load(), 1);
}

TEST(CustomDeleterWrapperTest, CacheWithCustomDeleter)
{
    test_utils::tracking_deleter<int>::reset_count();

    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, wrapper> cache{2};

    cache.Put("key1", 100);
    cache.Put("key2", 200);
    cache.Put("key3", 300);

    EXPECT_FALSE(cache.Cached("key1"));
    EXPECT_GE(test_utils::tracking_deleter<int>::delete_count.load(), 1);
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

TEST(FullControlWrapperTest, CombinesAllocatorAndDeleter)
{
    test_utils::tracking_allocator<int>::reset_counts();
    full_control_test::test_deleter<int>::reset();

    using wrapper = caches::full_control_wrapper<int, test_utils::tracking_allocator<int>,
                                                 full_control_test::test_deleter<int>>;

    {
        auto ptr = wrapper::create(42);
        ASSERT_NE(ptr, nullptr);
        EXPECT_EQ(*ptr, 42);
        EXPECT_GT(test_utils::tracking_allocator_state::allocation_count.load(), 0);
    }

    EXPECT_EQ(full_control_test::test_deleter<int>::count.load(), 1);
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

// Specialize wrapper_policy for SpecialValue
template <>
struct caches::wrapper_policy<specialization_test::SpecialValue>
    : specialization_test::custom_shared_ptr_wrapper
{
};

TEST(WrapperValueTraitsSpecializationTest, UsesSpecializedTraits)
{
    specialization_test::custom_shared_ptr_wrapper::reset();

    caches::cache<std::string, specialization_test::SpecialValue> cache{10};

    cache.Put("key", specialization_test::SpecialValue{42});
    EXPECT_EQ(specialization_test::custom_shared_ptr_wrapper::create_count.load(), 1);

    auto value = cache.Get("key");
    EXPECT_EQ(value->x, 42);
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

TEST(MakeWrapperTraitsTest, WorksWithCustomPolicy)
{
    make_wrapper_test::custom_policy<int>::reset();

    using my_wrapper = caches::make_wrapper_policy<make_wrapper_test::custom_policy>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>,
                  my_wrapper::policy<int>>
        cache{10};

    cache.Put("key", 42);
    EXPECT_EQ(make_wrapper_test::custom_policy<int>::count.load(), 1);

    EXPECT_EQ(*cache.Get("key"), 42);
}

TEST(WrapperIntegrationTest, WorksWithLRU)
{
    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<int, int, caches::LRU, caches::key_traits<int>, wrapper> cache{2};

    cache.Put(1, 100);
    cache.Put(2, 200);
    cache.Get(1);      // Touch 1, making 2 LRU
    cache.Put(3, 300); // Should evict 2

    EXPECT_TRUE(cache.Cached(1));
    EXPECT_FALSE(cache.Cached(2));
    EXPECT_TRUE(cache.Cached(3));
}

TEST(WrapperIntegrationTest, WorksWithFIFO)
{
    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<int, int, caches::FIFO, caches::key_traits<int>, wrapper> cache{2};

    cache.Put(1, 100);
    cache.Put(2, 200);
    cache.Get(1);      // Touch shouldn't matter for FIFO
    cache.Put(3, 300); // Should evict 1 (first in)

    EXPECT_FALSE(cache.Cached(1));
    EXPECT_TRUE(cache.Cached(2));
    EXPECT_TRUE(cache.Cached(3));
}

TEST(WrapperIntegrationTest, WorksWithLFU)
{
    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<int, int, caches::LFU, caches::key_traits<int>, wrapper> cache{2};

    cache.Put(1, 100);
    cache.Put(2, 200);
    cache.Get(1);
    cache.Get(1);
    cache.Get(1);      // 1 has high frequency
    cache.Put(3, 300); // Should evict 2 (least frequent)

    EXPECT_TRUE(cache.Cached(1));
    EXPECT_FALSE(cache.Cached(2));
    EXPECT_TRUE(cache.Cached(3));
}

TEST(WrapperLifetimeTest, ValueRemainsValidAfterEviction)
{
    using wrapper = caches::allocate_shared_wrapper<int, test_utils::tracking_allocator<int>>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, wrapper> cache{1};

    cache.Put("A", 42);
    auto value_a = cache.Get("A");

    cache.Put("B", 100);

    EXPECT_EQ(*value_a, 42);
    EXPECT_FALSE(cache.Cached("A"));
}

TEST(WrapperTryGetTest, ReturnsEmptyWrapperForMissingKey)
{
    using wrapper = caches::custom_deleter_wrapper<int, test_utils::tracking_deleter<int>>;
    caches::cache<std::string, int, caches::LRU, caches::key_traits<std::string>, wrapper> cache{
        10};

    auto result = cache.TryGet("nonexistent");
    EXPECT_FALSE(result.second);
    EXPECT_EQ(result.first, nullptr);
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

TEST(CustomPtrWrapper, Simple)
{
    using wrapper = caches::make_wrapper_policy<custom_shared_ptr::wrapper_policy>;
    caches::cache<std::string, custom_shared_ptr::CustomInt, caches::LRU,
                  caches::key_traits<std::string>, wrapper::policy<custom_shared_ptr::CustomInt>>
        cache{2};

    auto result = cache.TryGet("nonexistent");
    EXPECT_FALSE(result.second);
    EXPECT_EQ(result.first, nullptr);

    cache.Put("1", custom_shared_ptr::CustomInt{1});
    cache.Put("2", custom_shared_ptr::CustomInt{2});

    {
        auto result = cache.Get("1");
        EXPECT_NE(result, nullptr);
        EXPECT_EQ(result->v, 1);
        result = cache.Get("2");
        EXPECT_NE(result, nullptr);
        EXPECT_EQ(result->v, 2);
    }
}
