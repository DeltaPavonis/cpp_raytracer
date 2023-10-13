#ifndef HITTABLE_AND_HIT_INFO_H
#define HITTABLE_AND_HIT_INFO_H

#include <limits>
#include <cmath>
#include "ray3d.h"
#include "interval.h"

struct hit_info {
    /*
    hit_time: Time of intersection
    hit_point: Point of intersection
    surface_normal: Surface normal at the point of intersection
    
    hit_time = -1 signifies no intersection. */
    double hit_time = std::numeric_limits<double>::infinity();
    Point3D hit_point{};
    Vec3D surface_normal{};
    bool hit_from_outside = false;  /* Named `front_face` in the tutorial */

    /* Converts to `true` if it represents the presence of an intersection, and false if not */
    operator bool() {return !std::isinf(hit_time);}

    /* --- CONSTRUCTORS ---*/

    /* Default constructor denotes "no intersection"; `hit_time` is set to
    `std::numeric_limits<double>::infinity()`. */
    hit_info() {}

    /* Constructs a `hit_info` given `hit_time_` (the hit time), `hit_point_` (the point at
    which the ray intersects the surface), `outward_surface_normal` (an UNIT VECTOR equalling
    the normal to the surface at the ray's hit point), and finally, the ray `ray` itself.
    
    Again, `outward_surface_normal` is assumed to be an unit vector. */
    hit_info(double hit_time_, const Point3D &hit_point_, const Vec3D &outward_surface_normal,
             const Ray3D &ray)
        : hit_time{hit_time_}, hit_point{hit_point_}
    {
        /* Determine, based on the directions of the ray and the outward surface normal at
        the ray's point of intersection, whether the ray was shot from inside the surface
        or from outside the surface. */
        if (dot(ray.dir, outward_surface_normal) > 0) {
            /* Then the angle between the ray and the outward surface normal is in [0, 90) degrees,
            which means the ray originated INSIDE the object. */
            surface_normal = -outward_surface_normal;  /* So flip outward surface normal */
            hit_from_outside = false;
        } else {
            /* Then the angle between the ray and the outward surface normal is in [90, 180)
            degrees, which means the ray originated OUTSIDE the object. */
            surface_normal = outward_surface_normal;
            hit_from_outside = true;
        }
    }
};

struct Hittable {
    
    /* Check if the object is hit by a ray in the time range (t_min, t_max).
    = 0 causes `hit_by` to be a pure virtual function, and so `Hittable` is an abstract
    class, which means it itself cannot be instantiated (good, it's only an interface). */
    virtual hit_info hit_by(const Ray3D &ray, const Interval &ray_times) const = 0;

    virtual ~Hittable() = default;
};

#endif