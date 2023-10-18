#include "image.h"
#include "ray3d.h"
#include "camera.h"
#include "sphere.h"

/* Create a blue-to-white gradient depending on the ray's y-coordinate; bluer for lesser
y-coordinates and whiter for larger y-coordinates (so bluer at the top and whiter at the bottom). */
auto ray_color(const Ray3D &ray) {

    /* Add a completely green sphere with center (0, 0, -1) (one unit directly in front of the
    camera center), and radius 0.5. */
    if (Sphere(Point3D{0, 0, -1}, 0.5).hit_by(ray)) {
        return RGB::from_rgb(0, 255, 0);  /* Green */
    }

    /* For the B-component, we scale the y-component of the unit vector of the direction of `ray`
    to from 0 to 1 by observing that the y-component of any unit vector must be in [-1, 1], and
    so multiplying by 0.5 then adding 0.5 suffices. */
    return lerp(RGB::from_mag(1, 1, 1), RGB::from_mag(0.5, 0.7, 1),
                0.5 * ray.dir.unit_vector().y + 0.5);
}

int main()
{
    constexpr auto image_width = 400;
    constexpr auto aspect_ratio = 16. / 9.;  /* Very common aspect ratio */

    Camera().set_image_by_width_and_aspect_ratio(image_width, aspect_ratio)
            .set_viewport_height(2)
            .render_to(ray_color, "part_5_camera_sphere_test.ppm");

    /* Previous code for reference
    auto img = Image::with_width_and_aspect_ratio(image_width, aspect_ratio);
    auto vp = Camera::from_height_and_image(2, img);  // Choose arbitrary viewport height; 2 here

    render(img, vp);

    img.send_as_ppm("part_5_camera_sphere_test.ppm");
    */

    return 0;
}