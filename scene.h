#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#include "hittable.h"

class Scene : public Hittable {
    std::vector<std::shared_ptr<Hittable>> objects;
    AABB aabb;

public:

    /* Conversion operators to `std::vector<std::shared_ptr<Hittable>>`. */
    operator auto&() {return objects;}
    operator const auto&() const {return objects;}

    /* Implement convenience functions so that `Scene` can be used like a plain `std::vector`:
    `size()`, `clear`, `operator[]`, etc. */
    auto size() const {return objects.size();}
    void clear() {objects.clear();}
    auto& operator[] (size_t index) {return objects[index];}
    const auto& operator[] (size_t index) const {return objects[index];}

    /* Add a object, stored within a `std::shared_ptr`, to the list of objects */
    void add(std::shared_ptr<Hittable> object) {
        /* Pass `shared_ptr` by copy, so this `Scene` will keep the object alive
        as long as it itself has not been destroyed */

        /* Use `std::move` when inserting the `std::shared_ptr` into the `std::vector`
        of `Hittable`s. Passing the `std::shared_ptr<Hittable>` by copy and then
        moving it follows R34 of the C++ Core Guidelines (see https://tinyurl.com/bdfjfrub). */
        aabb.combine_with(object->get_aabb());  /* This must happen BEFORE `object` is moved! */
        objects.push_back(std::move(object));
    }

    /* Return the `hit_info`, if any, from the closest object hit by the 3D ray `ray`. */
    std::optional<hit_info> hit_by(const Ray3D &ray, const Interval &ray_times) const override {

        std::optional<hit_info> result;
        auto min_hit_time = ray_times.max;

        for (const auto &object : objects) {

            /* Update `result` and `min_hit_time` if the `ray` hits the current object
            in the time range before `min_hit_time` (the range (`t_min`, `min_hit_time`))*/
            if (auto curr = object->hit_by(ray, Interval(ray_times.min, min_hit_time)); curr) {
                result = curr;
                min_hit_time = curr->hit_time;
            }
        }

        return result;
    }

    /* Returns the AABB (Axis-Aligned Bounding Box) for this `Scene`. */
    AABB get_aabb() const {
        return aabb;
    }

    /* Prints every object in this `Scene` to the `std::ostream` specified by `os`. */
    void print_to(std::ostream &os) const override {
        os << "Scene with " << size() << " objects:\n";
        for (const auto &object : objects) {
            object->print_to(os);
            os << '\n';
        }
        os << std::flush;
    }

    Scene() = default;

    /* Constructs a `Scene` with objects given in `objects_` */
    Scene(const std::vector<std::shared_ptr<Hittable>> &objects_) : objects{objects_} {}
};

#endif