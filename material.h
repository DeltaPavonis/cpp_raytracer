#ifndef MATERIAL_H
#define MATERIAL_H

#include "rgb.h"
#include "ray3d.h"
#include "hittable.h"

struct scatter_info {

    /* `ray` = the scattered ray */
    Ray3D ray;
    /* `attenuation` = The color by which `ray_color(ray)` will be multiplied by (multiplying colors
    means element-wise multiplication; apparently this is how objects' intrinsic colors are
    accounted for). TODO: Elaborate on why this color is named "attenuation". */
    RGB attenuation;

    /* Converts to `true` if it represents a successful ray scattering, and false if not.
    TODO: Determine when a ray can be UNsuccessfully scattered and decide on how that will be
    indicated */
    operator bool() {return true;}

    scatter_info(const Ray3D &ray_, const RGB &attenuation_)
        : ray{ray_}, attenuation{attenuation_} {}
};

struct Material {
    virtual scatter_info scatter(const Ray3D &ray, const hit_info &info) const = 0;

    virtual ~Material() = default;
};

class Lambertian : public Material {
    /* `intrinsic_color` = The color intrinsic to this Lambertian reflector */
    RGB intrinsic_color;

public:

    scatter_info scatter(const Ray3D &ray, const hit_info &info) const override {

        /* Lambertian reflectance states that an incident ray will be reflected (scattered) at an
        angle of phi off the surface normal with probabiliyt cos(phi). This is equivalent to saying
        that the endpoint of the scattered ray is an uniformly random point on the unit sphere
        centered at the endpoint of the unit surface normal at the original ray's intersection
        point with the surface. */
        auto scattered_direction = info.surface_normal + Vec3D::random_unit_vector();

        /* If the random unit vector happens to equal `-info.surface_normal`, then
        `scattered_direction` will be the zero vector, which will lead to numerical errors.
        Thus, when `scattered_direction` is nearly a zero vector (here, we say that is the
        case if all its components have magnitude less than `1e-8`), we just set it to
        the direction of the surface normal at the intersection point. */
        if (scattered_direction.near_zero()) {
            scattered_direction = info.surface_normal;
        }

        /* The scattered ray goes from the original ray's hit point to the randomly-chosen point
        on the unit sphere centered at the unit surface normal's endpoint, and the attenuation
        is the same as the intrinsic color. */
        return scatter_info(Ray3D(info.hit_point, scattered_direction), intrinsic_color);
    }

    /* Constructs a Lambertian (diffuse) reflector with intrinsic color `intrinsic_color_`. */
    Lambertian(const RGB &intrinsic_color_) : intrinsic_color{intrinsic_color_} {}
};

class Metal : public Material {
    /* `intrinsic_color` = The color intrinsic to this metal */
    RGB intrinsic_color;
    double fuzz_factor;

public:

    scatter_info scatter(const Ray3D &ray, const hit_info &info) const override {

        /* Unlike Lambertian reflectors, metals display specular reflection; the incident
        light ray is reflected about the surface normal. */
        auto reflected_unit_dir = reflected(ray.dir.unit_vector(), info.surface_normal);
        /* To simulate fuzzy reflection off metal surfaces, the end point is chosen randomly
        off the sphere with radius `fuzz_factor` centered at the endpoint of `reflected_unit_dir`.
        Thus, `fuzz_factor` = 0 results in perfect specular (perfectly mirror-like) reflection.
        Note that this is why `reflected_unit_dir` is made an unit vector (and so it is why
        `ray.dir` is normalized before being passed to `reflected`; it's to ensure every direction
        of reflection has the same probability, just like in Lambertian reflectors). */
        auto scattered_dir = reflected_unit_dir + fuzz_factor * Vec3D::random_unit_vector();

        /* The scattered ray goes from the original ray's hit point to the randomly-chosen point
        on the unit sphere centered at the unit surface normal's endpoint, and the attenuation
        is the same as the intrinsic color. */
        return scatter_info(Ray3D(info.hit_point, scattered_dir), intrinsic_color);
    }

    /* Constructs a metal (specular) reflector with intrinsic color `intrinsic_color_`. */
    Metal(const RGB &intrinsic_color_, double fuzz = 0) : intrinsic_color{intrinsic_color_},
                                                          fuzz_factor{std::min(fuzz, 1.)} {}
};

#endif