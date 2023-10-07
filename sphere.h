#ifndef SPHERE_H
#define SPHERE_H

#include "vec3d.h"

/* A sphere in 3D space is represented by its center and its radius. */
struct Sphere {
    Point3D center;
    double radius;
};

#endif