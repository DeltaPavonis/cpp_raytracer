#ifndef HITTABLE_AND_HIT_INFO_H
#define HITTABLE_AND_HIT_INFO_H

#include <iostream>
#include <limits>
#include <cmath>
#include <memory>
#include <optional>
#include "ray3d.h"
#include "interval.h"
#include "aabb.h"

/* Forward-declare the class `Material` to avoid circular dependencies of
"material.h" and "hittable.h" on each other */
class Material;

/* `hit_info` stores information about a given ray-object intersection, including
its hit time, hit point, unit surface normal, front vs back face detection, as
well as the material of the object hit. */
struct hit_info {

    /* `hit_time` = The time where the ray intersects an object */
    double hit_time;
    /* `hit_point` = The point at which the ray intersects the object. If `ray`
    is the ray, then `hit_point` is equivalent to `ray(hit_time)`. */
    Point3D hit_point;
    /* `unit_surface_normal` = The unit vector normal to the surface at the point of intersection.
    `unit_surface_normal` points outward if the ray hit the outside of the surface, and it points
    inward if the ray hit the inside of the surface. Thus, `unit_surface_normal` represents the
    true unit surface normal at the hit point, taking into account whether the front or back face
    of the surface was hit. */
    Vec3D unit_surface_normal;
    /* `hit_from_outside` = Whether or not the ray hit the outside of the surface. */
    bool hit_from_outside = false;  /* Named `front_face` in the tutorial */
    /* `material` points to the `Material` of the object which the ray intersected */
    const Material* material;

    /* --- CONSTRUCTORS ---*/

    /* Constructs a `hit_info` given `hit_time_` (the hit time), `hit_point_` (the point at
    which the ray intersects the surface), `outward_unit_surface_normal` (an UNIT VECTOR equalling
    the normal to the surface at the ray's hit point), the ray `ray` itself, and finally, 
    `material_` (the material of the surface that `ray` hit).
    
    Again, `outward_unit_surface_normal` is assumed to be an unit vector. */
    hit_info(double hit_time_, const Point3D &hit_point_, const Vec3D &outward_unit_surface_normal,
             const Ray3D &ray, const std::shared_ptr<Material> &material_)
        : hit_time{hit_time_}, hit_point{hit_point_}, material{material_.get()}
    {
        /* Determine, based on the directions of the ray and the outward surface normal at
        the ray's point of intersection, whether the ray was shot from inside the surface
        or from outside the surface. Set `unit_surface_normal` correspondingly: if the ray
        was shot from inside the surface, then the unit surface normal should point inward,
        and if the ray was shot from outside the surface, then the unit surface normal should
        point outward. */
        if (dot(ray.dir, outward_unit_surface_normal) > 0) {
            /* Then the angle between the ray and the outward surface normal is in [0, 90) degrees,
            which means the ray originated INSIDE the object, and so the true surface normal will
            also point inward, and so we set `unit_surface_normal` equal to the negative of
            `outward_unit_surface_normal`. We also set `hit_from_outside` to false. */
            unit_surface_normal = -outward_unit_surface_normal;  /* Flip outward surface normal */
            hit_from_outside = false;
        } else {
            /* Then the angle between the ray and the outward surface normal is in [90, 180)
            degrees, which means the ray originated OUTSIDE the object, and so the true surface
            normal will point outward, and so we just set `unit_surface_normal` to
            `outward_unit_surface_normal` itself. We also set `hit_from_outside` to true. */
            unit_surface_normal = outward_unit_surface_normal;
            hit_from_outside = true;
        }
    }
};

/* Overload `operator<<` for `hit_info` to allow printing it to output streams */
std::ostream& operator<< (std::ostream &os, const hit_info &info) {
    os << "hit_info {\n\thit_time: " << info.hit_time << "\n\thit_point: " << info.hit_point
       << "\n\tsurface_normal: " << info.unit_surface_normal << "\n\thit_from_outside: "
       << info.hit_from_outside << "\n}\n";
    return os;
}

struct Hittable {
    
    /* Check if the object is hit by a ray in the time range specified by `ray_times`.
    = 0 causes `hit_by` to be a pure virtual function, and so `Hittable` is an abstract
    class, which means it itself cannot be instantiated (good, it's only an interface). */
    virtual std::optional<hit_info> hit_by(const Ray3D &ray, const Interval &ray_times) const = 0;

    /* Returns the AABB (Axis-Aligned Bounding Box) for this `Hittable` object */
    virtual AABB get_aabb() const = 0;

    /* Prints this `Hittable` object to the `std::ostream` specified by `os`. */
    virtual void print_to(std::ostream &os) const = 0;

    virtual ~Hittable() = default;
};

/* Overload `operator<<` for `Hittable` to allow printing it to output streams */
std::ostream& operator<< (std::ostream &os, const Hittable &object) {
    object.print_to(os);  /* Call the overriden `print_to` function for the type of `object` */
    return os;
}

#endif