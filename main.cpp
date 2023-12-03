#include "rand_util.h"
#include "sphere.h"
#include "scene.h"
#include "material.h"
#include "camera.h"
#include "parallelogram.h"

/* Instead of `std::make_shared<T>`, I just need to type `ms<T>` now. */
template<typename T, typename... Args>
auto ms(Args&&... args) -> std::shared_ptr<T> {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

/* First Parallelogram test (corresponds to the image rendered at the end of Section 6
of The Next Week)*/
void parallelogram_test() {
    Scene world;

    auto left_red     = std::make_shared<Lambertian>(RGB::from_mag(1.0, 0.2, 0.2));
    auto back_green   = std::make_shared<Lambertian>(RGB::from_mag(0.2, 1.0, 0.2));
    auto right_blue   = std::make_shared<Lambertian>(RGB::from_mag(0.2, 0.2, 1.0));
    auto upper_orange = std::make_shared<Lambertian>(RGB::from_mag(1.0, 0.5, 0.0));
    auto lower_teal   = std::make_shared<Lambertian>(RGB::from_mag(0.2, 0.8, 0.8));

    // Quads
    world.add(ms<Parallelogram>(Point3D(-3,-2, 5), Vec3D(0, 0,-4), Vec3D(0, 4, 0), left_red));
    world.add(ms<Parallelogram>(Point3D(-2,-2, 0), Vec3D(4, 0, 0), Vec3D(0, 4, 0), back_green));
    world.add(ms<Parallelogram>(Point3D( 3,-2, 1), Vec3D(0, 0, 4), Vec3D(0, 4, 0), right_blue));
    world.add(ms<Parallelogram>(Point3D(-2, 3, 1), Vec3D(4, 0, 0), Vec3D(0, 0, 4), upper_orange));
    world.add(ms<Parallelogram>(Point3D(-2,-3, 5), Vec3D(4, 0, 0), Vec3D(0, 0,-4), lower_teal));

    Camera().set_image_by_width_and_aspect_ratio(1000, 1.)
            .set_samples_per_pixel(100)
            .set_max_depth(50)
            .set_vertical_fov(80)
            .set_camera_center(Point3D{0, 0, 9})
            .set_camera_direction_towards(Point3D{0, 0, 0})
            .set_camera_up_direction(Point3D{0, 1, 0})
            .turn_blur_off()
            .render(world)
            .send_as_ppm("image_temp.ppm");
}

/* Renders an empty Cornell Box. */
void cornell_box_test() {

    Scene world;

    auto red   = ms<Lambertian>(RGB::from_mag(.65, .05, .05));
    auto white = ms<Lambertian>(RGB::from_mag(.73, .73, .73));
    auto green = ms<Lambertian>(RGB::from_mag(.12, .45, .15));
    auto light = ms<DiffuseLight>(RGB::from_mag(1, 1, 1), 15);

    /* Walls and light of the standard Cornell Box */
    world.add(ms<Parallelogram>(Point3D(555,0,0), Vec3D(0,555,0), Vec3D(0,0,555), green));
    world.add(ms<Parallelogram>(Point3D(0,0,0), Vec3D(0,555,0), Vec3D(0,0,555), red));
    world.add(ms<Parallelogram>(Point3D(343, 554, 332), Vec3D(-130,0,0), Vec3D(0,0,-105), light));
    world.add(ms<Parallelogram>(Point3D(0,0,0), Vec3D(555,0,0), Vec3D(0,0,555), white));
    world.add(ms<Parallelogram>(Point3D(555,555,555), Vec3D(-555,0,0), Vec3D(0,0,-555), white));
    world.add(ms<Parallelogram>(Point3D(0,0,555), Vec3D(555,0,0), Vec3D(0,555,0), white));

    /* 2160 width, 1. aspect ratio, 25000 samples per pixel, 50 max depth took 1:31:27 */
    Camera().set_image_by_width_and_aspect_ratio(1000, 1.)
        .set_samples_per_pixel(500)
        .set_max_depth(50)
        .set_vertical_fov(40)
        .set_camera_center(Point3D{278, 278, -800})
        .set_camera_direction_towards(Point3D{278, 278, 0})
        .set_camera_up_direction(Point3D{0, 1, 0})
        .turn_blur_off()
        .set_background(RGB::from_mag(0))  /* Black background */
        .render(world)
        .send_as_ppm("empty_cornell_box.ppm");
}

int main()
{
    cornell_box_test();
    return 0;
}
