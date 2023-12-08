#ifndef BOX_SURFACE_H
#define BOX_SURFACE_H

#include <array>
#include "hittable.h"
#include "parallelogram.h"
#include "scene.h"

/* `Box` is an abstraction over a 3D box - a rectangular prism - in 3D space. */
class Box : public Hittable {
    /* `faces` holds the six faces of this `Box`. */
    Scene faces;
    /* `material`: The material for the surface of this `Box`. */
    std::shared_ptr<Material> material;

public:

    /* A `Box` is practically identical to a `Scene`, so the abstract functions inherited from
    `Hittable` can just be implemented in terms of the same functions for `Scene`. */

    /* Returns a `hit_info` representing the earliest (minimum hit-time) intersection of the ray
    `ray` with this `Box` in the time itnerval `ray_times`. If no such intersection exists, then
    an empty `std::optional<hit_info>` is returned. */
    std::optional<hit_info> hit_by(const Ray3D &ray, const Interval &ray_times) const override {
        /* Simply just delegate this to the `Scene`; this returns the earliest intersection of
        `ray` in the time interval `ray_times` with the six faces of the box, as desired. */
        return faces.hit_by(ray, ray_times);
    }

    /* Returns the primitive components of this `Box`; namely, its six `Parallelogram`
    faces, which contributes to the construction of more efficient and complete
    `BVH`s. See the comments in `Hittable::get_primitive_components()` for a more
    detailed explanation. */
    std::vector<std::shared_ptr<Hittable>> get_primitive_components() const override {
        return faces.get_primitive_components();
    }

    /* Prints this `Box` to the `std::ostream` specified by `os`. */
    void print_to(std::ostream &os) const override {
        os << "Box {faces: " << faces << "} " << std::flush;
    }

    /* Returns the `AABB` for this `Box`. */
    AABB get_aabb() const override {
        return faces.get_aabb();
    }

    /* --- CONSTRUCTORS --- */

    /* Constructs a `Box` with opposite vertices `vertex` and `opposite_vertex`, as well as
    material specified by `material_`. */
    Box(const Point3D &vertex, const Point3D &opposite_vertex,
        std::shared_ptr<Material> material_)
        : material{std::move(material_)}
    {

        /* Compute `min_coordinates` and `max_coordinates`, which respectively are the points with
        all minimal and all maximal x-, y-, and z- coordinates from the given points `vertex` and
        `opposite_vertex`. Observe that `min_coordinates` and `max_coordinates` are both vertices
        of the box; in fact, they are opposite vertices of the box. */
        Point3D min_coordinates, max_coordinates;
        for (int i = 0; i < 3; ++i) {
            min_coordinates[i] = std::fmin(vertex[i], opposite_vertex[i]);
            max_coordinates[i] = std::fmax(vertex[i], opposite_vertex[i]);
        }

        /* Precompute the edge vectors of the box; namely, the x-, y-, and z- displacement vectors
        from `min_coordinates` to `max_coordinates`. */
        auto side_x = Vec3D{max_coordinates.x - min_coordinates.x, 0, 0};
        auto side_y = Vec3D{0, max_coordinates.y - min_coordinates.y, 0};
        auto side_z = Vec3D{0, 0, max_coordinates.z - min_coordinates.z};

        /* A `Box` is represented by 6 parallelogram (specifcially, rectangular) faces.
        `min_coordinates` and `max_coordinates` eachlie on three of those faces. We add
        the 6 faces to `faces`. */
        faces.add(std::make_shared<Parallelogram>(min_coordinates, side_x, side_y, material));
        faces.add(std::make_shared<Parallelogram>(min_coordinates, side_x, side_z, material));
        faces.add(std::make_shared<Parallelogram>(min_coordinates, side_y, side_z, material));

        faces.add(std::make_shared<Parallelogram>(max_coordinates, -side_x, -side_y, material));
        faces.add(std::make_shared<Parallelogram>(max_coordinates, -side_x, -side_z, material));
        faces.add(std::make_shared<Parallelogram>(max_coordinates, -side_y, -side_z, material));
    }
};

#endif