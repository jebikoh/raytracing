#pragma once

#include <jtxlib/math.hpp>


class BVHTree;
class PrimitiveList;

using World = BVHTree;

using Float = float;
using Vec3  = jtx::Vec3f;
using Vec2f = jtx::Vec2f;
using Ray   = jtx::Rayf;

constexpr Float INF = jtx::INFINITY_F;
constexpr Float PI  = jtx::PI_F;

using Vec2i = jtx::Vec2i;
using Vec3i = jtx::Vec3i;

using Mat4 = jtx::Mat4;

#ifdef ENABLE_CUDA_BACKEND
#define HOSTDEV __host__ __device__
#define DEV __device__
#define HOST __host__
#define INLINE __forceinline__
#else
#define HOSTDEV
#define DEV
#define HOST
#define INLINE inline
#endif

inline Float radians(const Float degrees) {
    return degrees * PI / static_cast<float>(180.0);
}