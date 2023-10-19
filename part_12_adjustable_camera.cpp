#include "image.h"
#include "ray3d.h"
#include "camera.h"
#include "sphere.h"
#include "scene.h"
#include "material.h"

Scene world;

auto ray_color(const Ray3D &ray, size_t depth_left) {

    /* If the ray has bounced the maximum number of times, then no light is collected
    from it. Thus, we return the RGB color (r: 0, g: 0, b: 0). */
    if (depth_left == 0) {
        return RGB::zero();
    }

    if (auto info = world.hit_by(ray, Interval::with_min(0.00001)); info) {

        /* If this ray hits an object in the scene, compute the scattered ray and the
        color attenuation, and return attenuation * ray_color(scattered ray).*/
        if (auto scattered = info->material->scatter(ray, *info); scattered) {
            return scattered->attenuation * ray_color(scattered->ray, depth_left - 1);
        }
        
        /* If the ray is not scattered (because it is absorbed, maybe? TODO: elaborate)
        then no light is gathered. */
        return RGB::zero();
    } else {
        /* If this ray doesn't intersect any object in the scene, then its color is determined
        by the background. Here, the background is a blue-to-white gradient depending on the ray's
        y-coordinate; bluer for lesser y-coordinates and whiter for larger y-coordinates (so bluer
        at the top and whiter at the bottom). */
        return lerp(RGB::from_mag(1, 1, 1), RGB::from_mag(0.5, 0.7, 1),
                    0.5 * ray.dir.unit_vector().y + 0.5);
    }
}

int main()
{
    constexpr auto image_width = 400;
    constexpr auto aspect_ratio = 16. / 9.;  /* Very common aspect ratio */

    auto material_for_ground = std::make_shared<Lambertian>(RGB::from_mag(0.8, 0.8, 0));
    auto material_for_center = std::make_shared<Lambertian>(RGB::from_mag(0.1, 0.2, 0.5));
    auto material_for_left   = std::make_shared<Dielectric>(1.5);  /* Test dielectric surface */
    auto material_for_right  = std::make_shared<Metal>(RGB::from_mag(0.8, 0.6, 0.2), 0.05);

    world.add(make_shared<Sphere>(Point3D( 0.0, -100.5, -1.0), 100.0, material_for_ground));
    world.add(make_shared<Sphere>(Point3D( 0.0,    0.0, -1.0),   0.5, material_for_center));
    world.add(make_shared<Sphere>(Point3D(-1.0,    0.0, -1.0),   0.5, material_for_left));
    world.add(make_shared<Sphere>(Point3D(-1.0,    0.0, -1.0),  -0.4, material_for_left));
    world.add(make_shared<Sphere>(Point3D( 1.0,    0.0, -1.0),   0.5, material_for_right));

    Camera().set_image_by_width_and_aspect_ratio(image_width, aspect_ratio)
            .set_vertical_fov(20)  /* Now set FOV instead of setting viewport dimensions directly */
            .set_camera_center(Point3D{-2, 2, 1})
            .set_camera_lookat(Point3D{0, 0, -1})
            .set_camera_up_direction(Point3D{0, 1, 0})
            .set_samples_per_pixel(100)  /* Anti-aliasing */
            .set_max_depth(20)  /* Now with maximum number of light ray bounces */
            .render_to(ray_color, "part_12_adjustable_camera.ppm");

    return 0;
}