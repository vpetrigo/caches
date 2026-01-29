#include "caches/caches.hpp"
#include "test_helper.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("LFUCache Simple_Test", "[lfu]", StdBackend, PhmapBackend)
{
    constexpr size_t FIRST_FREQ = 10;
    constexpr size_t SECOND_FREQ = 9;
    constexpr size_t THIRD_FREQ = 8;
    using cache_t = typename TestType::template cache_t<caches::LFU, std::string, int>;
    cache_t cache(3);

    cache.Put("A", 1);
    cache.Put("B", 2);
    cache.Put("C", 3);

    for (size_t i = 0; i < FIRST_FREQ; ++i)
    {
        CHECK(*cache.Get("B") == 2);
    }

    for (size_t i = 0; i < SECOND_FREQ; ++i)
    {
        CHECK(*cache.Get("C") == 3);
    }

    for (size_t i = 0; i < THIRD_FREQ; ++i)
    {
        CHECK(*cache.Get("A") == 1);
    }

    cache.Put("D", 4);

    CHECK(*cache.Get("B") == 2);
    CHECK(*cache.Get("C") == 3);
    CHECK(*cache.Get("D") == 4);
    CHECK_THROWS_AS(cache.Get("A"), std::range_error);
}

TEMPLATE_TEST_CASE("LFUCache Single_Slot", "[lfu]", StdBackend, PhmapBackend)
{
    constexpr size_t TEST_SIZE = 5;
    using cache_t = typename TestType::template cache_t<caches::LFU, int, int>;
    cache_t cache(1);

    cache.Put(1, 10);

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        cache.Put(1, static_cast<int>(i));
    }

    CHECK(*cache.Get(1) == 4);

    cache.Put(2, 20);

    CHECK_THROWS_AS(cache.Get(1), std::range_error);
    CHECK(*cache.Get(2) == 20);
}

TEMPLATE_TEST_CASE("LFUCache FrequencyIssue", "[lfu]", StdBackend, PhmapBackend)
{
    constexpr size_t TEST_SIZE = 50;
    using cache_t = typename TestType::template cache_t<caches::LFU, int, int>;
    cache_t cache(3);

    cache.Put(1, 10);
    cache.Put(2, 1);
    cache.Put(3, 2);

    // cache value with key '1' will have the counter 50
    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        CHECK_NOTHROW(cache.Get(1));
    }

    cache.Put(4, 3);
    cache.Put(5, 4);

    CHECK(*cache.Get(1) == 10);
    CHECK(*cache.Get(2) == 1);
    CHECK(*cache.Get(5) == 4);
    CHECK_THROWS_AS(cache.Get(3), std::range_error);
    CHECK_THROWS_AS(cache.Get(4), std::range_error);

    cache.Put(6, 5);
    cache.Put(7, 6);

    CHECK(*cache.Get(1) == 10);
    CHECK(*cache.Get(5) == 4);
    CHECK(*cache.Get(7) == 6);
    CHECK_THROWS_AS(cache.Get(3), std::range_error);
    CHECK_THROWS_AS(cache.Get(6), std::range_error);
}

TEMPLATE_TEST_CASE("LFUCache Remove_Test", "[lfu]", StdBackend, PhmapBackend)
{
    constexpr std::size_t TEST_SIZE = 10;
    using cache_t = typename TestType::template cache_t<caches::LFU, std::string, std::size_t>;
    cache_t fc(TEST_SIZE);

    for (std::size_t i = 0; i < TEST_SIZE; ++i)
    {
        fc.Put(std::to_string(i), i);
    }

    CHECK(fc.Size() == TEST_SIZE);

    for (std::size_t i = 0; i < TEST_SIZE; ++i)
    {
        CHECK(fc.Remove(std::to_string(i)));
    }

    CHECK(fc.Size() == 0);

    for (std::size_t i = 0; i < TEST_SIZE; ++i)
    {
        CHECK_FALSE(fc.Remove(std::to_string(i)));
    }
}

TEMPLATE_TEST_CASE("LFUCache Partial_Remove_Test", "[lfu]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LFU, std::string, int>;
    cache_t cache{5};

    for (int i = 0; i < 5; ++i)
    {
        cache.Put("key" + std::to_string(i), i);
    }

    const std::array<const char *, 5> access_order = {
        "key1", "key3", "key0", "key4", "key2",
    };

    for (const auto &key : access_order)
    {
        CHECK(cache.Get(key) != nullptr);
    }

    cache.Remove("key3");

    for (int i = 0; i < 5; ++i)
    {
        if (const auto key = "key" + std::to_string(i); key != "key3")
        {
            CHECK(cache.Cached(key));
        }
        else
        {
            CHECK_FALSE(cache.Cached(key));
        }
    }

    cache.Put("key5", 5);
    cache.Put("key6", 6);

    const std::array<const char *, 5> access_order3 = {
        "key0", "key6", "key1", "key4", "key2",
    };

    for (const auto &key : access_order3)
    {
        CHECK(cache.Cached(key));
        CHECK_NOTHROW(cache.Get(key));
    }
}

TEMPLATE_TEST_CASE("LFUCache TryGet", "[lfu]", StdBackend, PhmapBackend)
{
    constexpr std::size_t TEST_CASE{10};
    using cache_t = typename TestType::template cache_t<caches::LFU, std::string, std::size_t>;
    cache_t cache{TEST_CASE};

    for (std::size_t i = 0; i < TEST_CASE; ++i)
    {
        cache.Put(std::to_string(i), i);
    }

    for (std::size_t i = 0; i < TEST_CASE; ++i)
    {
        auto element = cache.TryGet(std::to_string(i));
        CHECK(element.second);
        CHECK(*element.first == i);
    }

    for (std::size_t i = TEST_CASE; i < TEST_CASE * 2; ++i)
    {
        auto element = cache.TryGet(std::to_string(i));
        CHECK_FALSE(element.second);
    }
}

TEMPLATE_TEST_CASE("LFUCache GetWithReplacement", "[lfu]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LFU, std::string, std::size_t>;
    cache_t cache{2};

    cache.Put("1", 1);
    cache.Put("2", 2);

    auto element1 = cache.Get("1");
    auto element2 = cache.Get("2");
    CHECK(*element1 == 1);
    CHECK(*element2 == 2);
    cache.Put("3", 3);
    auto element3 = cache.Get("3");
    CHECK(*element3 == 3);

    std::string replaced_key;

    for (size_t i = 1; i <= 2; ++i)
    {
        const auto key = std::to_string(i);

        if (!cache.Cached(key))
        {
            replaced_key = key;
        }
    }

    CHECK_FALSE(cache.Cached(replaced_key));
    CHECK_FALSE(cache.TryGet(replaced_key).second);
    CHECK_THROWS_AS(cache.Get(replaced_key), std::range_error);
    CHECK(*element1 == 1);
    CHECK(*element2 == 2);
    CHECK(*element3 == 3);
}

TEMPLATE_TEST_CASE("LFUCache InvalidSize", "[lfu]", StdBackend, PhmapBackend)
{
    using test_type = typename TestType::template cache_t<caches::LFU, std::string, int>;
    CHECK_THROWS_AS(test_type{0}, std::invalid_argument);
}
