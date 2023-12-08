#include "rand_util.h"
#include "sphere.h"
#include "scene.h"
#include "material.h"
#include "camera.h"
#include "parallelogram.h"
#include "box.h"

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

    Camera()
        .set_image_by_width_and_aspect_ratio(1000, 1.)
        .set_samples_per_pixel(100)
        .set_max_depth(50)
        .set_vertical_fov(80)
        .set_camera_center(Point3D{0, 0, 9})
        .set_camera_direction_towards(Point3D{0, 0, 0})
        .set_camera_up_direction(Point3D{0, 1, 0})
        .turn_blur_off()
        .set_background(RGB::from_mag(0.7, 0.8, 1))  /* Light-blueish background */
        .render(world)
        .send_as_ppm("parallelograms_test.ppm");
}

/* Renders a Cornell Box. If `empty` is true, then no boxes will be present inside
the Cornell Box. */
void cornell_box_test(bool empty = false) {
    Scene world;

    auto red   = ms<Lambertian>(RGB::from_mag(.65, .05, .05));
    auto white = ms<Lambertian>(RGB::from_mag(.73, .73, .73));
    auto green = ms<Lambertian>(RGB::from_mag(.12, .45, .15));
    auto light = ms<DiffuseLight>(RGB::from_mag(1, 1, 1), 15);

    /* Walls and light of the standard Cornell Box */
    world.add(ms<Parallelogram>(Point3D(555,0,0), Vec3D(0,555,0), Vec3D(0,0,555), green));
    world.add(ms<Parallelogram>(Point3D(0,0,0), Vec3D(0,555,0), Vec3D(0,0,555), red));
    world.add(ms<Parallelogram>(Point3D(343,554,332), Vec3D(-130,0,0), Vec3D(0,0,-105), light));
    world.add(ms<Parallelogram>(Point3D(0,0,0), Vec3D(555,0,0), Vec3D(0,0,555), white));
    world.add(ms<Parallelogram>(Point3D(555,555,555), Vec3D(-555,0,0), Vec3D(0,0,-555), white));
    world.add(ms<Parallelogram>(Point3D(0,0,555), Vec3D(555,0,0), Vec3D(0,555,0), white));

    /* If `empty` is false, add the two `Box`es to the standard Cornell Box. The boxes are
    unrotated for now.*/
    if (!empty) {
        world.add(ms<Box>(Point3D(130, 0, 65), Point3D(295, 165, 230), white));
        world.add(ms<Box>(Point3D(265, 0, 295), Point3D(430, 330, 460), white));
    }

    /* 2160 width, 1. aspect ratio, 25000 samples per pixel, 50 max depth took 1:31:27 for empty
    box. With the two boxes (unrotated) added, took 3:20:55.
    
    With 1000 width and samples_per_pixel, and with boxes, this takes about 90 seconds on my
    machine. */
    Camera()
        .set_image_by_width_and_aspect_ratio(1000, 1.)
        .set_samples_per_pixel(1000)
        .set_max_depth(1000)
        .set_vertical_fov(40)
        .set_camera_center(Point3D{278, 278, -800})
        .set_camera_direction_towards(Point3D{278, 278, 0})
        .set_camera_up_direction(Point3D{0, 1, 0})
        .turn_blur_off()
        .set_background(RGB::from_mag(0))  /* Black background */
        .render(world)
        .send_as_ppm("cornell_box_1.ppm");
}

void raining_on_the_dance_floor() {
    rng_seeds.seed_with(5987634);

    Scene world;

    /* Add the dance floor */
    for (int x = -1000; x <= 1000; ++x) {
        for (int z = -1000; z <= 100; ++z) {
            world.add(ms<Parallelogram>(
                Point3D{x + 0.1, 0, z + 0.1}, Point3D{0.8, 0, 0}, Point3D{0, 0, 0.8},
                ms<DiffuseLight>(RGB::random(), rand_double(0.5, 2))
            ));
        }
    }

    /* Add raindrops (and the occasional metal ball for some reason) */
    for (size_t i = 0; i < 25000; ++i) {
        auto choose_material = rand_double();
        std::shared_ptr<Material> material = ms<Dielectric>(rand_double(1.25, 2.5));
        if (choose_material < 0.05) {material = ms<Metal>(RGB::random(), 0);}
        world.add(ms<Sphere>(Point3D{
            rand_double(-1000, 1000), rand_double(2, 40), rand_double(-1000, 50)},
            rand_double(0.25, 0.8),
        material));
    }

    /* Add some raindrops closer to the camera center */
    for (size_t i = 0; i < 50; ++i) {
        world.add(ms<Sphere>(
            Point3D{rand_double(-20, 20), rand_double(1, 8), rand_double(-50, 50)},
            rand_double(0.25, 0.5),
            ms<Dielectric>(1.5)
        ));
    }

    /* width, aspect ratio, samples, depth, fov: 3840, 1., 25000, 50, 40. Took 9 hours 31 min */
    Camera()
        .set_image_by_width_and_aspect_ratio(3840, 1.)
        .set_samples_per_pixel(25000)
        .set_max_depth(50)
        .set_vertical_fov(40)
        .set_camera_center(Point3D{0, 10, 50})
        .set_camera_direction_towards(Point3D{0, 0, 0})
        .set_camera_up_direction(Point3D{0, 1, 0})
        .turn_blur_off()
        .set_background(RGB::from_mag(0))  /* Black background */
        .render(world)
        .send_as_ppm("raining_on_the_dance_floor.ppm");
}

int main()
{
    switch(0) {
        case 0: parallelogram_test(); break;
        case 1: cornell_box_test(true); break;
        case 2: cornell_box_test(false); break;
        case 3: raining_on_the_dance_floor(); break;
        default: std::cout << "Nothing to do" << std::endl; break;
    }

    return 0;
}
