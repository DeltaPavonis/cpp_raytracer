#ifndef AABB_H
#define AABB_H

#include "math/ray3d.h"
#include "math/interval.h"

class AABB {
    /* Observe that an n-dimensional axis-aligned bounding box is equivalent to the intersection of
    n axis-aligned intervals. Thus, a 3D axis-aligned bounding box is equivalent to the intersection
    of three intervals for the x/y/z-coordinates, which are stored in`x`, `y`, and `z` respectively.
    In raytracing, these axis-aligned intervals are called "slabs", and so this strategy of
    representing n-dimensional AABB's with n slabs is called the "slab method". */
    Interval x, y, z;

    AABB(const Interval &x_, const Interval &y_, const Interval &z_) : x{x_}, y{y_}, z{z_} {}

public:

    /* Returns the `Interval` corresponding to the axis specified by `axis`. Specifically,
    returns the x-, y-, and z- interval when `axis` is 0, 1, or 2, respectively. */
    auto& operator[] (size_t axis) {return (axis == 0 ? x : (axis == 1 ? y : z));}
    /* Returns the `Interval` corresponding to the axis specified by `axis`. Specifically,
    returns the x-, y-, and z- interval when `axis` is 0, 1, or 2, respectively. */
    const auto& operator[] (size_t axis) const {return (axis == 0 ? x : (axis == 1 ? y : z));}

    /* Returns the centroid of this `AABB`. */
    auto centroid() const {return Point3D{x.midpoint(), y.midpoint(), z.midpoint()};}
    /* Returns the surface area of this `AABB`. */
    auto surface_area() const {
        return 2 * (x.size() * x.size() + y.size() * y.size() + z.size() * z.size());
    }
    /* Returns the volume of this `AABB`. */
    auto volume() const {
        return x.size() * y.size() * z.size();
    }

    /* Returns `true` if the ray `ray` intersects this `AABB` in the time range specified by
    `ray_times`. */
    bool is_hit_by(const Ray3D &ray, Interval ray_times) const {
        /* Use the slab method to check ray-AABB intersections. Specifically, remember that a
        3-dimensional AABB is equivalent to the intersection of an x-interval, a y-interval, and
        a z-interval. This means that a point (x0, y0, z0) is inside the AABB if and only if
        x0 is in (x.min, x.max), y0 is in (y.min, y.max), and z0 is in (z.min, z.max). Now,
        checking if a ray intersects with this AABB is equivalent to checking if there exists some
        `t` such that ray(t) = ray.origin + t * ray.dir has each coordinate in its corresponding
        interval. This, in turn, holds iff the intersection of the three intervals of times where
        the x-, y-, and z-coordinates of ray(t) are each in their corresponding intervals is
        non-empty. Note that we know that the set of times where each coordinate is in its
        corresponding interval forms a single interval, because a ray is defined by a linear
        equation in its coordinates (we will show how exactly the compute the time ranges for each
        coordinate below). Finally, it remains to check if the intersection of the three time
        intervals intersects with the interval `ray_times`; if it does, then the original ray `ray`
        does hit this `AABB` in the Interval `ray_times`, and otherwise, it does not. */

        /* Note: We just copy-paste the code for each coordinate instead of using a loop because
            - I don't represent `Vec3D`s as arrays of `double`s. Instead, I represent them as three
            separate `double` variables. I'm afraid that writing a `double operator[]` would
            cause branching overhead, but I would need one to be able to simplify this code by using
            a `for`-loop.
            - This is a function that will take up a large portion of the runtime, because it will
            eventually be the main method used to determine ray-scene collisions. Thus, we'd want to
            have any loop we wrote inlined by the compiler (especially since it only has 3
            iterations). This approach pretty much inlines it automatically for us, so we know we
            don't have to worry about it.
        */

        /* Compute the time range where the x-coordinate of `ray` is in the interval `x`.
        Because ray(t).x = ray.origin.x + t * ray.dir.x by the definition of a ray, we know that
        ray(t).x is inside the Interval `x` when t is between (x.min - ray.origin.x) / ray.dir.x
        and (x.max - ray.origin.x) / ray.dir.x. */
        auto inverse_ray_dir = 1 / ray.dir.x;  /* Only compute this once */
        auto t0 = (x.min - ray.origin.x) * inverse_ray_dir;
        auto t1 = (x.max - ray.origin.x) * inverse_ray_dir;
        /* Make sure that `t0` < `t1`. This is necessary, because while we already know that
        `x.max` > `x.min`, whether or not `t0` is less than `t1` still depends on the sign of
        `inverse_ray_dir`. Specifically, when `inverse_ray_dir` is negative, then `t0` will
        actually be greater than `t1`, so we `std::swap` them then. */
        if (inverse_ray_dir < 0) {std::swap(t0, t1);}
        /* Update `ray_times` to equal its intersection with (t0, t1). This is a concise way of
        finding the intersection of the time intervals we currently have calculated with the
        original desired time interval `ray_times`. Note that the reason we pass `ray_times` by
        copy is because we modify `ray_times` when finding its intersection with the time intervals
        we compute fo each coordinate. */
        if (t0 > ray_times.min) {ray_times.min = t0;}
        if (t1 < ray_times.max) {ray_times.max = t1;}
        /* If `ray_times` is the empty interval, then there is no time t in `ray_times` where
        the x-coordinate is in the range specified by `x`. Thus, the ray `ray` does not intersect
        this `AABB` in the original time interval `ray_times`, and so we immediately return
        `false`. */
        if (ray_times.max <= ray_times.min) {return false;}

        /* Compute time range where the y-coordinate of `ray` is in the interval `y`. Identical
        process as for the x-coordinate above. */
        inverse_ray_dir = 1 / ray.dir.y;
        t0 = (y.min - ray.origin.y) * inverse_ray_dir;
        t1 = (y.max - ray.origin.y) * inverse_ray_dir;
        if (inverse_ray_dir < 0) {std::swap(t0, t1);}
        if (t0 > ray_times.min) {ray_times.min = t0;}
        if (t1 < ray_times.max) {ray_times.max = t1;}
        /* If `ray_times` is the empty interval, then there is no time t in `ray_times` where
        the x-coordinate is in the range specified by `x` and the y-coordinate is in the range
        specified by `y`. Thus, the ray `ray` does not intersect this `AABB` in the original time
        interval `ray_times`, and so we immediately return `false`. */
        if (ray_times.max <= ray_times.min) {return false;}

        /* Compute time range where the z-coordinate of `ray` is in the interval `z`. Identical
        process as for the x- and y-coordinates above. */
        inverse_ray_dir = 1 / ray.dir.z;
        t0 = (z.min - ray.origin.z) * inverse_ray_dir;
        t1 = (z.max - ray.origin.z) * inverse_ray_dir;
        if (inverse_ray_dir < 0) {std::swap(t0, t1);}
        if (t0 > ray_times.min) {ray_times.min = t0;}
        if (t1 < ray_times.max) {ray_times.max = t1;}
        /* If `ray_times` is the empty interval, then there is no time t in `ray_times` where
        the x-coordinate is in the range specified by `x`, the y-coordinate is in the range
        specified by `y`, and the z-coordinate is in the range specified by `z`. Thus, the ray
        `ray` does not intersect this `AABB` in the original time interval `ray_times`, and so
        we immediately return `false`. */
        if (ray_times.max <= ray_times.min) {return false;}

        /* Otherwise, if `ray_times` is non-empty, that means there does exist some time `t`
        in the original time range `ray_times` such that `ray(t)` is inside this `AABB`. Thus,
        we return `true` here. */
        return true;
    }

    /* Returns `true` if the ray `ray` intersects this `AABB` in the time range specified by
    `ray_times`. This function also takes the precomputed values `inverse_ray_direction`
    (the vector with components equal to the reciprocals of the given ray `ray`'s direction),
    and `direction_is_negative` (where `direction_is_negative[i]` = whether or not `ray.dir[i]`
    is negative). */
    bool is_hit_by_optimized(
        const Ray3D &ray, const Interval &ray_times,
        const Vec3D &inverse_ray_direction,
        const std::array<bool, 3> &direction_is_negative
    ) const {

        /* Precomputing `direction_is_negative` lets us directly compute the minimum and
        maximum time of intersection for each axis, rather than having to compute both
        times of intersection and then comparing them to see which one is the minimum. */
        auto x_tmin = (x[ direction_is_negative[0]] - ray.origin.x) * inverse_ray_direction.x;
        auto x_tmax = (x[!direction_is_negative[0]] - ray.origin.x) * inverse_ray_direction.x;
        auto y_tmin = (y[ direction_is_negative[1]] - ray.origin.y) * inverse_ray_direction.y;
        auto y_tmax = (y[!direction_is_negative[1]] - ray.origin.y) * inverse_ray_direction.y;

        /* If the x- and y- time intervals are disjoint, then the ray does not intersect
        this AABB, and so we return false. */
        if (x_tmin > y_tmax || y_tmin > x_tmax) {
            return false;
        }

        /* Otherwise, we merge the intervals [x_tmin, x_tmax] and [y_tmin, y_tmax] into
        `x_tmin` and `x_tmax`. */
        if (y_tmin > x_tmin) {x_tmin = y_tmin;}
        if (y_tmax < x_tmax) {x_tmax = y_tmax;}

        /* Now, calculate the time interval for the ray's intersection along the z-axis */
        auto z_tmin = (z[ direction_is_negative[2]] - ray.origin.z) * inverse_ray_direction.z;
        auto z_tmax = (z[!direction_is_negative[2]] - ray.origin.z) * inverse_ray_direction.z;
        if (x_tmin > z_tmax || z_tmin > x_tmax)  {
            return false; 
        }

        /* Now, merge the merged x- and y- time intervals with the z-axis time interval.
        After this, [x_tmin, x_tmax] gives the overall time interval for which the ray
        `ray` intersects with this `AABB`. */
        if (z_tmin > x_tmin) {x_tmin = z_tmin; }
        if (z_tmax < x_tmax) {x_tmax = z_tmax;}

        /* If the overall time interval of the ray `ray`'s intersection with this AABB
        intersects with the given time interval `ray_times`, then there is a intersection.
        Otherwise, there isn't. */
        return (x_tmin < ray_times.max) && (x_tmax > ray_times.min);
    }

    /* Updates (possibly expands) this `AABB` to also bound the `AABB` `other`. */
    auto& merge_with(const AABB &other) {
        /* Just combine the x-, y-, and z- intervals with those from `other` */
        x.merge_with(other.x);
        y.merge_with(other.y);
        z.merge_with(other.z);
        return *this;
    }

    /* Updates (possibly expands) this `AABB` to also bound the `Point3D` `p`. */
    auto& merge_with(const Point3D &p) {
        x.merge_with(p.x);
        y.merge_with(p.y);
        z.merge_with(p.z);
        return *this;
    }

    /* Setters */

    /* Pads all axes with length less than `min_axis_length` to have length exactly
    `min_axis_length` (well, "exactly" as far as floating-point arithmetic can give you). */
    auto& ensure_min_axis_length(double min_axis_length) {
        if (x.size() < min_axis_length) {x.pad_with((min_axis_length - x.size()) / 2);}
        if (y.size() < min_axis_length) {y.pad_with((min_axis_length - y.size()) / 2);}
        if (z.size() < min_axis_length) {z.pad_with((min_axis_length - z.size()) / 2);}
        return *this;
    }

    /* Overload `operator<<` to allow printing `AABB`s to output streams */
    friend std::ostream& operator<< (std::ostream &os, const AABB &aabb);

    /* --- CONSTRUCTORS --- */

    /* The default constructor constructs an empty `AABB`; that is, the `AABB` where all intervals
    are the empty interval `Interval::empty()`. Note: Prefer using the named constructor
    `AABB::empty()` instead; its functionality is equivalent, and it is more readable. This
    default constructor exists merely to allow other classes which have an `AABB` as a member
    to themselves be default constructible without having to specify a default member initializer
    for fields of type `AABB`. */
    AABB() : AABB(Interval::empty(), Interval::empty(), Interval::empty()) {}

    /* --- NAMED CONSTRUCTORS --- */

    /* Returns an empty `AABB`; specifically, the `AABB` where all "slabs" are set to the empty
    interval `Interval::empty()`. This is equivalent to the default constructor of `AABB`, but it is
    recommended to use `AABB::empty()` over `AABB::AABB()` for improved readability. */
    static AABB empty() {
        return AABB();
    }

    /* Constructs an AABB (Axis-Aligned Bounding Box) consisting of all points with x-coordinate
    in the interval `x_`, y-coordinate in the interval `y_`, and z-coordinate in the range `z_`.
    In other words, this creates the AABB from the three "slabs" (axis intervals) `x_`, `y_`, and
    `z_`. */
    static AABB from_axis_intervals(const Interval &x_, const Interval &y_, const Interval &z_) {
        return AABB(x_, y_, z_);
    }

    /* Constructs the minimum-volume AABB (Axis-Aligned Bounding Box) containing all the points
    specified in `points`. */
    static AABB from_points(std::initializer_list<Point3D> points) {
        auto ret = AABB::empty();
        for (const auto &p : points) {
            ret.merge_with(p);
        }
        return ret;
    }

    /* Returns the minimum-volume `AABB` that contains both of the `AABB`s `a` and `b`.
    That is, this returns the `AABB` that would result if `a` and `b` were combined into a single
    `AABB`. */
    static AABB merge(const AABB &a, const AABB &b) {
        return AABB(Interval::merge(a.x, b.x), Interval::merge(a.y, b.y),
                    Interval::merge(a.z, b.z));
    }
};

/* Overload `operator<<` to allow printing `AABB`s to output streams */
std::ostream& operator<< (std::ostream &os, const AABB &aabb) {
    os << "AABB {x: " << aabb.x << ", y: " << aabb.y << ", z: " << aabb.z << "} ";
    return os;
}

#endif