#ifndef RAY3D_H
#define RAY3D_H

#include <cmath>
#include "vec3d.h"

struct Ray3D {
    Point3D origin;
    Vec3D dir;

    /* Evaluate the ray at time `t` */
    auto operator() (double t) const {return origin + t * dir;}

    Ray3D(const Point3D &origin_, const Vec3D &dir_) : origin{origin_}, dir{dir_} {}
};

#endif