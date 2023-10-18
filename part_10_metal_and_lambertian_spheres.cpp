#include "image.h"
#include "ray3d.h"
#include "camera.h"
#include "sphere.h"
#include "scene.h"
#include "material.h"

Scene world;

auto ray_color(const Ray3D &ray) {

    auto info = world.hit_by(ray);
    if (!info) {  /* This goes first so this function's return type can be deduced as `RGB` */
        /* If this ray doesn't intersect any object in the scene, then its color is determined
        by the background. Here, the background is a blue-to-white gradient depending on the ray's
        y-coordinate; bluer for lesser y-coordinates and whiter for larger y-coordinates (so bluer
        at the top and whiter at the bottom). */
        return lerp(RGB::from_mag(1, 1, 1), RGB::from_mag(0.5, 0.7, 1),
                    0.5 * ray.dir.unit_vector().y + 0.5);
    } else {
        if (auto scattered = info.material->scatter(ray, info); scattered) {
            return scattered.attenuation * ray_color(scattered.ray);
        }
        return RGB::from_mag(0, 0, 0);
    }
}

int main()
{
    constexpr auto image_width = 400;
    constexpr auto aspect_ratio = 16. / 9.;  /* Very common aspect ratio */

    auto material_for_ground = std::make_shared<Lambertian>(RGB::from_mag(0.8, 0.8, 0));
    auto material_for_center = std::make_shared<Lambertian>(RGB::from_mag(0.7, 0.3, 0.3));
    /* Test fuzziness factors of 0.25 and 0.75 on the left and right spheres, respectively */
    auto material_for_left   = std::make_shared<Metal>(RGB::from_mag(0.8, 0.8, 0.8), 0.25);
    auto material_for_right  = std::make_shared<Metal>(RGB::from_mag(0.8, 0.6, 0.2), 0.75);

    world.add(make_shared<Sphere>(Point3D( 0.0, -100.5, -1.0), 100.0, material_for_ground));
    world.add(make_shared<Sphere>(Point3D( 0.0,    0.0, -1.0),   0.5, material_for_center));
    world.add(make_shared<Sphere>(Point3D(-1.0,    0.0, -1.0),   0.5, material_for_left));
    world.add(make_shared<Sphere>(Point3D( 1.0,    0.0, -1.0),   0.5, material_for_right));

    Camera().set_image_by_width_and_aspect_ratio(image_width, aspect_ratio)
            .set_viewport_height(2)
            .set_samples_per_pixel(50)  /* Now with anti-aliasing */
            .render_to(ray_color, "part_10_metal_and_lambertian_spheres_with_fuzzy_reflection.ppm");

    return 0;
}