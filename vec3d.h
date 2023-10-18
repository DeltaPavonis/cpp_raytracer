#ifndef VEC3D_H
#define VEC3D_H

#include <iostream>
#include <cmath>
#include "rand_util.h"

/* Vec3D represents a 3-dimensional vector, or equivalently, a point in 3D space. */
struct Vec3D {
    /* Three `double` components, 0 by default */
    double x = 0, y = 0, z = 0;

    /* Mathematical negation, +=, -=, *=, /= operators */
    auto operator-() const {return Vec3D{-x, -y, -z};}

    auto& operator+= (const Vec3D &rhs) {x += rhs.x; y += rhs.y; z += rhs.z; return *this;}
    auto& operator-= (const Vec3D &rhs) {x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;}
    auto& operator*= (double d) {x *= d; y *= d; z *= d; return *this;}
    auto& operator/= (double d) {x /= d; y /= d; z /= d; return *this;}

    /* Compute magnitude (length) and squared magnitude */
    auto mag() const {return std::sqrt(x * x + y * y + z * z);}
    auto mag_squared() const {return x * x + y * y + z * z;}

    /* Compute unit vector (forward declared) */
    Vec3D unit_vector() const;

    /* Check if all three components less than `epsilon` */
    auto near_zero(double epsilon = 1e-8) {
        return (fabs(x) < epsilon) && (fabs(y) < epsilon) && (fabs(z) < epsilon);
    }

    /* Generate random vector (by default, generates a vector with all components in [0, 1]) */
    static auto random(double min = 0, double max = 1) {
        return Vec3D{rand_double(min, max), rand_double(min, max), rand_double(min, max)};
    }

    /* Generate random unit vector */
    static auto random_unit_vector() {
        /* Generate a random vector in the unit sphere, then normalize it (turn it into
        an unit vector). This ensures that each unit vector has a theoretically equal
        probability of being generated. */
        Vec3D result;
        do {
            result = Vec3D::random(-1, 1);
        } while (!(result.mag_squared() < 1));

        /* Return the unit vector of the random vector in the unit sphere */
        return result.unit_vector();
    }

    /* Generate a random unit vector that is in the same hemisphere as `surface_normal`, 
    which is an OUTWARD surface normal at the same point on some surface as the random unit
    vector to be generated. Thus, this function returns an unit vector pointing out of
    a surface, from the same point as the given outward surface normal `surface_normal`.
    
    Forward-declared because the implementation requires `dot`, which has not been defined yet. */
    static auto random_unit_vector_on_hemisphere(const Vec3D &surface_normal);
};

/* Utility functions; vector addition/subtraction, multiplication/division by a scalar */
auto operator+ (const Vec3D &a, const Vec3D &b) {auto ret = a; ret += b; return ret;}
auto operator- (const Vec3D &a, const Vec3D &b) {auto ret = a; ret -= b; return ret;}
auto operator* (const Vec3D &a, double d) {auto ret = a; ret *= d; return ret;}
auto operator* (double d, const Vec3D &a) {return a * d;}
auto operator/ (const Vec3D &a, double d) {auto ret = a; ret /= d; return ret;}

/* Dot and cross product of two vectors*/
auto dot(const Vec3D &a, const Vec3D &b) {return a.x * b.x + a.y * b.y + a.z * b.z;}
auto cross(const Vec3D &a, const Vec3D &b) {
    return Vec3D{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

/* Overload operator<< to allow printing `Vec3D`s to output streams */
std::ostream& operator<< (std::ostream& os, const Vec3D &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

/* Unit vector is found by dividing the vector by its length/magnitude */
Vec3D Vec3D::unit_vector() const {return *this / this->mag();}

/* Now implement `random_unit_vector_on_hemisphere`, after `dot` has been defined. */

auto Vec3D::random_unit_vector_on_hemisphere(const Vec3D &surface_normal) {
    auto result = Vec3D::random_unit_vector();
    /* If the angle between `result` and the surface normal is less than 90 degrees,
    then `result` points in the correct hemisphere; that is, out of the surface. */
    return (dot(surface_normal, result) > 0 ? result : -result);
}

/* Returns the vector that is the vector `v` reflected across the unit normal vector
`unit_normal`. */
auto reflected(const Vec3D &v, const Vec3D &unit_normal) {
    /* Let the endpoint of `v` coincide with the origin of `unit_normal`. Then observe
    that the reflected vector is equivalent to `2*v - 2*b`, where `b` is the vector
    parallel to `unit_normal` with magnitude `|v|cos(theta)`, where `theta` is the angle
    between `v` and `unit_normal`. Now, observe that `|v|cos(theta)` = `|v||n|cos(theta)`
    since `n` is a unit vector, and so `b` = `unit_normal * dot(v, unit_normal)` by the
    definition of the dot product. `2*v - 2*b` is thus calculated as follows: */
    return v - 2 * dot(v, unit_normal) * unit_normal;
}

/* Returns the direction that results when the UNIT vector `unit_vec` is refracted; specifically,
`univ_vec` is some unit vector, `unit_normal` is an unit normal POINTING ON `unit_vec`'s SIDE
OF THE INTERFACE (the line of intersection), and `refractive_index_ratio` is the ratio of the
refractive index of the initial medium and the final medium.

For instance, if going from a refractive index of 1.5 to a refractive index of 2,
`refractive_index_ratio` should be set to 1.5 / 2 = 0.75. */
auto refracted(const Vec3D &unit_vec, const Vec3D &unit_normal, double refractive_index_ratio) {
    /* Use Snell's Law to compute the direction of the unit vector `v` after transitioning
    from a medium with refractive index x to a medium with refractive index y, where
    `refractive_index_ratio` = x / y. */

    /* Use `std::fmin` to bound `cos_theta` from above by 1., just in case a floating-point
    inaccuracy occurs which makes it a little greater than 1. */
    auto cos_theta = std::fmin(dot(-unit_vec, unit_normal), 1.);

    /* Individually compute the components of the resulting vector that are perpendicular
    and parallel to the surface normal on the side of the final medium, and then sum them
    to get the resulting vector. */
    auto v_out_perp = refractive_index_ratio * (unit_vec + cos_theta * unit_normal);
    auto v_out_para = -std::sqrt(std::fabs(1 - v_out_perp.mag_squared())) * unit_normal;
    return v_out_perp + v_out_para;
}

/* `Point3D` is a type alias for `Vec3D`, declared to improve clarity in the code */
using Point3D = Vec3D;

#endif