#ifndef CAMERA_H
#define CAMERA_H

#include <numbers>
#include "image.h"
#include "ray3d.h"
#include "bvh.h"

/* The class `Camera` encapsulates the notion of a camera viewing a 3D scene from
a designated camera/eye point, located a certain length (called the focal length)
away from the "viewport" or "image plane": the virtual rectangle upon with the
3D scene is projected to form the final 2D image. */
class Camera {

    /* Width and height (in pixels) of the final rendered image. 1280 x 720 by default */
    size_t image_w = 1280, image_h = 720;
    /* Width and height of the viewport (aka image plane). Note that these are real-valued.
    Both are determined by either `vertical_fov` or `horizontal_fov` (as well as the focal
    length) during `init()`. */
    double viewport_w, viewport_h;
    /* The horizontal and vertical delta vectors from pixel to pixel in the viewport. */
    Vec3D pixel_delta_x, pixel_delta_y;
    /* `camera` stores the camera ray; the coordinates of the camera/eye point,
    and the direction in which the camera looks. By default, the camera center is
    at the origin, and the camera looks toward the direction of negative z-axis
    (as is usual for right-handed coordinates, which we use). */
    Ray3D camera{.origin = Point3D{0, 0, 0}, .dir = Vec3D{0, 0, -1}};
    /* `camera_lookat`, if specified, is the point towards which the camera always looks, no matter
    where the camera center is. If not specified, then `camera.dir` will be used for the camera's
    direction. */
    std::optional<Point3D> camera_lookat;
    /* `view_up_dir` allows the user to specify the "up" direction for the camera.
    Specifically, the "up" direction on the viewport is equal to the projection of
    `view_up_dir` onto the viewport. By default, the "up" direction is the same as the 
    positive y-axis. */
    Vec3D view_up_dir{0, 1, 0};
    /* `cam_basis_x/y/z` form an orthonormal basis of the camera and its orientation.
    `cam_basis_x` is an unit vector pointing to the right, `cam_basis_y` is an unit vector
    pointing up (so `cam_basis_x` and `cam_basis_y` form an orthonormal basis for the viewport),
    and `cam_basis_z` is an unit vector pointing behind the camera, orthogonal to the viewport
    (it points behind the camera and not in front due to the use of right-handed coordinates).
    Calculated in `init()`, depends on `camera` and `view_up_dir` (and themselves; `cam_basis_z`
    is calculated by taking the cross product of `cam_basis_x` and `cam_basis_y`). */
    Vec3D cam_basis_x, cam_basis_y, cam_basis_z;
    /* `focus_dist` is this Camera's focus distance; that is, the distance from the
    camera center to the plane of perfect focus. Definition-wise, it is different
    from the focal length, which is the distance from the camera center to the viewport.
    However, for our model, the focus distance will always equal the focal length; that is,
    we will always place our viewport on the plane of perfect focus.
    
    `focus_dist` may be explicitly set by the user through `set_focus_distance`. If not set,
    then the focus distance will default to the length of the camera's direction vector in
    `init()` (that is, `focus_dist` defaults to `camera.dir.mag()` if not explicitly set).
    The rationale behind this decision is explained in the comments for `init()`. */
    std::optional<double> focus_dist;
    /* `defocus_angle` is the angle of the cone with apex at the viewport's center and circular
    base equivalent to the defocus disk (which is centered at the camera center). A `defocus_angle`
    of 0 represents no blur, and that is the default. `defocus_angle` is stored in radians. May
    be explicitly set by the user through `set_defocus_angle`. */
    double defocus_angle = 0;
    /* `defocus_disk_x/y` are the vectors representing the horizontal and vertical radii vectors
    of the defocus disk. Both are determined by `focal_length`, `defocus_angle`,  and
    `cam_basis_x/y`. */
    Vec3D defocus_disk_x, defocus_disk_y;
    /* Coordinates of the top-left image pixel */
    Point3D pixel00_loc;
    /* Number of rays sampled per pixel, 100 by default */
    size_t samples_per_pixel = 100;
    /* Maximum number of light ray bounces into the scene, 10 by default */
    size_t max_depth = 10;
    /* Vertical and horizontal FOV (Field of View) of the camera, stored in radians.
    At any instant, only one of the vertical and horizontal FOV has a specified value
    (only one of `vertical_fov` and `horizontal_fov` will be a non-empty `std::optional<double>`);
    the other will be inferred from the FOV that is specified, as well as the image's aspect ratio,
    in `init()`. By default, the vertical FOV is 90 degrees, and so the horizontal FOV will be
    determined from the vertical FOVof 90 degrees  and the aspect ratio of the images in `init()`.
    */
    std::optional<double> vertical_fov{90}, horizontal_fov;

    /* Set the values of `viewport_w`, `viewport_h`, `pixel_delta_x`, `pixel_delta_y`,
    `upper_left_corner`, and `pixel00_loc` based on `image_w` and `image_h`. This function
    is called before every render. */
    void init() {

        /* Calculate the true aspect ratio of the image. Note that this may be different
        from the aspect ratio passed in calls to `set_image_by_xxxxx_and_aspect_ratio`,
        because `image_w` and `image_h` both must be integers. */
        auto aspect_ratio = static_cast<double>(image_w) / static_cast<double>(image_h);

        /* If the user has explicitly provided a lookat point for the camera, then update
        the camera's direction to be towards that lookat point from the current camera
        center. */
        if (camera_lookat) {
            camera.dir = *camera_lookat - camera.origin;
        }

        /* If `focus_dist` is not explicitly provided by the user, then set it equal to
        the length of the camera's direction vector. This way, if the user only specifies
        a camera center and a camera lookat point, the focus distance will be so that the
        lookat point is in perfect focus. */
        if (!focus_dist) {
            focus_dist = camera.dir.mag();
        }

        /* The focal length (the distance from the camera center to the viewport) will always
        be set equal to the focus distance (the distance from the camera center to the plane of
        perfect focus). That is, we will always place our viewport on the plane of perfect
        focus. */
        auto focal_length = *focus_dist;

        /* Set viewport_w and viewport_h based on `vertical_fov` or `horizontal_fov`,
        `focal_length`, and `aspect_ratio`. */
        if (vertical_fov) {
            viewport_h = 2 * focal_length * std::tan(*vertical_fov / 2);
            viewport_w = viewport_h * aspect_ratio;
        } else {
            viewport_w = 2 * focal_length * std::tan(*horizontal_fov / 2);
            viewport_h = viewport_w / aspect_ratio;
        }

        /* Calculate an orthonormal basis for the camera and its orientation. */
        cam_basis_z = -camera.dir.unit_vector();
        cam_basis_x = cross(view_up_dir, cam_basis_z).unit_vector();
        cam_basis_y = cross(cam_basis_z, cam_basis_x); /* Unit vector because cam_basis_x/z are */

        /* `x_vec` and `y_vec` are the vectors right and down across the viewport (starting from the
        top-left corner), respectively. We use right-handed coordinates, which means the y-axis goes
        up, the x-axis goes right, and the NEGATIVE z-axis goes into the image. As a result, the
        vector going down across the viewport has a negative y-component. */
        Vec3D x_vec = viewport_w * cam_basis_x, y_vec = -viewport_h * cam_basis_y;
        pixel_delta_x = x_vec / static_cast<double>(image_w);  /* Divide by pixels per row */
        pixel_delta_y = y_vec / static_cast<double>(image_h);  /* Divide by pixels per column */

        /* Find `upper_left_corner`, the coordinates of the upper left corner of the viewport.
        The upper left point of the viewport is found by starting at the camera, moving
        `focal_length` units towards the camera (so adding negative `focal_length` times
        `cam_basis_z` to `camera.origin`, due to the use of right-handed coordinates), then
        subtracting half of `x_vec` and `y_vec`. */
        auto upper_left_corner = camera.origin - focal_length * cam_basis_z - x_vec / 2 - y_vec / 2;

        /* Pixels are inset from the edges of the camera by half the pixel-to-pixel distance.
        This ensures that the viewpoint area is evenly divided into `pixel_delta_x` by
        `pixel_delta_y`-sized regions. */
        pixel00_loc = upper_left_corner + pixel_delta_x / 2 + pixel_delta_y / 2;

        /* Calculate the defocus disk radius vectors. To see how the calculation for
        `defocus_disk_radius` works, remember that the defocus disk is the base of the
        right cone with apex at the center of the viewport, apex angle equal to `defocus_angle`,
        and with its circular base centered at the camera center. */
        auto defocus_disk_radius = focal_length * std::tan(defocus_angle / 2);
        defocus_disk_x = defocus_disk_radius * cam_basis_x;
        defocus_disk_y = defocus_disk_radius * cam_basis_y;
    }

    /* Returns a random point in the camera's defocus disk. */
    auto random_point_in_defocus_disk() const {

        /* First, generate a random vector in the unit disk */
        auto vec = Vec3D::random_vector_in_unit_disk();

        /* Then, use the defocus disk basis vectors to turn `vec` into
        a random vector in the camera's defocus disk. */
        return camera.origin + vec.x * defocus_disk_x + vec.y * defocus_disk_y;
    }

    /*
    Returns a ray originating from the defocus disk centered at `camera.origin`, and through
    a random point in the square region centered at the pixel in row `row` and column `col`.
    
    Note that the region is square because it is a rectangle with width |`pixel_delta_x`| and
    height |`pixel_delta_y`|, and we have `pixel_delta_x` = `x_vec` / `image_w` =
    `viewport_w` / `image_w`, and `pixel_delta_y` = `y_vec` / `image_h` = `viewport_h` / `image_h`.
    Then, `viewport_w` / `image_w` = `viewport_h` / `image_h` because `viewport_w` / `viewport_h`
    = `image_w` / `image_h` (as the aspect ratio of the viewport is equal to the aspect ratio of
    the image).
    
    Then why do we need both `pixel_delta_x` and `pixel_delta_y`? I think it's for clarity, or,
    perhaps, due to possible floating-point errors that cause slight differences in `pixel_delta_x`
    and `pixel_delta_y`. */
    auto random_ray_through_pixel(size_t row, size_t col) const {

        /* The ray originates from a random point in the camera's defocus disk */
        auto ray_origin = (defocus_angle <= 0 ? camera.origin : random_point_in_defocus_disk());

        /* Find the center of the pixel */
        auto pixel_center = pixel00_loc + static_cast<double>(row) * pixel_delta_y
                          + static_cast<double>(col) * pixel_delta_x;
        
        /* Find a random point in the square region centered at `pixel_center`. The region
        has width `pixel_delta_x` and height `pixel_delta_y`, so a random point in this
        region is found by adding `pixel_delta_x` and `pixel_delta_y` each multiplied by
        a random real number in the range [-0.5, 0.5]. */
        auto pixel_sample = pixel_center + rand_double(-0.5, 0.5) * pixel_delta_x
                          + rand_double(-0.5, 0.5) * pixel_delta_y;
        return Ray3D(ray_origin, pixel_sample - ray_origin);
    }

    /* Computes and returns the color of the light ray `ray` shot into the scene `world`.
    If `ray` has bounced more than `depth_left` times, returns `RGB::zero()`. */
    auto ray_color(const Ray3D &ray, size_t depth_left, const Hittable &world) {

        /* If the ray has bounced the maximum number of times, then no light is collected
        from it. Thus, we return the RGB color (r: 0, g: 0, b: 0). */
        if (depth_left == 0) {
            return RGB::zero();
        }

        /* Interval::with_min(0.00001) is the book's fix for shadow acne; ignore
        ray collisions that happen at very small times. */
        if (auto info = world.hit_by(ray, Interval::with_min(0.00001)); info) {

            /* If this ray hits an object in the scene, compute the scattered ray and the
            color attenuation, and return attenuation * ray_color(scattered ray).*/
            if (auto scattered = info->material->scatter(ray, *info); scattered) {
                return scattered->attenuation * ray_color(scattered->ray, depth_left - 1, world);
            }
            
            /* If the ray is not scattered (because it is absorbed, maybe? TODO: elaborate)
            then no light is gathered. */
            return RGB::zero();
        } else {
            /* If this ray doesn't intersect any object in the scene, then its color is determined
            by the background. Here, the background is a blue-to-white gradient depending on the
            ray's y-coordinate; bluer for lesser y-coordinates and whiter for larger y-coordinates
            (so bluer at the top and whiter at the bottom). */
            return lerp(RGB::from_mag(1, 1, 1), RGB::from_mag(0.5, 0.7, 1),
                        0.5 * ray.dir.unit_vector().y + 0.5);
        }
    }

public:

    /* Renders the Scene `world` to an `Image` and returns that image.
    Will render in parallel (using OpenMP for now) if available. */
    auto render(const Hittable &world) {
        init();

        /* Calculate and store the color of each pixel */
        auto img = Image::with_dimensions(image_w, image_h);
        ProgressBar pb(image_h, "Rendering image");
        
        /* Now use dynamic thread scheduling instead of static thread scheduling, with a block size
        of the maximum of `image_h` / 1024 and 1. */
        const size_t thread_chunk_size = std::max(image_h >> 10, size_t{1});
        #pragma omp parallel for schedule(dynamic, thread_chunk_size)
        for (size_t row = 0; row < image_h; ++row) {
            for (size_t col = 0; col < image_w; ++col) {

                /* Shoot `samples_per_pixel` random rays through the current pixel.
                The average of the resulting colors will be the color for this pixel. */
                auto pixel_color = RGB::zero();
                for (size_t sample = 0; sample < samples_per_pixel; ++sample) {
                    auto ray = random_ray_through_pixel(row, col);
                    pixel_color += ray_color(ray, max_depth, world);
                }
                pixel_color /= static_cast<double>(samples_per_pixel);

                img[row][col] = pixel_color;
             }

             pb.update();
        }
        return img;
    }

    /* When rendering a `Scene`, `Camera::render()` build a `BVH` over the `Scene` and render
    using that `BVH` to improve performance. */
    auto render(const Scene &world) {
        return render(BVH(world));
    }

    /* Setters. Each returns a mutable reference to this object to create a functional interface */
    
    /* Sets the camera center to the point `p`. This is where the camera is placed. */
    auto& set_camera_center(const Point3D &p) {camera.origin = p; return *this;}
    /* Sets the camera direction to the vector `dir`.
    
    Note: If previously unset, then this Camera's focus distance will automatically be set to
    the length of the direction vector `dir`. This way, objects placed at the end of the
    camera's direction vector (or at other points that are the same distance away from the
    camera center) will appear in perfect focus, and will linearly appear blurrier the further
    away they are from that distance. */
    auto& set_camera_direction(const Vec3D &dir) {camera.dir = dir; return *this;}
    /* Sets the direction of the camera to be the direction from the camera center
    to the point `p`. Note: this is NOT equivalent as `set_camera_lookat`; the latter
    instructs the camera to always face toward the provided lookat point, whereas this
    simply redirects the camera's current direction to face towards the provided point `p`.
        
    Note: If previously unset, then this Camera's focus distance will automatically be set to
    the distance from the camera center to the point `p`. This way, objects placed at the point
    `p` (and other points that are the same distance away from the camera center) will appear
    in perfect focus, and will linearly appear blurrier the further away they are from that
    distance. */
    auto& set_camera_direction_towards(const Point3D &p) {
        camera.dir = p - camera.origin;
        return *this;
    }
    /* Set the camera direction to always be towards the point `p`, no matter where the camera
    center is. */
    auto& set_camera_lookat(const Point3D &p) {camera_lookat = p; return *this;}
    /* Set the focus distance (the distance from the camera center to the plane of perfect
    focus) to `focus_distance` units. Objects placed at that distance will appear in perfect
    focus, and will linearly appear blurrier the further away they are from that distance. This
    therefore allows for simulating defocus blur (depth-of-field) in the camera. */
    auto& set_focus_distance(double focus_distance) {focus_dist = focus_distance; return *this;}
    /* Set the defocus angle of the camera (the angle of the cone with apex at the center of the
    viewpoint and with base as the defocus disk, which is centered at the camera center) to
    `defocus_angle_degrees` DEGREES, not radians. Smaller defocus angles result in less blur,
    and setting the defocus angle to 0 eliminates all blur, making everything render in perfect
    focus (which is equivalent to `Camera::turn_blur_off()`). */
    auto& set_defocus_angle(double defocus_angle_degrees) {
        defocus_angle = defocus_angle_degrees * std::numbers::pi / 180;  /* convert to radians */
        return *this;
    }
    /* Causes this Camera to render the whole scene in perfect focus, with no defocus blur. */
    auto& turn_blur_off() {defocus_angle = 0; return *this;}
    /* Sets the "camera up" direction to the vector `dir`. The true camera up direction will be
    determined by taking the projection of `dir` onto the viewport. */
    auto& set_camera_up_direction(const Vec3D &dir) {view_up_dir = dir; return *this;}
    /* Sets the width of the final outputted image to `width`. */
    auto& set_image_width(size_t width) {image_w = width; return *this;}
    /* Sets the height of the final outputted image to `height`. */
    auto& set_image_height(size_t height) {image_h = height; return *this;}
    /* Sets the dimensions of the final outputted image to be `width` by `height`. */
    auto& set_image_dimensions(size_t width, size_t height) {
        image_w = width;
        image_h = height;
        return *this;
    }
    /* Sets the image width to `width`, and infers the image height from `width` and
    `aspect_ratio`.*/
    auto& set_image_by_width_and_aspect_ratio(size_t width, double aspect_ratio) {
        auto height = static_cast<size_t>(std::round(static_cast<double>(width) / aspect_ratio));
        /* Make sure height is at least 1 */
        return set_image_dimensions(width, std::max(size_t{1}, height));
    }
    /* Sets the image height to `height`, and infers the image width from `height` and
    `aspect_ratio`. */
    auto& set_image_by_height_and_aspect_ratio(size_t height, double aspect_ratio) {
        auto width = static_cast<size_t>(std::round(static_cast<double>(height) * aspect_ratio));
        /* Make sure width is at least 1 */
        return set_image_dimensions(std::max(size_t{1}, width), height);
    }

    /* Sets the number of rays sampled for each pixel to `samples`. */
    auto& set_samples_per_pixel(size_t samples) {samples_per_pixel = samples; return *this;}
    /* Sets the maximum recursive depth for the camera (the maximum number of bounces for
    a given light ray) to `max_depth_`. */
    auto& set_max_depth(size_t max_depth_) {max_depth = max_depth_; return *this;}
    /* Sets the vertical FOV (field of view) of the camera to `vertical_fov_degrees` degrees.
    The horizontal FOV will then be inferred from `vertical_fov` and the image's aspect ratio
    in `init()`. */
    auto& set_vertical_fov(double vertical_fov_degrees) {
        vertical_fov = vertical_fov_degrees * std::numbers::pi / 180;  /* convert to radians */
        horizontal_fov.reset();
        return *this;
    }
    /* Sets the horizontal FOV (field of view) of the camera to `horizontal_fov_degrees` degrees.
    The vertical FOV will then be inferred from `horizontal_fov` and the image's `aspect_ratio`
    in `init()`. */
    auto& set_horizontal_fov(double horizontal_fov_degrees) {
        horizontal_fov = horizontal_fov_degrees * std::numbers::pi / 180;  /* convert to radians */
        vertical_fov.reset();
        return *this;
    }

    /* Prints this `Camera` to the `std::ostream` specified by `os`. */
    void print_to(std::ostream &os) {
        init();  /* Calculate all members before printing them */
        os << "Camera {\n"
           << "\tImage dimensions: " << image_w << " x " << image_h << '\n'
           << "\tViewport dimensions: " << viewport_w << " x " << viewport_h << '\n'
           << "\tpixel_delta_x: " << pixel_delta_x << '\n'
           << "\tpixel_delta_y: " << pixel_delta_y << '\n'
           << "\tCamera center: " << camera.origin << '\n'
           << "\tCamera direction: " << camera.dir << '\n'
           << "\tUp direction: " << view_up_dir << '\n'
           << "\tCamera orientation x-, y-, and z- orthonormal basis vectors {"
           << "\n\t\tx: " << cam_basis_x
           << "\n\t\ty: " << cam_basis_y
           << "\n\t\tz: " << cam_basis_z << "\n\t}\n"
           << "\tFocus distance: " << *focus_dist << '\n'
           << "\tDefocus angle: " << defocus_angle << " rad, "
           << defocus_angle * 180 / std::numbers::pi << " degrees\n"
           << "\tDefocus dist x-, y- orthonormal basis vectors {"
           << "\n\t\tx: " << defocus_disk_x
           << "\n\t\ty: "<< defocus_disk_y << "\n\t}\n"
           << "\tViewport top-left corner location: " << pixel00_loc << '\n'
           << "\tSamples per pixel: " << samples_per_pixel << '\n'
           << "\tMaximum bounces per ray: " << max_depth << '\n'
           << "\tVertical FOV (-1 means not given): " << vertical_fov.value_or(-1) << " rad, "
           << (vertical_fov ? *vertical_fov * 180 / std::numbers::pi : -1) << " degrees\n"
           << "\tHorizontal FOV (-1 means not given): " << horizontal_fov.value_or(-1) << " rad, "
           << (horizontal_fov ? *horizontal_fov * 180 / std::numbers::pi : -1) << " degrees\n}";
    }
};

/* Overload `operator<<` to allow printing `Camera`s to output streams */
std::ostream& operator<< (std::ostream &os, Camera cam) {
    cam.print_to(os);
    return os;
}

#endif