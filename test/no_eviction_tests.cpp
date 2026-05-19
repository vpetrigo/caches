#include "caches/caches.hpp"
#include "test_helper.hpp"
#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_test_macros.hpp>
#include <stdexcept>
#include <string>

TEMPLATE_TEST_CASE("NoEvictionCache: Add one element", "[NoEvictionCache]", StdBackend,
                   PhmapBackend)
{
    constexpr std::size_t cache_size = 1;
    using cache_t = typename TestType::template cache_t<caches::NoEviction, std::string, int>;
    cache_t cache(cache_size);

    cache.Put("Hello", 1);
    CHECK(*cache.Get("Hello") == 1);
}

TEMPLATE_TEST_CASE("NoEvictionCache: Add delete add one element", "[NoEvictionCache]", StdBackend,
                   PhmapBackend)
{
    constexpr std::size_t cache_size = 1;
    using cache_t = typename TestType::template cache_t<caches::NoEviction, std::string, int>;
    cache_t cache(cache_size);

    cache.Put("Hello", 1);
    cache.Put("World", 2);
    CHECK_THROWS_AS(cache.Get("Hello"), std::range_error);
    CHECK(*cache.Get("World") == 2);
}

TEMPLATE_TEST_CASE("NoEvictionCache: Add many elements", "[NoEvictionCache]", StdBackend,
                   PhmapBackend)
{
    constexpr std::size_t cache_size = 1024;
    using cache_t =
        typename TestType::template cache_t<caches::NoEviction, std::string, std::size_t>;
    cache_t cache(cache_size);

    for (std::size_t i = 0; i < cache_size; ++i)
    {
        cache.Put(std::to_string(i), i);
    }

    CHECK(cache.Size() == cache_size);

    for (std::size_t i = 0; i < cache_size; ++i)
    {
        CHECK(*cache.Get(std::to_string(i)) == i);
    }
}

TEMPLATE_TEST_CASE("NoEvictionCache: Small cache many elements", "[NoEvictionCache]", StdBackend,
                   PhmapBackend)
{
    constexpr std::size_t cache_size = 1;
    using cache_t =
        typename TestType::template cache_t<caches::NoEviction, std::string, std::size_t>;
    cache_t cache(cache_size);

    for (std::size_t i = 0; i < cache_size; ++i)
    {
        std::string temp_key = std::to_string(i);
        cache.Put(temp_key, i);
        CHECK(*cache.Get(temp_key) == i);
    }

    CHECK(cache.Size() == cache_size);
}

TEMPLATE_TEST_CASE("NoEvictionCache: Remove Test", "[NoEvictionCache]", StdBackend, PhmapBackend)
{
    constexpr std::size_t TEST_SIZE = 10;
    using cache_t =
        typename TestType::template cache_t<caches::NoEviction, std::string, std::size_t>;
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

TEMPLATE_TEST_CASE("NoEvictionCache: TryGet", "[NoEvictionCache]", StdBackend, PhmapBackend)
{
    constexpr std::size_t TEST_CASE{10};
    using cache_t =
        typename TestType::template cache_t<caches::NoEviction, std::string, std::size_t>;
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

TEMPLATE_TEST_CASE("NoEvictionCache: GetWithReplacement", "[NoEvictionCache]", StdBackend,
                   PhmapBackend)
{
    using cache_t =
        typename TestType::template cache_t<caches::NoEviction, std::string, std::size_t>;
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

TEMPLATE_TEST_CASE("NoEvictionCache: InvalidSize", "[NoEvictionCache]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::NoEviction, std::string, int>;
    CHECK_THROWS_AS(cache_t{0}, std::invalid_argument);
}

TEMPLATE_TEST_CASE("NoEviction ReplCandidate Consistency", "[NoEvictionCache]", StdBackend,
                   PhmapBackend)
{
    constexpr std::size_t cache_size = 3;
    using cache_t = typename TestType::template cache_t<caches::NoEviction, int, int>;
    cache_t cache{cache_size};

    cache.Put(1, 10);
    cache.Put(2, 20);
    cache.Put(3, 30);

    CHECK(cache.Size() == cache_size);

    for (int i = 4; i <= 10; ++i)
    {
        cache.Put(i, i * 10);
        CHECK(cache.Size() == cache_size);
    }

    int present_count = 0;
    for (int i = 1; i <= 10; ++i)
    {
        if (cache.Cached(i))
            present_count++;
    }

    CHECK(present_count == static_cast<int>(cache_size));
}

TEMPLATE_TEST_CASE("NoEviction RapidEvictions", "[NoEvictionCache]", StdBackend, PhmapBackend)
{
    using cache_t = typename TestType::template cache_t<caches::NoEviction, int, int>;
    cache_t cache{2};

    for (int i = 0; i < 10; ++i)
    {
        cache.Put(i, i * 10);
        CHECK(cache.Size() <= 2);
    }

    CHECK(cache.Size() == 2);
}
