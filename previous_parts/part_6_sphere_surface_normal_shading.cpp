#include "image.h"
#include "ray3d.h"
#include "camera.h"
#include "sphere.h"

using namespace std;

auto ray_color(const Ray3D &ray) {

    /* Add a sphere with center (0, 0, -1) (one unit directly in front of the camera center),
    and radius 0.5, SHADED BY SURFACE NORMALS. */
    if (auto t = Sphere(Point3D{0, 0, -1}, 0.5).hit_by(ray).hit_time; !std::isinf(t)) {

        /* The surface normal at the point P on a sphere S is simply the unit vector with the same
        direction as P - S.center (the vector from the center of the sphere to P). Note that P - S.center
        always has length equal to the radius of S, which is 0.5 here, so we could just multiply it by
        2 to make it an unit vector. */
        auto surf_normal = (ray(t) - Point3D{0, 0, -1}).unit_vector();

        /* Use a "common trick" for visualizing surface normals; scale the components of
        the normal to be from 0 to 1, then use the components as the RGB magnitudes.
        Here, because `surf_normal` is a unit vector, all components of `surf_normal`
        are from -1 to 1, so it suffices to add 1 then multiply by 0.5 to scale them to
        the range 0 to 1. */
        return RGB::from_mag(
            0.5 * (surf_normal.x + 1),
            0.5 * (surf_normal.y + 1),
            0.5 * (surf_normal.z + 1)
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

    Camera().set_image_by_width_and_aspect_ratio(image_width, aspect_ratio)
            .set_viewport_height(2)
            .render_to(ray_color, "part_6_sphere_surface_normal_shading.ppm");

    /* Previous code for reference
    auto img = ImagePPMStream::with_width_and_aspect_ratio(
        image_width,
        aspect_ratio,
        "part_6_sphere_surface_normal_shading.ppm"
    );
    auto vp = Camera::from_height_and_image(2, img);  // Choose arbitrary viewport height; 2 here

    render(img, vp);
    */

    return 0;
}