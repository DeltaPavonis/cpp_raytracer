#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <iterator>
#include <memory>
#include <span>
#include "hittable.h"

/* `Scene` is an abstraction over a list of `Hittable` objects in 3D space.
As its name suggests, it is designed to be used to represent `Scene`s of objects. */
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
    /* To allow for range-`for` loops */
    auto begin() {return objects.begin();}
    auto begin() const {return objects.cbegin();}
    auto end() {return objects.end();}
    auto end() const {return objects.cend();}

    /* Add a object, stored within a `std::shared_ptr`, to the list of objects */
    void add(std::shared_ptr<Hittable> object) {
        /* Pass `shared_ptr` by copy, so this `Scene` will keep the object alive
        as long as it itself has not been destroyed */

        /* Update `aabb` with the new object `object` */
        aabb.merge_with(object->get_aabb());  /* This must happen BEFORE `object` is moved! */
        /* Use `std::move` when inserting the `std::shared_ptr` into the `std::vector`
        of `Hittable`s. Passing the `std::shared_ptr<Hittable>` by copy and then
        moving it follows R34 of the C++ Core Guidelines (see https://tinyurl.com/bdfjfrub). */
        objects.push_back(std::move(object));
    }

    /* Adds all objects in the Scene `scene` to this `Scene`. */
    void add(const Scene &scene) {
        /* The reason we don't just add a single `std::shared_ptr` to `Scene` to
        `objects` is because (a) `scene` isn't a `std::shared_ptr` already, and
        (b) it makes the debugging output more confusing to have `Scene`s being
        printed as part of other `Scene`s' output. */
        for (const auto &object : scene) {
            add(object);
        }
    }

    /* Return the `hit_info`, if any, from the earliest object hit by the 3D ray `ray`. */
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
    AABB get_aabb() const override {
        return aabb;
    }
    
    /* Returns the list of primitive components of all `Hittable` objects in this `Scene`,
    which contributes to more efficient and complete `BVH`s. See the comments for this
    function in `Hittable` for a more detailed explanation. */
    std::vector<std::shared_ptr<Hittable>> get_primitive_components() const override {

        /* Simply collect all the primitive components for each object in turn into `ret`,
        using the `get_primitive_components()` function that all `Hittable`s must have. */
        std::vector<std::shared_ptr<Hittable>> ret;
        for (const auto &obj : objects) {
            if (auto obj_components = obj->get_primitive_components(); !obj_components.empty()) {
                /* If `get_primitive_components()` returns a list of `Hittable` components for
                `obj`, then add those components into `ret`. */
                ret.insert(ret.end(), std::make_move_iterator(obj_components.begin()),
                                      std::make_move_iterator(obj_components.end()));
            } else {
                /* If `get_primitive_components()` returns an empty list, that means the current
                object `obj` is already an indivisible unit; it itself is a primitive, so we will
                just add itself to `ret`. */
                ret.push_back(obj);
            }
        }

        return ret;
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

    /* Constructs a `Scene` with objects given in `objs` */
    Scene(std::span<const std::shared_ptr<Hittable>> objects_) {
        for (const auto &i : objects_) {
            add(i);
        }
    }
};

#endif