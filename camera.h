#ifndef CAMERA_H
#define CAMERA_H

#include <numbers>
#include "rand_util.h"
#include "image.h"
#include "ray3d.h"

/* The class `Camera` encapsulates the notion of a camera viewing a 3D scene from
a designated camera/eye point, located a certain length (called the focal length)
away from the "viewport" or "image plane": the virtual rectangle upon with the
3D scene is projected to form the final 2D image. */
class Camera {

    /* Width and height (in pixels) of the final rendered image. 1280 x 720 by default */
    size_t image_w = 1280, image_h = 720;
    /* Width and height of the viewport (aka image plane). Note that these are real-valued.
    Both are determined by either `vertical_fov` or `horizontal_fov` during `init()`. */
    double viewport_w{}, viewport_h{};
    /* The horizontal and vertical delta vectors from pixel to pixel. */
    Vec3D pixel_delta_x{}, pixel_delta_y{};
    /* `camera` stores the camera ray; the coordinates of the camera/eye point,
    and the direction in which the camera looks. `focal_length` is the distance of
    the camera away from the viewport. */
    Ray3D camera{.origin = Point3D{0, 0, 0}, .dir = Vec3D{0, 0, -1}};
    double focal_length = std::numeric_limits<double>::infinity();
    /* `view_up_dir` allows the user to specify the "up" direction for the camera.
    Specifically, the "up" direction on the viewport is equal to the projection of
    `view_up_dir` onto the viewport. By default, the "up" direction is the same as the 
    positive y-axis. */
    Vec3D view_up_dir{0, 1, 0};
    /* `basis_x/y/z` form an orthonormal basis of the camera and its orientation.
    `basis_x` is an unit vector pointing to the right, `basis_y` is an unit vector
    pointing up (so `basis_x` and `basis_y` form an orthonormal basis for the viewport),
    and `basis_z` is an unit vector pointing behind the camera, orthogonal to the viewport
    (it poitns behind the camera and not in front due to the use of right-handed coordinates. )*/
    Vec3D basis_x{}, basis_y{}, basis_z{};
    /* Coordinates of the top-left image pixel */
    Point3D pixel00_loc{};
    /* Number of rays sampled per pixel, 50 by default */
    size_t samples_per_pixel = 50;
    /* Maximum number of light ray bounces into the scene, 10 by default */
    size_t max_depth = 10;
    /* Vertical and horizontal FOV (Field of View) of the camera, stored in radians.
    By default, the vertical FOV is 90 degrees, and the horizontal FOV is determined
    by the vertical FOV and the aspect ratio of the image  */
    double vertical_fov = 90, horizontal_fov = std::numeric_limits<double>::infinity();

    /* Set the values of `viewport_w`, `viewport_h`, `pixel_delta_x`, `pixel_delta_y`,
    `upper_left_corner`, and `pixel00_loc` based on `image_w` and `image_h`. This function
    is called before every render. */
    void init() {

        /* Calculate the true aspect ratio of the image. Note that this may be different
        from the aspect ratio passed in calls to `set_image_by_xxxxx_and_aspect_ratio`,
        because `image_w` and `image_h` both must be integers. */
        auto aspect_ratio = static_cast<double>(image_w) / static_cast<double>(image_h);

        /* If `focal_length` is not explicitly provided by the user, then set it equal to
        the length of the camera's direction vector. This is what is done in the tutorial. */
        if (std::isinf(focal_length)) { focal_length = camera.dir.mag();}

        /* Set viewport_w and viewport_h based on `vertical_fov` or `horizontal_fov`,
        `focal_length`, and `aspect_ratio`. */
        if (!std::isinf(vertical_fov)) {
            viewport_h = 2 * focal_length * std::tan(vertical_fov / 2);
            viewport_w = viewport_h * aspect_ratio;
        } else {
            viewport_w = 2 * focal_length * std::tan(vertical_fov / 2);
            viewport_h = viewport_w / aspect_ratio;
        }

        /* Calculate an orthonormal basis for the camera and its orientation. */
        basis_z = -camera.dir.unit_vector();
        basis_x = cross(view_up_dir, basis_z).unit_vector();
        basis_y = cross(basis_z, basis_x);  /* Always an unit vector because basis_z and x are */

        /* `x_vec` and `y_vec` are the vectors right and down across the viewport, respectively
        We use right-handed coordinates, which means the y-axis goes up, the x-axis goes right,
        and the NEGATIVE z-axis goes into the image. Thus, the vector going down across the
        viewport has a negative y-component. */
        Vec3D x_vec = viewport_w * basis_x, y_vec = -viewport_h * basis_y;
        pixel_delta_x = x_vec / static_cast<double>(image_w);  /* Divide by pixels per row */
        pixel_delta_y = y_vec / static_cast<double>(image_h);  /* Divide by pixels per column */

        /* Find `upper_left_corner`, the coordinates of the upper left corner of the viewport.
        The upper left point of the viewport is found by starting at the camera, moving
        `focal_length` units towards the camera (so adding negative `focal_length` times
        `basis_z` to `camera.origin`, due to the use of right-handed coordinates), then
        subtracting half of `x_vec` and `y_vec`. */
        auto upper_left_corner = camera.origin - focal_length * basis_z - x_vec / 2 - y_vec / 2;

        /* Pixels are inset from the edges of the camera by half the pixel-to-pixel distance.
        This ensures that the viewpoint area is evenly divided into `pixel_delta_x` by
        `pixel_delta_y`-sized regions. */
        pixel00_loc = upper_left_corner + pixel_delta_x / 2 + pixel_delta_y / 2;
    }

    /* Returns a ray from `camera.origin` through a random point in the square region centered at
    the pixel in row `row` and column `col`. Note that the region is square because it is a
    rectangle with width |`pixel_delta_x`| and height |`pixel_delta_y`|, and we have `pixel_delta_x`
    = `x_vec` / `image_w` = `viewport_w` / `image_w`, and `pixel_delta_y` = `y_vec` / `image_h`
    = `viewport_h` / `image_h`. Then, `viewport_w` / `image_w` = `viewport_h` / `image_h` because
    `viewport_w` / `viewport_h` = `image_w` / `image_h` (as the aspect ratio of the viewport is
    equal to the aspect ratio of the image).
    
    Then why do we need both `pixel_delta_x` and `pixel_delta_y`? I think it's for clarity. */
    auto random_ray_through_pixel(size_t row, size_t col) {

        /* Find the center of the pixel */
        auto pixel_center = pixel00_loc + static_cast<double>(row) * pixel_delta_y
                          + static_cast<double>(col) * pixel_delta_x;
        
        /* Find a random point in the square region centered at `pixel_center`. The region
        has width `pixel_delta_x` and height `pixel_delta_y`, so a random point in this
        region is found by adding `pixel_delta_x` and `pixel_delta_y` each multiplied by
        a random real number in the range [-0.5, 0.5]. */
        auto pixel_sample = pixel_center + rand_double(-0.5, 0.5) * pixel_delta_x
                          + rand_double(-0.5, 0.5) * pixel_delta_y;
        return Ray3D(camera.origin, pixel_sample - camera.origin);
    }

public:

    /* NOTE: `render_to` just takes the function `ray_color` as a parameter for now, because
    `hittable` has not been fully implemented. */
    void render_to(const auto &ray_color, const std::string &file_name) {
        init();

        /* Calculate color of each pixel and write it to the file */
        auto img = ImagePPMStream::with_dimensions(image_w, image_h, file_name);
        for (ProgressBar<size_t> row(0, image_h, "Rendering to " + file_name); row(); ++row) {
            for (size_t col = 0; col < image_w; ++col) {

                /* Shoot `samples_per_pixel` random rays through the current pixel.
                The average of the resulting colors will be the color for this pixel. */
                auto pixel_color = RGB::zero();
                for (size_t sample = 0; sample < samples_per_pixel; ++sample) {
                    auto ray = random_ray_through_pixel(row, col);
                    pixel_color += ray_color(ray, max_depth);
                }
                pixel_color /= static_cast<double>(samples_per_pixel);

                img.add(pixel_color);
             }
        }
    }

    Image render(const auto &ray_color) {
        init();

        /* Calculate and store the color of each pixel */
        auto img = Image::with_dimensions(image_w, image_h);
        for (ProgressBar<size_t> row(0, image_h, "Rendering"); row(); ++row) {
            for (size_t col = 0; col < image_w; ++col) {

                /* Shoot `samples_per_pixel` random rays through the current pixel.
                The average of the resulting colors will be the color for this pixel. */
                auto pixel_color = RGB::zero();
                for (size_t sample = 0; sample < samples_per_pixel; ++sample) {
                    auto ray = random_ray_through_pixel(row, col);
                    pixel_color += ray_color(ray, max_depth);
                }
                pixel_color /= static_cast<double>(samples_per_pixel);

                img[row][col] = pixel_color;
             }
        }
        return img;
    }

    /* Setters. Each returns a mutable reference to this object to create a functional interface */

    /* Sets the camera center to the point `p`. */
    auto& set_camera_center(const Point3D &p) {camera.origin = p; return *this;}
    /* Sets the camera direction to the vector `dir`. */
    auto& set_camera_direction(const Vec3D &dir) {camera.dir = dir; return *this;}
    /* Sets the point that the camera looks at from the camera center. */
    auto& set_camera_lookat(const Point3D &p) {camera.dir = p - camera.origin; return *this;}
    /* Sets the focal length (the distance from the camera center to the viewport) to `focal_len`. */
    auto& set_focal_length(double focal_len) {focal_length = focal_len; return *this;}
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
    /* Sets the number of rays sampled for each pixel to `rays_per_pixel_`. */
    auto& set_samples_per_pixel(size_t samples) {samples_per_pixel = samples; return *this;}
    /* Sets the maximum recursive depth for the camera (the maximum number of bounces for
    a given light ray) to `max_depth_`. */
    auto& set_max_depth(size_t max_depth_) {max_depth = max_depth_; return *this;}
    /* Sets the vertical FOV (field of view) of the camera to `vertical_fov_degrees` degrees.
    The horizontal FOV will then be inferred from `vertical_fov` and the image's aspect ratio
    in `init()`. */
    auto& set_vertical_fov(double vertical_fov_degrees) {
        vertical_fov = vertical_fov_degrees * std::numbers::pi / 180;
        horizontal_fov = std::numeric_limits<double>::infinity();
        return *this;
    }
    /* Sets the horizontal FOV (field of view) of the camera to `horizontal_fov_degrees` degrees.
    The vertical FOV will then be inferred from `horizontal_fov` and the image's `aspect_ratio`
    in `init()`. */
    auto& set_horizontal_fov(double horizontal_fov_degrees) {
        horizontal_fov = horizontal_fov_degrees * std::numbers::pi / 180;
        vertical_fov = std::numeric_limits<double>::infinity();
        return *this;
    }
};

#endif