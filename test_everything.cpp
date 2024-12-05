#include "Base.hpp"
#include "CENG477.hpp"
#include "Rasterizer.hpp"
#include "World.hpp"

#include <cstddef>
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <numbers>
#include <optional>

namespace util
{
m::Matrix4 glm2base(glm::mat4 t)
{
    using namespace m;
    std::vector<Vec4f> c {};

    for (int x = 0; x < 4; x++)
    {
        c.push_back({ t[x][0], t[x][1], t[x][2], t[x][3] });
    }
    return Matrix4::from_columns(c);
}
}   // namespace util

// I don't think this test is needed
TEST(Basic, HelloTesting)
{
    EXPECT_STRNE("hello", "world");
}

TEST(Basic, LineEquation)
{
    using namespace m;
    Ray r {
        {0,  0, 0},
        { 1, 1, 0}
    };
    // EXPECT_LE(line(r, {1,0,0}), 0);
}

TEST(Basic, Matrix3Det1)
{
    using namespace m;
    Matrix3 mat = Matrix3::i3();
    EXPECT_EQ(1, mat.det());
}

TEST(Basic, Matrix3Det2)
{
    using namespace m;
    Matrix3 mat = Matrix3::from_rows({
        {-1, 0, 0},
        { 0, 1, 0},
        { 0, 0, 1}
    });
    EXPECT_EQ(-1, mat.det());
}

TEST(Basic, Matrix3Det3)
{
    using namespace m;
    Matrix3 mat = Matrix3::from_rows({
        {1,  2, 3},
        { 4, 5, 6},
        { 0, 0, 0}
    });
    EXPECT_EQ(0, mat.det());
}

TEST(Basic, Matrix3Det4)
{
    using namespace m;
    auto mat = Matrix3::from_rows({
        {1,  2, 3},
        { 4, 5, 6},
        { 7, 8, 9}
    });
    dprint("matrix in question", mat);
    fp res = mat.det();
    fp expected = glm::determinant(glm::mat4({
        {1,  2, 3},
        { 4, 5, 6},
        { 7, 8, 9}
    }));
    EXPECT_EQ(res, expected);
}

TEST(Basic, Matrix4Mul1)
{
    using namespace m;
    Matrix4 mat1 = Matrix4::from_rows({
        {1,  0, 0, 0},
        { 0, 1, 0, 0},
        { 0, 0, 1, 0},
        { 0, 0, 0, 1},
    });
    Matrix4 mat2 = Matrix4::from_rows({
        {1,   2,  3,  4 },
        { 5,  6,  7,  8 },
        { 9,  10, 11, 12},
        { 13, 14, 15, 16},
    });
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
    Matrix4 mat = Matrix4::from_rows({
        {1,  0, 0, 0},
        { 0, 1, 0, 0},
        { 0, 0, 1, 0},
        { 0, 0, 0, 1},
    });
    EXPECT_EQ(1, mat.det());
}

TEST(Basic, Matrix4Det2)
{
    using namespace m;
    Matrix4 mat = Matrix4::from_rows({
        {-1, 0, 0, 0},
        { 0, 1, 0, 0},
        { 0, 0, 1, 0},
        { 0, 0, 0, 1},
    });
    EXPECT_EQ(-1, mat.det());
}

TEST(Basic, Matrix4Det3)
{
    using namespace m;
    Matrix4 mat = Matrix4::from_rows({
        {1,   2,  3,  4 },
        { 5,  6,  7,  8 },
        { 9,  10, 11, 12},
        { 13, 14, 15, 16},
    });
    EXPECT_EQ(0, mat.det());
}

TEST(Basic, Matrix4Det4)
{
    using namespace m;
    Matrix4 mat = Matrix4::from_rows({
        {0,  2,  3,  4 },
        { 0, 6,  7,  8 },
        { 0, 10, 11, 12},
        { 0, 14, 15, 16},
    });
    EXPECT_EQ(0, mat.det());
}

TEST(Basic, Matrix4Invert1)
{
    using namespace m;
    Matrix4 res = Matrix4::i4().invert();
    dprint("result of inverting identity4 ", res);
    for (size_t i = 0; i < 4; i++)
    {
        for (size_t j = 0; j < 4; j++)
        {
            // printf("i = %d \t j = %d\n", i, j);
            EXPECT_FLOAT_EQ(res.rc({ i, j }), Matrix4::i4().rc({ i, j }));
        }
    }
}

TEST(Basic, Matrix4Invert2)
{
    using namespace m;
    Matrix4 t = Matrix4::from_rows({
        {0,  0, -1, 0},
        { 0, 1, 0,  0},
        { 1, 0, 0,  0},
        { 0, 0, 0,  1}
    });
    const auto expected = util::glm2base(glm::inverse(glm::transpose(glm::mat4({
        {0,  0, -1, 0},
        { 0, 1, 0,  0},
        { 1, 0, 0,  0},
        { 0, 0, 0,  1}
    }))));
    dprint("expected matrix", expected);
    Matrix4 res = t.invert();
    dprint("resultant matrix (probably wrong) ", res);
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
        {0,  0, 0},
        { 0, 0, 1},
    };
    const Matrix4 r = homorotate(ccw_angle, z);
    const Matrix4 expected = Matrix4::from_rows({
        {0,  -1, 0, 0},
        { 1, 0,  0, 0},
        { 0, 0,  1, 0},
        { 0, 0,  0, 1}
    });
    // dprint("final r", r);
    // dprint("expected value", expected);
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

TEST(Basic, Homorotate2)
{
    using namespace m;
    const fp ccw_angle = std::numbers::pi / 2;
    const Ray y {
        {0,  0, 0},
        { 0, 1, 0},
    };
    const Matrix4 r = homorotate(ccw_angle, y);
    const Matrix4 expected = Matrix4::from_rows({
        {0,   0, 1, 0},
        { 0,  1, 0, 0},
        { -1, 0, 0, 0},
        { 0,  0, 0, 1}
    });
    // dprint("final r", r);
    // dprint("expected value", expected);
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

TEST(Basic, Homorotate3)
{
    using namespace m;
    const fp ccw_angle = std::numbers::pi / 2;
    const Ray x {
        {0,  0, 0},
        { 1, 0, 0},
    };
    const Matrix4 r = homorotate(ccw_angle, x);
    const Matrix4 expected = Matrix4::from_rows({
        {1,  0, 0,  0},
        { 0, 0, -1, 0},
        { 0, 1, 0,  0},
        { 0, 0, 0,  1}
    });
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

// TODO turn this into an actual test, but I think it works so I'm leaving it
// out
TEST(Basic, PointRendering1)
{
    using namespace m;

    const bool perspective = false;
    const ViewFrustum vf { -1, 1, 1, -1, 1, 5 };
    const ViewConfig v = ViewConfig("test", 3, 3, { 0, 0, 1 }, { 0, 0, -1 },
                                    { 1, 0, 0 }, vf, {}, perspective, false);

    dprint("t_vp: ", v.t_viewport);
    dprint("v.t_projection", v.t_projection);
    dprint("v.t_camera", v.t_camera);

    const Matrix4 t_render = v.t_viewport * v.t_projection * v.t_camera;
    dprint("final point rendering matrix", t_render);

    dprint("W(0,0,0) becomes", (t_render * Vec4f(0, 0, 0, 1)).dehomogenize());
    dprint("W(1,0,0) becomes", (t_render * Vec4f(1, 0, 0, 1)).dehomogenize());
    dprint("W(1,1,0) becomes", (t_render * Vec4f(1, 1, 0, 1)).dehomogenize());

    // dprint("t_vp * [1,0,0]", t_viewport * (Vec3f(1, 0, 0).homopoint()));
    // dprint("t_vp * [0,1,0]", t_viewport * (Vec3f(0, 1, 0).homopoint()));
    // dprint("t_vp * [0,0,1]", t_viewport * (Vec3f(0, 0, 1).homopoint()));

    // dprint("t_vp * [-1,0,0]", t_viewport * (Vec3f(-1, 0, 0).homopoint()));
    // dprint("t_vp * [0,-1,0]", t_viewport * (Vec3f(0, -1, 0).homopoint()));
    // dprint("t_vp * [0,0,-1]", t_viewport * (Vec3f(0, 0, -1).homopoint()));

    // dprint("t_vp * [1,1,0]", t_viewport * (Vec3f(1, 1, 0).homopoint()));
    // dprint("t_vp * [-1,-1,0]", t_viewport * (Vec3f(-1, -1, 0).homopoint()));
}

TEST(Basic, IntersectAAInner1)
{
    Vec3f pn { 0, 0, 1 };   // sample point on plane: {0,0,-1}
    Vec4f start = Vec3f(0, 0, 0).homopoint(1);
    Vec4f end = Vec3f(0.5, 0.5, 0.5).homopoint(1);
    HomoLine input { start, end };
    HomoLine expected = input;
    std::optional<HomoLine> result = intersect_aa_inner(pn, input);

    EXPECT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(start.x, result->start.x);
    EXPECT_FLOAT_EQ(start.y, result->start.y);
    EXPECT_FLOAT_EQ(start.z, result->start.z);
    EXPECT_FLOAT_EQ(end.x, result->end.x);
    EXPECT_FLOAT_EQ(end.y, result->end.y);
    EXPECT_FLOAT_EQ(end.z, result->end.z);
}

TEST(Basic, IntersectAAInner2)
{
    Vec3f pn { 0, 0, 1 };   // sample point on plane: {0,0,-1}
    Vec4f start = Vec3f(0, 0, 0).homopoint(-1);
    Vec4f end = Vec3f(0.5, 0.5, 0.5).homopoint(-1);
    HomoLine input { start, end };
    HomoLine expected = input;
    std::optional<HomoLine> result = intersect_aa_inner(pn, input);

    EXPECT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(start.x, result->start.x);
    EXPECT_FLOAT_EQ(start.y, result->start.y);
    EXPECT_FLOAT_EQ(start.z, result->start.z);
    EXPECT_FLOAT_EQ(end.x, result->end.x);
    EXPECT_FLOAT_EQ(end.y, result->end.y);
    EXPECT_FLOAT_EQ(end.z, result->end.z);
}

TEST(Basic, IntersectAAInner3)
{
    Vec3f pn { 0, 0, 1 };   // sample point on plane: {0,0,-1}
    Vec4f start = Vec3f(0, 0, 0).homopoint(1);
    Vec4f end = Vec3f(0.5, 0.5, 0.5).homopoint(-1);
    HomoLine input { start, end };
    HomoLine expected = input;
    std::optional<HomoLine> result = intersect_aa_inner(pn, input);
    printf("The current normal is %f %f %f\n", pn.x, pn.y, pn.z);

    EXPECT_TRUE(result.has_value());
    EXPECT_FLOAT_EQ(start.x, result->start.x);
    EXPECT_FLOAT_EQ(start.y, result->start.y);
    EXPECT_FLOAT_EQ(start.z, result->start.z);
    EXPECT_FLOAT_EQ(end.x, result->end.x);
    EXPECT_FLOAT_EQ(end.y, result->end.y);
    EXPECT_FLOAT_EQ(end.z, result->end.z);
}

TEST(Basic, IntersectAAInner4)
{
    std::vector<Vec3f> pns {
        {0,   0,  1 },
        { 0,  0,  -1},
        { 0,  1,  0 },
        { 0,  -1, 0 },
        { 1,  0,  0 },
        { -1, 0,  0 }
    };
    Vec3f start3 { 0, 0, 0 };
    Vec3f end3 { 0.5, 0.5, 0.5 };
    for (auto pn : pns)
    {
        for (int i = -100; i < 101; i++)
        {
            if (i == 0)
            {
                continue;
            }
            // printf("The current normal is %f %f %f and dnom is %d\n", pn.x,
            //        pn.y, pn.z, i);
            Vec4f start = start3.homopoint(1.0 / i);
            Vec4f end = end3.homopoint(1.0 / i);
            HomoLine input { start, end };
            HomoLine expected = input;
            std::optional<HomoLine> result = intersect_aa_inner(pn, input);
            EXPECT_TRUE(result.has_value());
            EXPECT_FLOAT_EQ(start.x, result->start.x);
            EXPECT_FLOAT_EQ(start.y, result->start.y);
            EXPECT_FLOAT_EQ(start.z, result->start.z);
            EXPECT_FLOAT_EQ(end.x, result->end.x);
            EXPECT_FLOAT_EQ(end.y, result->end.y);
            EXPECT_FLOAT_EQ(end.z, result->end.z);
        }
    }
}

TEST(Basic, IntersectAAInner5)
{
    std::vector<Vec3f> pns {
        {0,   0,  1 },
        { 0,  0,  -1},
        { 0,  1,  0 },
        { 0,  -1, 0 },
        { 1,  0,  0 },
        { -1, 0,  0 }
    };
    // Trivially Fails every plane but 3
    Vec3f start3 { 10, 10, 10 };
    Vec3f end3 { 11, 11, 11 };
    int successes = 0;

    for (auto pn : pns)
    {
        for (int i = -100; i < 101; i++)
        {
            if (i == 0)
            {
                continue;
            }
            // printf("The current normal is %f %f %f and dnom is %d\n", pn.x,
            //        pn.y, pn.z, i);
            Vec4f start = start3.homopoint(1.0 / i);
            Vec4f end = end3.homopoint(1.0 / i);
            HomoLine input { start, end };
            HomoLine expected = input;
            std::optional<HomoLine> result = intersect_aa_inner(pn, input);
            if (result.has_value())
            {
                successes++;
            }
            // EXPECT_TRUE(result.has_value());
            // EXPECT_FLOAT_EQ(start.x, result->start.x);
            // EXPECT_FLOAT_EQ(start.y, result->start.y);
            // EXPECT_FLOAT_EQ(start.z, result->start.z);
            // EXPECT_FLOAT_EQ(end.x, result->end.x);
            // EXPECT_FLOAT_EQ(end.y, result->end.y);
            // EXPECT_FLOAT_EQ(end.z, result->end.z);
        }
    }
    EXPECT_EQ(successes, 600);
}

TEST(Basic, IntersectAAInner6)
{
    std::vector<Vec3f> pns {
  // {0,   0,  1 },
  // { 0,  0,  -1},
  // { 0,  1,  0 },
        {0, -1, 0},
 // { 1,  0,  0 },
  // { -1, 0,  0 }
    };
    Vec3f start3 { 0, 0, 0 };
    Vec3f end3 { 0, 10, 0 };

    for (auto pn : pns)
    {
        for (int i = -100; i < 101; i++)
        {
            if (i == 0)
            {
                continue;
            }
            printf("The current normal is %f %f %f and dnom is %d\n", pn.x,
                   pn.y, pn.z, i);
            Vec4f start = start3.homopoint(1.0 / i);
            Vec4f end = end3.homopoint(1.0 / i);
            HomoLine input { start, end };
            HomoLine expected {
                start, {0, 1, 0, 1}
            };
            std::optional<HomoLine> result = intersect_aa_inner(pn, input);
            dprint("the actual returned ending", result->end);

            EXPECT_TRUE(result.has_value());
            EXPECT_FLOAT_EQ(expected.start.dehomogenize().x,
                            result->start.dehomogenize().x);
            EXPECT_FLOAT_EQ(expected.start.dehomogenize().x,
                            result->start.dehomogenize().y);
            EXPECT_FLOAT_EQ(expected.start.dehomogenize().x,
                            result->start.dehomogenize().z);
            EXPECT_FLOAT_EQ(expected.end.dehomogenize().x,
                            result->end.dehomogenize().x);
            EXPECT_FLOAT_EQ(expected.end.dehomogenize().y,
                            result->end.dehomogenize().y);
            EXPECT_FLOAT_EQ(expected.end.dehomogenize().z,
                            result->end.dehomogenize().z);
        }
    }
}

TEST(Basic, IntersectAAInner7)
{
    std::vector<Vec3f> pns {
  // {0,   0,  1 },
  // { 0,  0,  -1},
  // { 0,  1,  0 },
        {0, -1, 0},
 // { 1,  0,  0 },
  // { -1, 0,  0 }
    };
    Vec3f start3 { 0, 0, 0 };
    Vec3f end3 { 0, 10, 0 };

    for (auto pn : pns)
    {
        for (int i = -100; i < 101; i++)
        {
            if (i == 0)
            {
                continue;
            }
            printf("The current normal is %f %f %f and dnom is %d\n", pn.x,
                   pn.y, pn.z, i);
            Vec4f start = start3.homopoint(1.0 / i);
            Vec4f end = end3.homopoint(1.0 / i);
            HomoLine input { start, end };
            HomoLine expected {
                start, {0, 1, 0, 1}
            };
            std::optional<HomoLine> result = intersect_aa_inner(pn, input);
            dprint("the actual returned ending", result->end);

            EXPECT_TRUE(result.has_value());
            EXPECT_FLOAT_EQ(expected.start.dehomogenize().x,
                            result->start.dehomogenize().x);
            EXPECT_FLOAT_EQ(expected.start.dehomogenize().x,
                            result->start.dehomogenize().y);
            EXPECT_FLOAT_EQ(expected.start.dehomogenize().x,
                            result->start.dehomogenize().z);
            EXPECT_FLOAT_EQ(expected.end.dehomogenize().x,
                            result->end.dehomogenize().x);
            EXPECT_FLOAT_EQ(expected.end.dehomogenize().y,
                            result->end.dehomogenize().y);
            EXPECT_FLOAT_EQ(expected.end.dehomogenize().z,
                            result->end.dehomogenize().z);
        }
    }
}
