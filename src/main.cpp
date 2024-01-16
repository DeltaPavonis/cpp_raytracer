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

void rtow_final_image() {
    Scene world;

    /* The same code as from the tutorial for their final scene */

    /* Big gray sphere for the ground */
    auto ground_material = std::make_shared<Lambertian>(RGB::from_mag(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3D(0,-1000,0), 1000, ground_material));

    /* Generate small spheres */
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = rand_double();
            Point3D center(a + 0.9*rand_double(), 0.2, b + 0.9*rand_double());

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
    Camera()
        .set_image_by_width_and_aspect_ratio(1200, 16. / 9.)
        .set_vertical_fov(20)  /* Smaller vertical FOV zooms in, also avoids shape stretching */
        .set_camera_center(Point3D{13, 2, 3})
        .set_camera_lookat(Point3D{0, 0, 0})
        .set_camera_up_direction(Vec3D{0, 1, 0})
        .set_defocus_angle(0.6)
        .set_focus_distance(10)
        .set_samples_per_pixel(500)  /* For a high-quality image */
        .set_max_depth(20)  /* More light bounces for higher quality */
        .set_background(RGB::from_mag(0.7, 0.8, 1))
        .render(world)
        .send_as_ppm("rtweekend_final_image.ppm");
}

void rtow_final_lights_with_tone_mapping() {
    /* Now with custom fixed seeds. */
    SeedSeqGenerator::get_instance().set_seed(2286021279);

    Scene world;

    /* The same code as from the tutorial for their final scene */

    /* Big gray sphere for the ground */
    auto ground_material = std::make_shared<Lambertian>(RGB::from_mag(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3D(0,-1000000,0), 1000000, ground_material));

    /* Generate small spheres */
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = rand_double();
            Point3D center(a + 0.9*rand_double(), 0.2, b + 0.9*rand_double());

            if ((center - Point3D(4, 0.2, 0)).mag() > 0.9) {
                std::shared_ptr<Material> sphere_material;

                if (choose_mat < 0.035) {
                    auto albedo = RGB::random();
                    sphere_material = std::make_shared<DiffuseLight>(albedo, rand_double(30, 100));
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = RGB::random() * RGB::random();
                    sphere_material = std::make_shared<Lambertian>(albedo);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.9) {
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
    
    /* Light in the sky (like a moon) */
    auto light_material = std::make_shared<DiffuseLight>(
        RGB::from_mag(0.380205, 0.680817, 0.385431),
        150
    );
    world.add(std::make_shared<Sphere>(Point3D(0, 2.5, 2.5), 0.2, light_material));

    Camera()
        .set_image_by_width_and_aspect_ratio(1080, 16. / 9.)
        .set_vertical_fov(25)  /* Smaller vertical FOV zooms in, also avoids shape stretching */
        .set_camera_center(Point3D{13, 2, 3})
        .set_camera_lookat(Point3D{0, 0, 0})
        .set_camera_up_direction(Vec3D{0, 1, 0})
        .set_defocus_angle(0.48)
        .set_focus_distance(10)
        .set_samples_per_pixel(2000)  /* For a high-quality image */
        .set_max_depth(20)  /* More light bounces for higher quality */
        .set_background(RGB::zero())
        .render(world)
        .send_as_ppm("rtow_final_lights_with_tone_mapping.ppm");
}

void millions_of_spheres() {
    Scene world;

    /* The same code as from the tutorial for their final scene, except now with a lot more spheres
    */

    /* Big gray sphere for the ground */
    auto ground_material = std::make_shared<Lambertian>(RGB::from_mag(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3D(0,-1000000,0), 1000000, ground_material));

    /* Generate small spheres */
    for (int a = -1001; a < 1001; a++) {
        for (int b = -1001; b < 51; b++) {
            auto choose_mat = rand_double();
            Point3D center(a + 0.9*rand_double(), 0.2, b + 0.9*rand_double());

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

    /* Render image (about 3hr 15min on Dell XPS 8960, 16 cores, 24 threads) */
    Camera()
        .set_image_by_width_and_aspect_ratio(2160, 16. / 9.)
        .set_vertical_fov(40)  /* Smaller vertical FOV zooms in, also avoids shape stretching */
        .set_camera_center(Point3D{0, 10, 50})
        .set_camera_lookat(Point3D{0, 0, 0})
        .set_camera_up_direction(Vec3D{0, 1, 0})
        .set_defocus_angle(0.1)
        .set_focus_distance(51)
        .set_samples_per_pixel(500)  /* For a high-quality image */
        .set_max_depth(50)  /* More light bounces for higher quality */
        .render(world)
        .send_as_ppm("millions_of_spheres.ppm");
}

void millions_of_spheres_with_lights() {
    SeedSeqGenerator::get_instance().set_seed(473654968);

    Scene world;

    /* Big gray sphere for the ground */
    auto ground_material = std::make_shared<Lambertian>(RGB::from_mag(0.5, 0.5, 0.5));
    world.add(std::make_shared<Sphere>(Point3D(0,-1000000,0), 1000000, ground_material));

    /* Generate small spheres */
    for (int a = -1001; a < 1001; a++) {
        for (int b = -1501; b < 51; b++) {
            auto choose_mat = rand_double();
            Point3D center(a + 0.9*rand_double(), 0.2, b + 0.9*rand_double());

            if ((center - Point3D(4, 0.2, 0)).mag() > 0.9) {
                std::shared_ptr<Material> sphere_material;

                if (choose_mat < 0.035) {
                    auto albedo = RGB::random();
                    sphere_material = std::make_shared<DiffuseLight>(albedo, rand_double(5, 15));
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = RGB::random() * RGB::random();
                    sphere_material = std::make_shared<Lambertian>(albedo);
                    world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.9) {
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
    
    /* Big light directly up from the origin */
    auto light_material = std::make_shared<DiffuseLight>(
        RGB::from_mag(0.380205, 0.680817, 0.385431),
        150
    );
    world.add(std::make_shared<Sphere>(Point3D(0, 12, 0), 3, light_material));
    
    Camera()
        .set_image_by_width_and_aspect_ratio(1080, 16. / 9.)
        .set_vertical_fov(40)  /* Smaller FOV means more zoomed in (also avoids stretching) */
        .set_camera_center(Point3D{0, 12.5, 50})
        .set_camera_lookat(Point3D{0, 0, 0})
        .set_camera_up_direction(Vec3D{0, 1, 0})
        .set_defocus_angle(0.1)
        .set_focus_distance(51)
        .set_samples_per_pixel(1000)  /* For a high-quality image */
        .set_max_depth(20)  /* More light bounces for higher quality */
        .set_background(RGB::zero())
        .render(world)
        .send_as_ppm("millions_of_spheres_with_lights.ppm");
}

/* First Parallelogram test (corresponds to the image rendered at the end of Section 6
of The Next Week). */
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
    Camera()
        .set_image_by_width_and_aspect_ratio(1000, 1.)
        .set_samples_per_pixel(10)
        .set_max_depth(1000)
        .set_vertical_fov(40)
        .set_camera_center(Point3D{278, 278, -800})
        .set_camera_direction_towards(Point3D{278, 278, 0})
        .set_camera_up_direction(Point3D{0, 1, 0})
        .turn_blur_off()
        .set_background(RGB::from_mag(0))  /* Black background */
        .render(world)
        .send_as_ppm((empty ? "empty_cornell_box.ppm" : "cornell_box_1.ppm"));
}

/* Renders an image of a scene consisting of a bunch of colored parallelogram lights stretching
away into the distance, above which are suspended numerous glass (and a few metal) "raindrops"
(spheres). */
void raining_on_the_dance_floor() {
    SeedSeqGenerator::get_instance().set_seed(5987634);

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

    Camera()
        .set_image_by_width_and_aspect_ratio(2160, 16. / 9.)
        .set_samples_per_pixel(50)
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

void christmas_tree_made_of_spheres() {
    SeedSeqGenerator::get_instance().set_seed(20231225);  /* Nice seed */

    Scene world;

    /* Add flat ground; color close to white to symbolize snow */
    auto ground = ms<Parallelogram>(
        Point3D{-1000000, 0, -1000000}, Vec3D{2000000, 0, 0}, Vec3D{0, 0, 2000000},
        ms<Lambertian>(RGB::from_mag(0.25))
    );
    world.add(ground);

    /* Moon toward the top right, above the ground */
    auto moon = ms<Sphere>(Point3D{20, 25, -25}, 2.5, ms<DiffuseLight>(RGB::from_mag(0.8), 500));
    world.add(moon);

    /* The Christmas tree will be a right cone with base on the xz-plane centered at the origin,
    and apex at the point (0, cone_apex_y, 0). In other words, `cone_apex_y` is the height of
    the Christmas tree. The radius of the cone's circular base will be equal to its height times
    `cone_radius_to_height_ratio`. */
    auto cone_apex_y = 20;
    auto cone_radius_to_height_ratio = 1. / 3.;

    /* The Christmas tree will be made out of metal ornaments, which will either be red, green,
    blue, or gray. */
    const std::array colors{
        RGB::from_rgb(156, 10, 72),
        RGB::from_rgb(66, 106, 33),
        RGB::from_rgb(41, 119, 133),
        /* Gray appears at 3x the probability of red, green, or blue */
        RGB::from_mag(0.5),
        RGB::from_mag(0.5),
        RGB::from_mag(0.5),
    };

    /* Generate metal ornaments (spheres) on the lateral surface of the cone that is the
    Christmas tree. Here, we generate 200 of them. */
    for (int i = 0; i < 200; ++i) {

        /* Add a random ornament (metal sphere) centered on the cone's lateral surface. */
        while (true) {
            /* Choose a y-coordinate (which is in [0, `cone_apex_y`]). */
            auto random_y = rand_double(0, cone_apex_y);
            /* Reduce excessive clustering of spheres at the top of the Christmas tree,
            which happens because the y-coordinates are uniformly random, and the available
            surface area for ornaments at the top of the Christmas tree (with larger
            y-coordinates) is smaller. */
            if (random_y > 17) {random_y = rand_double(0, cone_apex_y);}

            /* The first ornament will be at the apex of the Christmas tree cone */
            if (i == 0) {
                random_y = cone_apex_y;
            }

            /* Now, choose any point at the y-coordinate `random_y` on the cone. This
            will be the center of the current ornament. To choose a random point with
            this y-coordinate, we observe that the set of points on the cone's surface
            at a given y-coordinate form a circle, so it suffices to choose a random
            angle and then use sin/cos in conjunction with the radius of the cone at
            this height (which can be found using `cone_radius_to_height_ratio`). */
            auto radius_at_this_y = (20 - random_y) * cone_radius_to_height_ratio;
            auto angle = rand_double(0, 2 * std::numbers::pi);  /* Generate random angle */
            Point3D sphere_center{
                /* Use sin/cos on the x/z coordinates. Remember that the cone's base is on
                the xz-plane. */
                radius_at_this_y * std::sin(angle),
                random_y,
                radius_at_this_y * std::cos(angle),
            };

            /* Generate random sphere (ornament) radius */
            auto sphere_radius = rand_double(0.25, 0.45);

            /* If this ornament with the center and radius will come close (here, this means within
            0.1) of intersecting any of the previously-placed ornaments, then reject it and
            generate another random ornament in the next iteration of this `while(true)` loop. */
            if (std::any_of(world.begin(), world.end(), [&](const std::shared_ptr<Hittable> &obj) {
                auto s = dynamic_pointer_cast<Sphere>(obj);
                if (s == nullptr) {return false;}
                return (sphere_center - s->center).mag() <= sphere_radius + s->radius + 0.1; 
            })) {
                continue;
            }

            /* If this ornament does not come too close to intersecting with any of the previous
            ornaments we placed, then we will add it to the scene. First, choose its material.
            All ornaments will be metal, with color uniformly chosen from the `colors` array,
            and metal fuzz factor very low (randomly chosen in the range [0, 0.1]). */
            std::shared_ptr<Material> material;
            material = ms<Metal>(
                colors[rand_int(0, static_cast<int>(colors.size() - 1))],
                rand_double(0, 0.1)
            );
            /* The first ornament, which is located at the very top of the Christmas tree, will
            instead be a white `DiffuseLight` with a relative intensity of 10. This is similar to
            the star ornament placed on many Christmas trees I've seen. */
            if (i == 0) {
                material = ms<DiffuseLight>(RGB::from_mag(1), 10);
            }

            /* Add the current ornament to the scene, and we are done, so `break` afterwards. */
            world.add(ms<Sphere>(sphere_center, sphere_radius, material));

            break;
        }
    }

    /* Now, generate particles of snow in the scene. Each snow particle will be a very small
    white Lambertian sphere. */
    std::vector<std::shared_ptr<Sphere>> snow;
    auto snow_material = ms<Lambertian>(RGB::from_mag(1));  /* White Lambertian material */
    for (int i = 0; i < 4000; ++i) {  /* Generate 4000 snow particles */

        /* Generate a snow particle */
        while (true) {

            /* Choose random point as the center of the snow particle (which, remember, is a
            sphere), and a random radius. */
            Point3D snow_center{rand_double(-30, 30), rand_double(0, 30), rand_double(-50, 50)};
            /* If the snow particle is close to the camera center, then make its size smaller,
            otherwise the particles will look strangely large in the final result. */
            auto snow_radius = (snow_center.z > 35 ? 0.015 : (snow_center.z > 20 ? 0.03 : 0.05));

            /* If the current snow particle comes too close (within 0.1, here) of intersecting any
            of the ornaments, then reject it and generate another one in the next iteration of this
            `while(true)` loop.
            
            Note that we do not check if snow particles intersect each other. This is to avoid
            this loop taking `O((number of snow particles)^2) time, and because in general the
            snow particles are so small that (a) it's very unlikely for them to intersect given
            the sheer size of the scene compared to the size of the snow particles, and (b) they
            are so small that an intersection would probably be unnoticeable anyways. */
            if (std::any_of(world.begin(), world.end(), [&](const std::shared_ptr<Hittable> &obj) {
                auto s = dynamic_pointer_cast<Sphere>(obj);
                if (s == nullptr) {return false;}
                return (snow_center - s->center).mag() <= snow_radius + s->radius + 0.1; 
            })) {
                continue;
            }

            /* Add the snow particle to the list of snow particles (which is separate from the
            `Scene` because in the `if`-statement above, the range from `world.begin()` up to
            but not includin `world.end()` represents only the ornaments), and `break`. */
            snow.push_back(ms<Sphere>(snow_center, snow_radius, snow_material));

            break;
        }
    }

    /* Now, add all the snow particles to the scene. */
    for (const auto &i : snow) {world.add(i);}

    /* Render the image. */
    Camera()
        /* Specify the rendered image dimensions and the background color */
        .set_image_by_width_and_aspect_ratio(1080, 16. / 9.)
        .set_background(RGB::zero())
        /* Specify the camera itself and its vertical field-of-view */
        .set_camera_center(Point3D{0, 17.5, 50}) // 17.5
        .set_camera_direction_towards(Point3D{0, 10, 0})
        .set_camera_up_direction(Point3D{0, 1, 0})
        .set_vertical_fov(35)
        /* Specify the quality of the render (number of samples per pixel, and
        the maximum number of ray bounces) */
        .set_samples_per_pixel(10000)
        .set_max_depth(50)
        /* Render the image and send it to a PPM file */
        .render(world)
        .send_as_ppm("christmas_tree_of_spheres.ppm");
}

void bvh_pathological_test() {
    Scene world;

    /* This results in a BVH tree with depth 116. The idea is that if spheres increase
    exponentially in size, then the SAH will prefer to partition so that the largest
    sphere gets its own node. This means the depth would theoretically be linear,
    not logarithmic, in the number of primitives, which is what happens here. */
    int num_spheres = 135;
    double pos_scale = 10.7;
    double rad_scale = 17.3;
    for (int i = 0; i < num_spheres; ++i) {
        world.add(ms<Sphere>(
            Point3D{std::pow(pos_scale, i), 0, 0},
            std::pow(rad_scale, i),
            ms<Lambertian>(RGB::zero())
        ));
    }

    BVH bvh(world);

    /* Code I used to find the above values. Note that `BVH::build_bvh_tree` needs to be modified
    to have a `depth` parameter, and there also needs to be a `max_depth` global variable in
    `bvh.h`.

        std::atomic<int> maxmax_depth = 0;
        std::mutex mtx2, mtx3;
        std::tuple info{0, 0., 0.};
        #pragma omp parallel for schedule(dynamic)
        for (int spheres = 100; spheres <= 200; ++spheres) {
            {
                std::lock_guard guard(mtx2);
                std::cout << "Testing " << spheres << " spheres" << std::endl;
            }
            for (double pos = 10; pos <= 30; pos += 0.1) {
                for (double rad = 10; rad <= 20; rad += 0.1) {
                    Scene world;

                    for (int i = 0; i < spheres; ++i) {
                        world.add(ms<Sphere>(
                            Point3D{std::pow(pos, i), 0, 0},
                            std::pow(rad, i),
                            ms<Lambertian>(RGB::zero())
                        ));
                    }

                    max_depth = 0;
                    BVH bvh(world);

                    {
                        std::lock_guard guard(mtx3);
                        if (max_depth > maxmax_depth) {
                            maxmax_depth = max_depth;
                            info = {spheres, pos, rad};
                        }
                    }
                }
            }
        }

        std::cout << maxmax_depth << " <- result" << std::endl;

        std::cout << std::get<0>(info) << " " << std::get<1>(info) << " "
                  << std::get<2>(info) << std::endl;
        return 0;
    */
}

int main()
{
    switch(4) {
        case -10: bvh_pathological_test(); break;
        case -4: rtow_final_image(); break;
        case -3: rtow_final_lights_with_tone_mapping(); break;
        case -2: millions_of_spheres(); break;
        case -1: millions_of_spheres_with_lights(); break;
        case 0: parallelogram_test(); break;
        case 1: cornell_box_test(true); break;
        case 2: cornell_box_test(false); break;
        case 3: raining_on_the_dance_floor(); break;
        case 4: christmas_tree_made_of_spheres(); break;
        default: std::cout << "Nothing to do" << std::endl; break;
    }

    return 0;
}
