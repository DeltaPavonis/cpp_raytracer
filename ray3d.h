#ifndef RAY3D_H
#define RAY3D_H

#include <iostream>
#include <cmath>
#include "vec3d.h"

struct Ray3D {
    Point3D origin = {0, 0, 0};
    Vec3D dir = {0, 0, 0};

    /* Evaluate the ray at time `t` */
    auto operator() (double t) const {return origin + t * dir;}

    /* Constructs a ray with origin `origin_` and direction specified by `dir_` */
    // Ray3D(const Point3D &origin_, const Vec3D &dir_) : origin{origin_}, dir{dir_} {}

    /* Default constructor results in a ray with origin and direction (0, 0, 0) */
    // Ray3D() : origin{0, 0, 0}, dir{0, 0, 0} {}
};

/* Overload `operator<<` to allow printing `Ray3D`s to output streams */
std::ostream& operator<< (std::ostream &os, const Ray3D &ray) {
    os << "Ray3D {origin: " << ray.origin << ", dir: " << ray.dir << "}";
    return os;
}

#endif