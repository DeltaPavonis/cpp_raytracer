#ifndef MATERIAL_H
#define MATERIAL_H

#include "rgb.h"
#include "ray3d.h"
#include "hittable.h"
#include "rand_util.h"

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
    virtual std::optional<scatter_info> scatter(const Ray3D &ray, const hit_info &info) const = 0;

    virtual ~Material() = default;
};

class Lambertian : public Material {
    /* `intrinsic_color` = The color intrinsic to this Lambertian reflector */
    RGB intrinsic_color;

public:

    std::optional<scatter_info> scatter(const Ray3D &ray, const hit_info &info) const override {

        /* Lambertian reflectance states that an incident ray will be reflected (scattered) at an
        angle of phi off the surface normal with probabiliyt cos(phi). This is equivalent to saying
        that the endpoint of the scattered ray is an uniformly random point on the unit sphere
        centered at the endpoint of the unit surface normal at the original ray's intersection
        point with the surface. */
        auto scattered_direction = info.unit_surface_normal + Vec3D::random_unit_vector();

        /* If the random unit vector happens to equal `-info.surface_normal`, then
        `scattered_direction` will be the zero vector, which will lead to numerical errors.
        Thus, when `scattered_direction` is nearly a zero vector (here, we say that is the
        case if all its components have magnitude less than `1e-8`), we just set it to
        the direction of the surface normal at the intersection point. */
        if (scattered_direction.near_zero()) {
            scattered_direction = info.unit_surface_normal;
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

    std::optional<scatter_info> scatter(const Ray3D &ray, const hit_info &info) const override {

        /* Unlike Lambertian reflectors, metals display specular reflection; the incident
        light ray is reflected about the surface normal. */
        auto reflected_unit_dir = reflected(ray.dir.unit_vector(), info.unit_surface_normal);
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

    /* Constructs a metal (specular) reflector with intrinsic color `intrinsic_color_`
    and "fuzz factor" `fuzz`. A fuzz factor of 0 indicates perfect specular reflection,
    and the maximum fuzz factor is 1.  */
    Metal(const RGB &intrinsic_color_, double fuzz = 0) : intrinsic_color{intrinsic_color_},
                                                          fuzz_factor{std::min(fuzz, 1.)} {}
};

class Dielectric : public Material {

    /* `refr_index` = The refractive index of this dielectric surface */
    double refr_index;

    /* Calculate the reflectance (the specular reflection coefficient) of this
    dielectric material using Schlick's approximation.
    `cos_theta` = cos(theta), where theta is the angle between the incident light
    ray and the surface normal on the side of the initial medium.
    `refractive_index_ratio` = the ratio of the refractive index of the initial medium
    to the refractive index of the final medium. */
    static auto reflectance(double cos_theta, double refractive_index_ratio) {
        /* Use Schlick's approximation to calculate the reflectance.
        See https://en.wikipedia.org/wiki/Schlick%27s_approximation. */
        auto r0 = (1 - refractive_index_ratio) / (1 + refractive_index_ratio);
        r0 *= r0;
        return r0 + (1 - r0) * pow(1 - cos_theta, 5);
    }

public:

    std::optional<scatter_info> scatter(const Ray3D &ray, const hit_info &info) const override {

        /* If the ray hits this dielectric from the outside, then it is transitioning from
        air (refractive index assumed to be 1) to the current object, so the ratio is 1 / ri.
        Otherwise, if the ray hits this dielectric from the inside, then it is transitioning from
        the current object to air, so the ratio is the reciprocal ri / 1. */
        auto refractive_index_ratio = (info.hit_from_outside ? 1. / refr_index : refr_index / 1.);
        auto unit_dir = ray.dir.unit_vector();

        /* Calculate direction of resulting ray. Try refraction, then if no refraction is
        possible under Snell's Law the ray must be reflected. Also, even if refraction
        is possible, the material will have a reflectance depending on the current angle;
        we reflect the light ray with probability equal to the reflectance. */
        auto dir = refracted(unit_dir, info.unit_surface_normal, refractive_index_ratio); 
        if (std::isinf(dir.x)) {
            /* If `std::isinf(dir.x)`, then this dielectric material cannot refract the light
            ray, and so it must reflect it. */
            dir = reflected(unit_dir, info.unit_surface_normal);
        } else {
            /* Reflect with probability equal to the reflectance of this dielectric object */
            auto cos_theta = std::fmin(dot(-unit_dir, info.unit_surface_normal), 1.);
            if (rand_double() < reflectance(cos_theta, refractive_index_ratio)) {
                dir = reflected(unit_dir, info.unit_surface_normal);
            }
        }

        /* Attenuance is just (1, 1, 1); color is not lost when moving through (or reflecting
        off of) a Dielectric material apparently. The tutorial says "Attenuation is always 1"
        because a glass surface absorbs nothing. Makes sense, but does this hold for ALL
        dielectric surfaces? Maybe for tinted glass it doesn't, but our current implementation
        of Dielectric doesn't have a field for intrinsic color or tint. */
        return scatter_info(Ray3D(info.hit_point, dir), RGB::from_mag(1, 1, 1));
    }

    /* Constructs a dielectric reflector with refractive index `refractive_index`. */
    Dielectric(double refractive_index) : refr_index{refractive_index} {}
};

#endif