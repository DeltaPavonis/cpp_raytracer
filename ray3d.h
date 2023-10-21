#ifndef RAY3D_H
#define RAY3D_H

#include <iostream>
#include <cmath>
#include "vec3d.h"

/* `Ray3D` represents a ray in 3D space; that is, an origin (a 3D point) and
a direction (a 3D vector). */
struct Ray3D {
    Point3D origin = {0, 0, 0};
    Vec3D dir = {0, 0, 0};

    /* Evaluate the ray at time `t` */
    auto operator() (double t) const {return origin + t * dir;}
};

/* Overload `operator<<` to allow printing `Ray3D`s to output streams */
std::ostream& operator<< (std::ostream &os, const Ray3D &ray) {
    os << "Ray3D {origin: " << ray.origin << ", dir: " << ray.dir << "}";
    return os;
}

#endif