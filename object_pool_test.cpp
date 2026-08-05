#include <catch2/catch_test_macros.hpp>

#include <unordered_set>
#include <vector>

#include "object_pool.h"

TEST_CASE("object pool allocate returns a usable object") {
    ObjectPool<int> pool;

    int* item = pool.allocate();
    *item = 42;

    REQUIRE(item != nullptr);
    CHECK(*item == 42);
}

TEST_CASE("object pool reuses deallocated storage") {
    ObjectPool<int> pool;

    int* first = pool.allocate();
    *first = 99;
    pool.deallocate(first);

    int* second = pool.allocate();
    *second = 2;

    CHECK(second == first);
    CHECK(*second == 2);
}

TEST_CASE("object pool grows beyond one batch") {
    ObjectPool<int> pool;
    std::vector<int*> allocated;
    allocated.reserve(65);

    for (int i{ 0 }; i < 65; ++i) {
        int* item = pool.allocate();
        *item = i;
        allocated.push_back(item);
    }

    std::unordered_set<int*> unique_addresses{ allocated.begin(), allocated.end() };
    CHECK(unique_addresses.size() == allocated.size());

    for (int i{ 0 }; i < 65; ++i) {
        CHECK(*allocated[i] == i);
    }
}
