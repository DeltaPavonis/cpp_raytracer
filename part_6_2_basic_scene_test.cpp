#include "image.h"
#include "ray3d.h"
#include "camera.h"
#include "sphere.h"
#include "scene.h"

Scene world;

auto ray_color(const Ray3D &ray) {

    if (auto info = world.hit_by(ray); info) {

        /* Use a "common trick" for visualizing surface normals; scale the components of
        the normal to be from 0 to 1, then use the components as the RGB magnitudes. */
        auto surface_normal = info.surface_normal;
        return RGB::from_mag(
            0.5 * (surface_normal.x + 1),
            0.5 * (surface_normal.y + 1),
            0.5 * (surface_normal.z + 1)
        );
    }

    /* Otherwise, create a blue-to-white gradient depending on the ray's y-coordinate; bluer for
    lesser y-coordinates and whiter for larger y-coordinates (so bluer at the top and whiter at the
    bottom). */
    return lerp(RGB::from_mag(1, 1, 1), RGB::from_mag(0.5, 0.7, 1),
                0.5 * ray.dir.unit_vector().y + 0.5);
}

int main()
{
    constexpr auto image_width = 400;
    constexpr auto aspect_ratio = 16. / 9.;  /* Very common aspect ratio */

    world.add(std::make_shared<Sphere>(Point3D{0, 0, -1}, 0.5));
    world.add(std::make_shared<Sphere>(Point3D{0, -100.5, -1}, 100));
    // world.add(std::make_shared<Sphere>(Point3D{0, 0.15, -0.5}, 0.3));

    Camera().set_image_by_width_and_aspect_ratio(image_width, aspect_ratio)
            .set_viewport_height(2)
            .render_to(ray_color, "part_6_2_basic_scene_test.ppm");

    return 0;
}