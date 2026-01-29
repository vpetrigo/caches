#include "caches/caches.hpp"
#include "test_helper.hpp"

#include <array>
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <string>

TEMPLATE_TEST_CASE("LRUCache SimplePut", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, int>;
    cache_t cache(1);

    cache.Put("test", 666);

    CHECK(*cache.Get("test") == 666);
}

TEMPLATE_TEST_CASE("LRUCache PutWithUpdate", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, std::size_t>;
    constexpr std::size_t TEST_CASE = 4;
    cache_t cache{TEST_CASE};

    for (size_t i = 0; i < TEST_CASE; ++i)
    {
        cache.Put(std::to_string(i), i);

        const auto value = cache.Get(std::to_string(i));
        REQUIRE(*value == i);
    }

    for (size_t i = 0; i < TEST_CASE; ++i)
    {
        REQUIRE(cache.Cached(std::to_string(i)));
        cache.Put(std::to_string(i), i * 10);

        const auto value = cache.Get(std::to_string(i));
        REQUIRE(*value == i * 10);
    }
}

TEMPLATE_TEST_CASE("LRUCache MissingValue", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, int>;
    cache_t cache(1);

    CHECK_THROWS_AS(cache.Get("test"), std::range_error);
}

TEMPLATE_TEST_CASE("LRUCache KeepsAllValuesWithinCapacity", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, int, int>;
    constexpr int CACHE_CAP = 50;
    const int TEST_RECORDS = 100;
    cache_t cache(CACHE_CAP);

    for (int i = 0; i < TEST_RECORDS; ++i)
    {
        cache.Put(i, i);
    }

    for (int i = 0; i < TEST_RECORDS - CACHE_CAP; ++i)
    {
        CHECK_THROWS_AS(cache.Get(i), std::range_error);
    }

    for (int i = TEST_RECORDS - CACHE_CAP; i < TEST_RECORDS; ++i)
    {
        CHECK(*cache.Get(i) == i);
    }
}

TEMPLATE_TEST_CASE("LRUCache Remove_Test", "[lru]", StdBackend, PhmapBackend)
{
    constexpr std::size_t TEST_SIZE = 10;
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, std::size_t>;
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

TEMPLATE_TEST_CASE("LRUCache Partial_Remove_Test", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, int>;
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
        "key5", "key6", "key0", "key2", "key4",
    };

    for (const auto &key : access_order3)
    {
        CHECK(cache.Cached(key));
        CHECK_NOTHROW(cache.Get(key));
    }
}

TEMPLATE_TEST_CASE("LRUCache CachedCheck", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, std::size_t>;
    constexpr std::size_t TEST_SUITE = 4;
    cache_t cache(TEST_SUITE);

    for (std::size_t i = 0; i < TEST_SUITE; ++i)
    {
        cache.Put(std::to_string(i), i);
    }

    for (std::size_t i = 0; i < TEST_SUITE; ++i)
    {
        CHECK(cache.Cached(std::to_string(i)));
    }

    for (std::size_t i = TEST_SUITE; i < TEST_SUITE * 2; ++i)
    {
        CHECK_FALSE(cache.Cached(std::to_string(i)));
    }
}

TEMPLATE_TEST_CASE("LRUCache ConstructCache", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, std::size_t>;
    CHECK_THROWS_AS((cache_t(0)), std::invalid_argument);
    CHECK_NOTHROW((cache_t(1024)));
}

TEMPLATE_TEST_CASE("LRUCache TryGet", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, std::size_t>;
    constexpr std::size_t TEST_CASE{10};
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

TEMPLATE_TEST_CASE("LRUCache GetWithReplacement", "[lru]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::LRU, std::string, std::size_t>;
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

TEMPLATE_TEST_CASE("LRUCache InvalidSize", "[lru]", StdBackend, PhmapBackend)
{
    using test_type = typename TestType::template cache_t<caches::LRU, std::string, int>;
    CHECK_THROWS_AS((test_type{0}), std::invalid_argument);
}
