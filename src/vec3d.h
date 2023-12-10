#ifndef VEC3D_H
#define VEC3D_H

#include <iostream>
#include <cmath>
#include <optional>
#include "rand_util.h"

/* Vec3D represents a 3-dimensional vector, or equivalently, a point in 3D space. */
struct Vec3D {
    /* Three `double` components, all 0 by default.
    
    Note: Later, we may no longer set the components to 0 by default with a member default
    value, so that `Vec3D` can be trivially constructible. */
    double x = 0, y = 0, z = 0;

    /* Returns the coordinate of this `Vec3D` on the axis specified by `axis`. Specifically,
    returns the `x`-, `y`-, and `z`- coordinate if `axis` is `0`, `1`, or `2`, respectively. */
    const auto& operator[] (size_t axis) const {return (axis == 0 ? x : (axis == 1 ? y : z));}
    auto& operator[] (size_t axis) {return (axis == 0 ? x : (axis == 1 ? y : z));}

    /* Mathematical negation, +=, -=, *=, /= operators */

    /* Element-wise negation for `Vec3D`s */
    auto operator-() const {return Vec3D{-x, -y, -z};}

    /* Element-wise addition assignment operator for `Vec3D`s */
    auto& operator+= (const Vec3D &rhs) {x += rhs.x; y += rhs.y; z += rhs.z; return *this;}
    /* Element-wise subtraction assignment operator for `Vec3D`s */
    auto& operator-= (const Vec3D &rhs) {x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;}
    /* Element-wise multiplication assignment operator for `Vec3D`s */
    auto& operator*= (double d) {x *= d; y *= d; z *= d; return *this;}
    /* Element-wise division assignment operator for `Vec3D`s */
    auto& operator/= (double d) {return *this *= (1 / d);}  /* Multiply by 1/d for less divisions */

    /* Compute magnitude (length) of this vector */
    auto mag() const {return std::sqrt(x * x + y * y + z * z);}
    /* Compute squared magnitude (squared length) of this vector */
    auto mag_squared() const {return x * x + y * y + z * z;}

    /* Compute unit vector (forward declared) since it requires operator/, which has
    not yet been defined */

    Vec3D unit_vector() const;

    /* Returns `true` if all three components have magnitude strictly less than `epsilon`. */
    auto near_zero(double epsilon = 1e-8) {
        return (std::fabs(x) < epsilon) && (std::fabs(y) < epsilon) && (std::fabs(z) < epsilon);
    }

    /* --- NAMED CONSTRUCTORS --- */

    /* Returns a vector with all components set to 0; the zero vector. */
    static auto zero() {
        return Vec3D{0, 0, 0};
    }

    /* Generate random vector with real components in the interval [min, max] ([0, 1] by default) */
    static auto random(double min = 0, double max = 1) {
        return Vec3D{rand_double(min, max), rand_double(min, max), rand_double(min, max)};
    }

    /* Generates an uniformly random unit vector */
    static auto random_unit_vector() {
        /* Generate a random vector in the unit sphere, then normalize it (turn it into
        an unit vector). This ensures that each unit vector has a theoretically equal
        probability of being generated, unlike simply returning Vec3D::random(-1, 1). */
        Vec3D result;
        do {
            result = Vec3D::random(-1, 1);
        } while (!(result.mag_squared() < 1));

        /* Return the unit vector of the random vector in the unit sphere */
        return result.unit_vector();
    }

    /* Generates an uniformly random vector in the unit disk; that is, generates a
    vector (a, b, 0) where a^2 + b^2 = 1. */
    static auto random_vector_in_unit_disk() {
        Vec3D result;
        do {
            result = Vec3D{rand_double(-1, 1), rand_double(-1, 1), 0};
        } while (!(result.mag_squared() < 1));
        return result;
    }

    /* Generate a random unit vector that is in the same hemisphere as `surface_normal`, 
    which is an OUTWARD surface normal at the same point on some surface as the random unit
    vector to be generated. Thus, this function returns an unit vector pointing out of
    a surface, from the same point as the given outward surface normal `surface_normal`.
    as not been defined yet. */
    static auto random_unit_vector_on_hemisphere(const Vec3D &surface_normal);
};

/* Math utility functions; vector addition/subtraction, multiplication/division by a scalar */

/* Performs element-wise addition on two `Vec3D`s */
auto operator+ (const Vec3D &a, const Vec3D &b) {auto ret = a; ret += b; return ret;}
/* Performs element-wise subtraction on two `Vec3D`s */
auto operator- (const Vec3D &a, const Vec3D &b) {auto ret = a; ret -= b; return ret;}
/* Performs element-wise multiplication by `d` on a `Vec3D` */
auto operator* (const Vec3D &a, double d) {auto ret = a; ret *= d; return ret;}
/* Performs element-wise multiplication by `d` on a `Vec3D` */
auto operator* (double d, const Vec3D &a) {return a * d;}
/* Performs element-wise division by `d` on a `Vec3D` */
auto operator/ (const Vec3D &a, double d) {auto ret = a; ret /= d; return ret;}

/* Dot and cross product of two vectors. Note that these are not static because
in OOP, static functions ought to not depend on the values of the member variables,
or on the existence of actual instances of the class which they are a static member of.
See https://softwareengineering.stackexchange.com/a/113034/426687. */

/* Computes the dot product of `a` and `b` */
auto dot(const Vec3D &a, const Vec3D &b) {return a.x * b.x + a.y * b.y + a.z * b.z;}
/* Computes the cross product of `a` and `b` */
auto cross(const Vec3D &a, const Vec3D &b) {
    return Vec3D{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

/* Overload operator<< to allow printing `Vec3D`s to output streams */
std::ostream& operator<< (std::ostream& os, const Vec3D &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

/* Returns the unit vector of this `Vec3D`. */
Vec3D Vec3D::unit_vector() const {
    /* Unit vector is found by dividing the vector by its length/magnitude */
    return *this / this->mag();
}

/* Now implement `random_unit_vector_on_hemisphere`, after `dot` has been defined. */

auto Vec3D::random_unit_vector_on_hemisphere(const Vec3D &surface_normal) {
    auto result = Vec3D::random_unit_vector();
    /* If the angle between `result` and the surface normal is less than 90 degrees,
    then `result` points in the correct hemisphere; that is, out of the surface. */
    return (dot(surface_normal, result) > 0 ? result : -result);
}

/* Returns the resulting direction when the direction vector `dir` is reflected across the unit
normal vector `unit_normal`, where the endpoint of `dir` is assumed to coincide with the origin
of `unit_normal`. The returned direction vector will thus have the same magnitude as `dir`. */
auto reflected(const Vec3D &dir, const Vec3D &unit_normal) {
    /* Observe that the reflected direction is equivalent to `dir + 2*b`, where `b` is the
    vector parallel to `unit_normal` with magnitude `|dir|cos(theta)` (where `theta` is
    the angle made between the incoming vector and `unit_normal`). Now, observe that
    `|dir|cos(theta) = |dir||unit_normal|cos(theta)` since `|unit_normal| = 1`. Finally,
    observe that `theta` is the supplementary angle of the actual angle `x` between
    `dir` and `unit_normal`, and so `cos(theta) = -cos(x)`. Thus, `b` is the vector
    parallel to `unit_normal` with magnitude `|dir||unit_normal|cos(theta)
    = -|dir||unit_normal|cos(x) = -dot(dir, unit_normal)`, and so the reflected direction
    (which is `dir - 2 * b`) is calculated as follows: */
    return dir - 2 * dot(dir, unit_normal) * unit_normal;
}

/*
@brief Returns the direction of the resulting ray when an incident ray with direction `unit_vec`
is refracted at the interface (boundary) between two isotropic (uniform) media with a given
refractive index ratio. If the ray cannot be refracted (under Snell's Law), then an empty
`std::optional<Vec3D>` object is returned.
@param `unit_dir`: The unit direction of the incoming ray. Assumed to be an unit vector.
@param `unit_normal`: An unit normal to the interface, pointing on the side of `unit_dir`.
@param `refractive_index_ratio`: The ratio of the refractive index of the medium the ray is
initially passing through, to the refractive index of the medium the ray is passing into.
For instance, if going from a medium with a refractive index of 1.5 to a medium with a refractive
index of 2, `refractive_index_ratio` should be set to 1.5 / 2 = 0.75. */
std::optional<Vec3D> refracted(const Vec3D &unit_dir, const Vec3D &unit_normal,
                               double refractive_index_ratio)
{
    /* Use Snell's Law to compute the direction of the unit vector `v` after transitioning
    from a medium with refractive index x to a medium with refractive index y, where
    `refractive_index_ratio` = x / y. */

    /* Use `std::fmin` to bound `cos_theta` from above by 1., just in case a floating-point
    inaccuracy occurs which makes it a little greater than 1.. This prevents the computation
    of `sin_theta` from taking the square root of a negative number. */
    auto cos_theta = std::fmin(dot(-unit_dir, unit_normal), 1.);
    auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);

    /* By Snell's law, n1sin(theta_1) = n2sin(theta_2), where n1 and n2 are the refractive
    indices of the initial and final mediums, theta_1 is the angle between the incident ray
    and the surface normal on the side of the initial medium, and theta_2 is the angle between
    the resulting ray and the surface normal on the side of the final medium. Clearly, a
    solution to theta_2 exists if and only if (n1/n2) * sin(theta_1) <= 1 (we already know this
    is >= 0 because n1/n2 >= 0 and 0 <= theta_1 <= 90 degrees). Thus, if
    `refractive_index_ratio * sin_theta` > 1, there is no solution and thus no refracted ray. */
    if (refractive_index_ratio * sin_theta > 1) {
        /* This ray cannot be refracted under Snell's Law as-is. We report this by returning an
        empty `std::optional<Vec3D>`, and allow the caller to decide what should be done instead. */
        return {};
    }

    /* Individually compute the components of the resulting vector that are perpendicular
    and parallel to the surface normal on the side of the final medium, and then sum them
    to get the resulting vector. */
    auto v_out_perp = refractive_index_ratio * (unit_dir + cos_theta * unit_normal);
    auto v_out_para = -std::sqrt(std::fabs(1 - v_out_perp.mag_squared())) * unit_normal;
    return v_out_perp + v_out_para;
}

/* `Point3D` is a type alias for `Vec3D`, declared to improve clarity in the code */
using Point3D = Vec3D;

#endif