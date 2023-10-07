#ifndef HITTABLE_AND_HIT_INFO_H
#define HITTABLE_AND_HIT_INFO_H

#include "ray3d.h"

struct hit_info {
    /*
    t: Time of intersection
    hit_point: Point of intersection
    surface_normal: Surface normal at the point of intersection
    
    t = -1 by default to signify no intersection */
    double t = -1;
    Point3D hit_point{};
    Vec3D surface_normal{};

    /* Converts to `true` if it represents the presence of an intersection, and false if not */
    operator bool() {return t >= 0.0;}
};

struct Hittable {
    virtual hit_info hit_by(const Ray3D &ray, double t_min, double t_max) const {return {};}
    virtual ~Hittable() = default;
};

#endif