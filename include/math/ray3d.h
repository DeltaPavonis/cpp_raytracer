#ifndef RAY3D_H
#define RAY3D_H

#include <iostream>
#include "math/vec3d.h"

/* `Ray3D` represents a ray in 3D space; that is, an origin (a 3D point) and
a direction (a 3D vector). */
struct Ray3D {
    /* `origin` = a 3D point representing the origin of the ray. (0, 0, 0) by default. */
    Point3D origin{0, 0, 0};
    /* `dir` = a 3D vector representing the direction of the ray. (0, 0, 0) by default. */
    Vec3D dir{0, 0, 0};

    /* Returns the point located at time `t` on this ray */
    auto operator() (double t) const {return origin + t * dir;}
};

/* Overload `operator<<` to allow printing `Ray3D`s to output streams */
std::ostream& operator<< (std::ostream &os, const Ray3D &ray) {
    os << "Ray3D {origin: " << ray.origin << ", dir: " << ray.dir << "}";
    return os;
}

#endif