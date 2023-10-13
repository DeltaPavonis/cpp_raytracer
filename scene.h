#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#include "hittable.h"

class Scene : public Hittable {
    std::vector<std::shared_ptr<Hittable>> objects{};

public:

    /* Add a object, stored within a `std::shared_ptr`, to the list of objects */
    void add(std::shared_ptr<Hittable> object) {
        /* Pass `shared_ptr` by copy, so this `Scene` will keep the object alive
        as long as it itself has not been destroyed */
        objects.push_back(object);
    }

    /* Return the `hit_info` from the closest object hit by the 3D ray `ray` */
    hit_info hit_by(const Ray3D &ray,
                    const Interval &ray_times = Interval::nonnegative()) const override {

        hit_info result;
        auto min_hit_time = ray_times.max;

        for (const auto &object : objects) {

            /* Update `result` and `min_hit_time` if the `ray` hits the current object
            in the time range before `min_hit_time` (the range (`t_min`, `min_hit_time`))*/
            if (auto curr = object->hit_by(ray, Interval(ray_times.min, min_hit_time)); curr) {
                result = curr;
                min_hit_time = curr.hit_time;
            }
        }
        return result;
    }
};

#endif