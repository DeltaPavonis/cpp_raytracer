#include "image.h"
#include "ray3d.h"
#include "viewport.h"

using namespace std;

auto ray_color(const Ray3D &ray) {

    /* Add a sphere with center (0, 0, -1) (one unit directly in front of the camera center),
    and radius 0.5, SHADED BY SURFACE NORMALS. */
    if (auto t = ray.hit_time(Sphere{.center = {0, 0, -1}, .radius = 0.5}); t >= 0) {

        /* The surface normal at the point P on a sphere S is simply the unit vector with the same
        direction as P - S.center (the vector from the center of the sphere to P). Note that P - S.center
        always has length equal to the radius of S, which is 0.5 here, so we could just multiply it by
        2 to make it an unit vector. */
        auto surf_normal = (ray(t) - Point3D{0, 0, -1}).unit_vector();

        /* Use a "common trick" for visualizing surface normals; scale the components of
        the normal to be from 0 to 1, then use the components as the RGB magnitudes.
        Here, because the radius of the sphere is 0.5, all components of `surf_normal`
        are from -0.5 to 0.5, so it suffices to add 0.5 to scale them to the range 0 to 1. */
        // return RGB::from_mag(surf_normal.x + 0.5, surf_normal.y + 0.5, surf_normal.z + 0.5);
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

void render(Image &img, const Viewport &vp) {

    /* Render each pixel */
    for (ProgressBar<size_t> row(0, img.height(), "Rendering image"); row(); ++row) {
        for (size_t col = 0; col < img.width(); ++col) {
            /* Send a ray from the camera through the center of the pixel */
            auto pixel_center = vp.pixel_center(row, col);
            Ray3D ray(vp.camera_center, pixel_center - vp.camera_center);

            /* Calculate color for this ray */
            img[row][col] = ray_color(ray);
        }
    }
}

int main()
{
    constexpr auto image_width = 400;
    constexpr auto aspect_ratio = 16. / 9.;  /* Very common aspect ratio */

    auto img = Image::with_width_and_aspect_ratio(image_width, aspect_ratio);
    auto vp = Viewport::from_height_and_image(2, img);  /* Choose arbitrary viewport height; 2 here */

    render(img, vp);

    img.send_as_ppm("part_6_sphere_surface_normal_shading.ppm");

    return 0;
}