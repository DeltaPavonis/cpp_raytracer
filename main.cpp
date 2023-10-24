#include "rand_util.h"
#include "sphere.h"
#include "scene.h"
#include "material.h"
#include "camera.h"

int main()
{
    Scene world;

    /* The same code as from the tutorial for their final scene, with one change (see comment)
    on line 52) */

    /* Big gray sphere for the ground */
    auto ground_material = std::make_shared<Lambertian>(RGB::from_mag(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3D(0,-1000,0), 1000, ground_material));

    /* Generate small spheres */
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = rand_double();
            /* Changed 0.9 to 0.8 to minimize ball intersections (I think that's what it does) */
            Point3D center(a + 0.8*rand_double(), 0.2, b + 0.8*rand_double());

            if ((center - Point3D(4, 0.2, 0)).mag() > 0.9) {
                std::shared_ptr<Material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = RGB::random() * RGB::random();
                    sphere_material = std::make_shared<Lambertian>(albedo);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // Metal
                    auto albedo = RGB::random(0.5, 1);
                    auto fuzz = rand_double(0, 0.5);
                    sphere_material = std::make_shared<Metal>(albedo, fuzz);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = std::make_shared<Dielectric>(1.5);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    /* Three big spheres */
    auto material1 = std::make_shared<Dielectric>(1.5);
    world.add(std::make_shared<Sphere>(Point3D(0, 1, 0), 1.0, material1));

    auto material2 = std::make_shared<Lambertian>(RGB::from_mag(0.4, 0.2, 0.1));
    world.add(std::make_shared<Sphere>(Point3D(-4, 1, 0), 1.0, material2));

    auto material3 = std::make_shared<Metal>(RGB::from_mag(0.7, 0.6, 0.5), 0.0);
    world.add(std::make_shared<Sphere>(Point3D(4, 1, 0), 1.0, material3));

    /* Render image */
    Camera().set_image_by_width_and_aspect_ratio(1080, 16. / 9.)  /* Should be height 1080, oops */
            .set_vertical_fov(20)  /* Smaller vertical FOV zooms in, also avoids shape stretching */
            .set_camera_center(Point3D{13, 2, 3})
            .set_camera_lookat(Point3D{0, 0, 0})
            .set_camera_up_direction(Vec3D{0, 1, 0})
            .set_defocus_angle(0.6)
            .set_focus_distance(10)
            .set_samples_per_pixel(500)  /* For a high-quality image */
            .set_max_depth(20)  /* More light bounces for higher quality */
            .render(world)
            .send_as_ppm("rtweekend_final_image.ppm");

    return 0;
}