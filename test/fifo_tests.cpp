#include "caches/caches.hpp"
#include "test_helper.hpp"

#include <array>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>

TEMPLATE_TEST_CASE("FIFO Cache: Simple_Test", "[fifo]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::FIFO, int, int>;
    cache_t fc(2);

    fc.Put(1, 10);
    fc.Put(2, 20);

    CHECK(fc.Size() == 2);
    CHECK(*fc.Get(1) == 10);
    CHECK(*fc.Get(2) == 20);

    fc.Put(1, 30);
    CHECK(fc.Size() == 2);
    CHECK(*fc.Get(1) == 30);

    fc.Put(3, 30);
    CHECK_THROWS_AS(fc.Get(1), std::range_error);
    CHECK(*fc.Get(2) == 20);
    CHECK(*fc.Get(3) == 30);
}

TEMPLATE_TEST_CASE("FIFO Cache: Missing_Value", "[fifo]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::FIFO, int, int>;
    cache_t fc(2);

    fc.Put(1, 10);

    CHECK(fc.Size() == 1);
    CHECK(*fc.Get(1) == 10);
    CHECK_THROWS_AS(fc.Get(2), std::range_error);
}

TEMPLATE_TEST_CASE("FIFO Cache: Sequence_Test", "[fifo]", StdBackend, PhmapBackend)
{
    constexpr int TEST_SIZE = 10;
    using cache_t = typename TestType::template cache_t<caches::FIFO, std::string, int>;
    cache_t fc(TEST_SIZE);

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        fc.Put(std::to_string('0' + i), static_cast<int>(i));
    }

    CHECK(fc.Size() == TEST_SIZE);

    for (size_t i = 0; i < TEST_SIZE; ++i)
    {
        CHECK(*fc.Get(std::to_string('0' + i)) == static_cast<int>(i));
    }

    // replace a half
    for (size_t i = 0; i < TEST_SIZE / 2; ++i)
    {
        fc.Put(std::to_string('a' + i), static_cast<int>(i));
    }

    CHECK(fc.Size() == TEST_SIZE);

    for (size_t i = 0; i < TEST_SIZE / 2; ++i)
    {
        CHECK_THROWS_AS(fc.Get(std::to_string('0' + i)), std::range_error);
    }

    for (size_t i = 0; i < TEST_SIZE / 2; ++i)
    {
        CHECK(*fc.Get(std::to_string('a' + i)) == static_cast<int>(i));
    }

    for (size_t i = TEST_SIZE / 2; i < TEST_SIZE; ++i)
    {
        CHECK(*fc.Get(std::to_string('0' + i)) == static_cast<int>(i));
    }
}

TEMPLATE_TEST_CASE("FIFO Cache: Remove_Test", "[fifo]", StdBackend, PhmapBackend)
{
    constexpr std::size_t TEST_SIZE = 10;
    using cache_t = typename TestType::template cache_t<caches::FIFO, std::string, std::size_t>;
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

TEMPLATE_TEST_CASE("FIFO Cache: Partial_Remove_Test", "[fifo]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::FIFO, std::string, int>;
    cache_t cache{5};

    for (int i = 0; i < 5; ++i)
    {
        cache.Put("key" + std::to_string(i), i);
    }

    constexpr std::array access_order = {
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

    constexpr std::array access_order3 = {
        "key5", "key6", "key1", "key2", "key4",
    };

    for (const auto &key : access_order3)
    {
        CHECK(cache.Cached(key));
        CHECK_NOTHROW(cache.Get(key));
    }
}

TEMPLATE_TEST_CASE("FIFO Cache: TryGet", "[fifo]", StdBackend, PhmapBackend)
{
    constexpr std::size_t TEST_CASE{10};
    using cache_t = typename TestType::template cache_t<caches::FIFO, std::string, std::size_t>;
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

TEMPLATE_TEST_CASE("FIFO Cache: GetWithReplacement", "[fifo]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::FIFO, std::string, std::size_t>;
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

TEMPLATE_TEST_CASE("FIFO Cache: InvalidSize", "[fifo]", StdBackend, PhmapBackend)
{
    using test_type = typename TestType::template cache_t<caches::FIFO, std::string, int>;
    CHECK_THROWS_AS(test_type{0}, std::invalid_argument);
}
