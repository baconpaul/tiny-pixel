#include "catch2.hpp"

#include "tiny-pixel/damage.h"

TEST_CASE("separate rects stay separate", "[damage]")
{
    tpx::DamageRegion d;
    d.add({0, 0, 10, 10});
    d.add({50, 50, 10, 10});
    REQUIRE(d.rects().size() == 2);
}

TEST_CASE("overlapping and touching rects merge, transitively", "[damage]")
{
    tpx::DamageRegion d;
    d.add({0, 0, 10, 10});
    d.add({20, 0, 10, 10});
    d.add({10, 0, 10, 10}); // touches both
    REQUIRE(d.rects().size() == 1);
    REQUIRE((d.rects()[0] == tpx::Rect{0, 0, 30, 10}));
}

TEST_CASE("contained and empty rects add nothing", "[damage]")
{
    tpx::DamageRegion d;
    d.add({0, 0, 10, 10});
    d.add({2, 2, 3, 3});
    d.add({});
    REQUIRE(d.rects().size() == 1);
}

TEST_CASE("too many rects collapse to the bounds", "[damage]")
{
    tpx::DamageRegion d;
    for (int i = 0; i < 9; ++i)
        d.add({i * 20, 0, 5, 5});
    REQUIRE(d.rects().size() == 1);
    REQUIRE((d.rects()[0] == tpx::Rect{0, 0, 165, 5}));
}
