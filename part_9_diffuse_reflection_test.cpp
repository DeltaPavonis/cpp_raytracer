#include "image.h"
#include "ray3d.h"
#include "camera.h"
#include "sphere.h"
#include "scene.h"

Scene world;

auto ray_color(const Ray3D &ray) {
    auto info = world.hit_by(ray, Interval::nonnegative());
    if (!info) {  /* This goes first so this function's return type can be deduced as `RGB` */
        /* If this ray doesn't intersect any object in the scene, then its color is determined
        by the background. Here, the background is a blue-to-white gradient depending on the ray's
        y-coordinate; bluer for lesser y-coordinates and whiter for larger y-coordinates (so bluer
        at the top and whiter at the bottom). */
        return lerp(RGB::from_mag(1, 1, 1), RGB::from_mag(0.5, 0.7, 1),
                    0.5 * ray.dir.unit_vector().y + 0.5);
    }

    /* Otherwise, the ray does hit an object. By true Lambertian reflectance, the ray will
    be reflected at an angle of phi off the surface normal with a probability of cos(phi).
    This is equivalent to saying that the endpoint of the ray is a random point on the
    unit sphere centered at the endpoint of the unit surface normal. */
    auto reflected_direction = info.surface_normal + Vec3D::random_unit_vector();
    /* Albedo = proportion of light reflected = 1 - proportion of light absorbed. The higher
    the albedo, the whiter the color; an albedo of 1 is pure white, and an albedo of 0 is black.
    Setting 0.5 gives a dark-grayish color (which is actually darker than expected; we will learn
    why this happens and how to fix this later). */
    constexpr double albedo = 0.5;

    /* A proportion of (1 - albedo) of the light is absorbed by the object hit, so the remaining
    proportion of light is albedo. */
    return albedo * ray_color(Ray3D(info.hit_point, reflected_direction));
}

int main()
{
    constexpr auto image_width = 400;
    constexpr auto aspect_ratio = 16. / 9.;  /* Very common aspect ratio */

    world.add(std::make_shared<Sphere>(Point3D{0, 0, -1}, 0.5));
    world.add(std::make_shared<Sphere>(Point3D{0, -100.5, -1}, 100));

    Camera().set_image_by_width_and_aspect_ratio(image_width, aspect_ratio)
            .set_viewport_height(2)
            .set_samples_per_pixel(50)  /* Now with anti-aliasing */
            .render_to(ray_color, "part_9_diffuse_reflection_test_true_lambertian_gamma_corrected.ppm");

    return 0;
}