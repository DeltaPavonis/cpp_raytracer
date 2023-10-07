#ifndef RAY3D_H
#define RAY3D_H

#include <cmath>
#include "vec3d.h"
#include "sphere.h"
using namespace std;

struct Ray3D {
    Point3D origin;
    Vec3D dir;

    /* Evaluate the ray at time `t` */
    auto operator() (double t) const {return origin + t * dir;}

    Ray3D(const Point3D &origin_, const Vec3D &dir_) : origin{origin_}, dir{dir_} {}

    /* --- RAY-GEOMETRY INTERSECTIONS --- */

    /* A ray hits a sphere iff it intersects its surface. Now, a sphere with radius R centered at
    C = (sx, sy, sz) can be expressed as the vector equation (P - C) dot (P - C) = R^2; any point
    P that satisfies this equation is on the sphere. So, the ray P(t) = A + tB hits the sphere if
    there exists some t for which (P(t) - C) dot (P(t) - C) = R^2; if we can find a solution to
    the equation ((A + tb) - C) dot ((A + tb) - C) = R^2. Grouping the t's, rearranging, and
    expanding, we find that t^2(b dot b) + 2tb dot (A - C) + (A - C) dot (A - C) - r^2 = 0.
    This is a quadratic in t; ax^2 + bx + c = 0, where
    a = b dot b
    b = 2b dot (A - C)
    c = (A - C) dot (A - C) - r^2
    So, we use the Quadratic formula to find if any t exists which solves the equation.

    Note: In reality, we need to find a non-negative solution of t, because the ray heads in
    that direction. The current function would return true if the LINE intersects the sphere,
    not necessarily the ray (so if the sphere was located behind the camera, it could still be
    drawn). We will fix this in the future. */
    
    /* `hit` returns the smallest `t` (assuming all solutions for `t` are non-negative) where this
    ray intersects the sphere `s`. If there are no solutions for `t`, returns a negative number. */
    double hit(const Sphere &s) const {
        auto origin_to_center = origin - s.center;
        auto a = dot(dir, dir);
        auto b_half = dot(dir, origin_to_center);
        auto c = dot(origin_to_center, origin_to_center) - s.radius * s.radius;
        auto discriminant_quarter = b_half * b_half - a * c;

        /* Quadratic has a solution iff discriminant is non-negative */
        return (discriminant_quarter >= 0 ? (-b_half - std::sqrt(discriminant_quarter)) / a : -1);
    }
};

#endif