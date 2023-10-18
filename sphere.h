#ifndef SPHERE_H
#define SPHERE_H

#include <memory>
#include "hittable.h"
#include "vec3d.h"
#include "ray3d.h"
#include "material.h"

/* A sphere in 3D space is represented by its center and its radius. */
struct Sphere : public Hittable {
    Point3D center;
    double radius;
    std::shared_ptr<Material> material;

    /* A ray hits a sphere iff it intersects its surface. Now, a sphere with radius R centered at
    C = (sx, sy, sz) can be expressed as the vector equation (P - C) dot (P - C) = R^2; any point
    P that satisfies this equation is on the sphere. So, the ray P(t) = A + tB hits the sphere if
    there exists some t for which (P(t) - C) dot (P(t) - C) = R^2; if we can find a solution to
    the equation ((A + tb) - C) dot ((A + tb) - C) = R^2. Grouping the t's, rearranging, and
    expanding, we find that t^2(b dot b) + 2tb dot (A - C) + (A - C) dot (A - C) - r^2 = 0.
    This is a quadratic in t; at^2 + bt + c = 0, where
    a = b dot b
    b = 2b dot (A - C)
    c = (A - C) dot (A - C) - r^2
    So, we use the Quadratic formula to find if any t exists which solves the equation.

    Note: In reality, we need to find a non-negative solution of t, because the ray heads in
    that direction. The current function would return true if the LINE intersects the sphere,
    not necessarily the ray (so if the sphere was located behind the camera, it could still be
    drawn). We will fix this in the future. */

    /* `Sphere::hit_by(ray)` returns a `hit_info` object representing the minimum time of
    intersection in the time range specified by `ray_times`, of `ray` with this Sphere. */
    hit_info hit_by(const Ray3D &ray,
                    const Interval &ray_times = Interval::nonnegative()) const override {
        
        /* Set up quadratic formula calculation */
        auto center_to_origin = ray.origin - center;
        auto a = dot(ray.dir, ray.dir);
        auto b_half = dot(ray.dir, center_to_origin);
        auto c = dot(center_to_origin, center_to_origin) - radius * radius;
        auto discriminant_quarter = b_half * b_half - a * c;

        /* Quadratic has no solutions whenever the discriminant is negative */
        if (discriminant_quarter < 0) {return {};}

        /* If the quadratic has solutions, find the smallest one in the range `ray_times` */
        auto discriminant_quarter_sqrt = std::sqrt(discriminant_quarter);  /* Evaluate this once */
        auto root = (-b_half - discriminant_quarter_sqrt) / a;  /* Check smaller root first */

        if (!ray_times.contains_exclusive(root)) {
            /* Smaller root not in the range `ray_times`, try the other root */
            root = (-b_half + discriminant_quarter_sqrt) / a;

            if (!ray_times.contains_exclusive(root)) {
                /* No root in the range `ray_times`, return {} */
                return {};
            }
        }

        /* Shadow acne occurs when `hit_time` is a little too large; that causes `hit_point`
        to be inside the Sphere, and so the next reflected ray will hit the inside of the sphere
        at a very small time and then continue to bounce off the inside of the sphere over and
        over. I fix this in a different method from the book: I decrease the hit time. Specifically,
        if `hit_time` > 1e-10, I simply subtract 1e-10 from it, and otherwise I halve it. */
        root = (root > 1e-10 ? root - 1e-10 : root / 2);

        auto hit_point = ray(root);  /* Evaluate this once */
        auto outward_unit_normal = (hit_point - center) / radius;
        return hit_info(root, hit_point, outward_unit_normal, ray, material);
    }

    /* Constructs a Sphere with center `center_`, radius `radius_`, and material
    specified by `material_` */
    Sphere(const Point3D &center_, double radius_, std::shared_ptr<Material> material_)
        : center{center_}, radius{radius_}, material{material_} {}
};

#endif