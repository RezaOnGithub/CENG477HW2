#include "Base.hpp"
#include "CENG477.hpp"

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
        { 0, 0, 0 },
        { 1, 1, 0 }
    };
    // EXPECT_LE(line(r, {1,0,0}), 0);
}

TEST(Basic, Matrix3Det1)
{
    using namespace m;
    Matrix3 mat {
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 }
    };
    EXPECT_EQ(1, mat.det());
}

TEST(Basic, Matrix3Det2)
{
    using namespace m;
    Matrix3 mat {
        { -1, 0, 0 },
        { 0,  1, 0 },
        { 0,  0, 1 }
    };
    EXPECT_EQ(-1, mat.det());
}

TEST(Basic, Matrix3Det3)
{
    using namespace m;
    Matrix3 mat {
        { 1, 2, 3 },
        { 4, 5, 6 },
        { 0, 0, 0 }
    };
    EXPECT_EQ(0, mat.det());
}

TEST(Basic, Matrix3Det4)
{
    using namespace m;
    Matrix3 mat {
        { 1, 2, 3 },
        { 4, 5, 6 },
        { 7, 8, 9 }
    };
    EXPECT_EQ(0, mat.det());
}

TEST(Basic, Matrix4Mul1)
{
    using namespace m;
    Matrix4 mat1 {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 },
    };
    Matrix4 mat2 {
        { 1,  2,  3,  4  },
        { 5,  6,  7,  8  },
        { 9,  10, 11, 12 },
        { 13, 14, 15, 16 },
    };
    Matrix4 res = mat1 * mat2;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            EXPECT_FLOAT_EQ(res.column(i).row(j), mat2.column(i).row(j));
        }
    }
}

TEST(Basic, Matrix4Det1)
{
    using namespace m;
    Matrix4 mat {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 },
    };
    EXPECT_EQ(1, mat.det());
}

TEST(Basic, Matrix4Det2)
{
    using namespace m;
    Matrix4 mat {
        { -1, 0, 0, 0 },
        { 0,  1, 0, 0 },
        { 0,  0, 1, 0 },
        { 0,  0, 0, 1 },
    };
    EXPECT_EQ(-1, mat.det());
}

TEST(Basic, Matrix4Det3)
{
    using namespace m;
    Matrix4 mat {
        { 1,  2,  3,  4  },
        { 5,  6,  7,  8  },
        { 9,  10, 11, 12 },
        { 13, 14, 15, 16 },
    };
    EXPECT_EQ(0, mat.det());
}

TEST(Basic, Matrix4Det4)
{
    using namespace m;
    Matrix4 mat {
        { 0, 2,  3,  4  },
        { 0, 6,  7,  8  },
        { 0, 10, 11, 12 },
        { 0, 14, 15, 16 },
    };
    EXPECT_EQ(0, mat.det());
}

TEST(Basic, Matrix4Invert1)
{
    using namespace m;
    Matrix4 identity {
        { 1, 0, 0, 0 },
        { 0, 1, 0, 0 },
        { 0, 0, 1, 0 },
        { 0, 0, 0, 1 },
    };
    Matrix4 res = identity.invert();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // printf("i = %d \t j = %d\n", i, j);
            EXPECT_FLOAT_EQ(res.column(i).row(j), identity.column(i).row(j));
        }
    }
}

TEST(Basic, Matrix4Invert2)
{
    using namespace m;
    Matrix4 t = Matrix4({
                            { 0, 0, -1, 0 },
                            { 0, 1, 0,  0 },
                            { 1, 0, 0,  0 },
                            { 0, 0, 0,  1 }
    })
                    .transpose();
    Matrix4 expected = Matrix4({
                                   { 0,  0, 1, 0 },
                                   { 0,  1, 0, 0 },
                                   { -1, 0, 0, 0 },
                                   { 0,  0, 0, 1 },
    })
                           .transpose();
    Matrix4 res = t.invert();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // printf("i = %d \t j = %d\n", i, j);
            EXPECT_FLOAT_EQ(res.column(i).row(j), expected.column(i).row(j));
        }
    }
}

TEST(Basic, Homorotate1)
{
    using namespace m;
    const fp ccw_angle = std::numbers::pi / 2;
    const Ray z {
        { 0, 0, 0 },
        { 0, 0, 1 },
    };
    const Matrix4 r = homorotate(ccw_angle, z);
    const Matrix4 expected =
        Matrix4({ 0, -1, 0, 0 }, { 1, 0, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1
        })
            .transpose();
    // dprint("final r", r);
    // dprint("expected value", expected);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // printf("i = %d\tj =%d\n", i, j);
            EXPECT_TRUE(eq_within(r.column(j).row(i),
            expected.column(j).row(i),
                                  ceng::EPSILON));
        }
    }
}

TEST(Basic, Homorotate2)
{
    using namespace m;
    const fp ccw_angle = std::numbers::pi / 2;
    const Ray y {
        { 0, 0, 0 },
        { 0, 1, 0 },
    };
    const Matrix4 r = homorotate(ccw_angle, y);
    const Matrix4 expected = Matrix4({
                                         { 0,  0, 1, 0 },
                                         { 0,  1, 0, 0 },
                                         { -1, 0, 0, 0 },
                                         { 0,  0, 0, 1 }
    })
                                 .transpose();
    // dprint("final r", r);
    // dprint("expected value", expected);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // printf("i = %d\tj =%d\n", i, j);
            EXPECT_TRUE(eq_within(r.column(j).row(i),
            expected.column(j).row(i),
                                  ceng::EPSILON));
        }
    }
}

TEST(Basic, Homorotate3)
{
    using namespace m;
    const fp ccw_angle = std::numbers::pi / 2;
    const Ray x {
        { 0, 0, 0 },
        { 1, 0, 0 },
    };
    const Matrix4 r = homorotate(ccw_angle, x);
    const Matrix4 expected =
        Matrix4({ 1, 0, 0, 0 }, { 0, 0, -1, 0 }, { 0, 1, 0, 0 }, { 0, 0, 0, 1 })
            .transpose();
    dprint("final r", r);
    dprint("expected value", expected);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            // printf("i = %d\tj =%d\n", i, j);
            EXPECT_TRUE(eq_within(r.column(j).row(i), expected.column(j).row(i),
                                  ceng::EPSILON));
        }
    }
}
