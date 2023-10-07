#ifndef SPHERE_H
#define SPHERE_H

#include <limits>
#include "hittable.h"
#include "vec3d.h"
#include "ray3d.h"

/* A sphere in 3D space is represented by its center and its radius. */
struct Sphere : Hittable {
    Point3D center;
    double radius;

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

    /* `Sphere::hit_by(ray)` returns the smallest `t` (assuming all solutions for `t` are
    non-negative) where the ray `ray` intersects this sphere. If there are no solutions for `t`,
    returns a negative number. */
    hit_info hit_by(const Ray3D &ray, double t_min = 0,
                    double t_max = std::numeric_limits<double>::infinity()) const {
        
        /* Set up quadratic formula calculation */
        auto origin_to_center = ray.origin - center;
        auto a = dot(ray.dir, ray.dir);
        auto b_half = dot(ray.dir, origin_to_center);
        auto c = dot(origin_to_center, origin_to_center) - radius * radius;
        auto discriminant_quarter = b_half * b_half - a * c;

        /* Quadratic has no solutions whenever the discriminant is negative */
        if (discriminant_quarter < 0) {return {};}

        /* If the quadratic has solutions, find the smallest one in the range [t_min, t_max] */
        auto discriminant_quarter_sqrt = std::sqrt(discriminant_quarter);  /* Evaluate this once */
        auto root = (-b_half - discriminant_quarter_sqrt) / a;  /* Check smaller root first*/

        if (!(t_min <= root && root <= t_max)) {

            /* Smaller root not in [t_min, t_max], try the other root */
            root = (-b_half + discriminant_quarter_sqrt) / a;

            if (!(t_min <= root && root <= t_max)) {
                /* No root in the range [t_min, t_max], return {} */
                return {};
            }
        }

        auto hit_point = ray(root);  /* Evaluate this once */
        return hit_info {
            .t = root,
            .hit_point = hit_point,
            .surface_normal = (hit_point - center).unit_vector()
        };
    }

    Sphere(const Point3D &center_, double radius_) : center{center_}, radius{radius_} {}
};

#endif