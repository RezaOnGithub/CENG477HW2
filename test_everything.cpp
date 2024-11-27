#include "Base.hpp"

#include <gtest/gtest.h>

// I don't think this test is needed
TEST(Basic, HelloTesting)
{
    EXPECT_STRNE("hello", "world");
}

TEST(Basic, LineEquation)
{
    using namespace m;
    Ray r {
        {0,0,0},
        {1,1,0}
    };
    // EXPECT_LE(line(r, {1,0,0}), 0);
}
