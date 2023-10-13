#ifndef CAMERA_H
#define CAMERA_H

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
    /* Viewport width and height. Note that these are real-valued. By default, the
    viewport has width 1.0, and its height will be determined at render-time based
    on the aspect ratio of the image (the viewport must have the same aspect ratio
    as the final rendered image). The -1 denotes "will be determined by the other
    dimension". */
    double viewport_w = 1, viewport_h = -1;
    /* The horizontal and vertical delta vectors from pixel to pixel. */
    Vec3D pixel_delta_x{}, pixel_delta_y{};
    /* Coordinates of the camera/eye point and its distance away from the viewport,
    by default (0, 0, 0) and 1, respectively. */
    Point3D camera_center{};
    double focal_length = 1;
    /* Coordinates of the upper left corner of the viewport, and the coordinates of the
    topmost and leftmost image pixel */
    Point3D upper_left_corner{}, pixel00_loc{};
    /* Number of rays sampled per pixel */
    size_t samples_per_pixel = 1;

    /* Set the values of `viewport_w`, `viewport_h`, `pixel_delta_x`, `pixel_delta_y`,
    `upper_left_corner`, and `pixel00_loc` based on `image_w` and `image_h`. This function
    is called before every render. */
    void init() {

        /* Calculate the true aspect ratio of the image. Note that this may be different
        from the aspect ratio passed in calls to `set_image_by_xxxxx_and_aspect_ratio`,
        because `image_w` and `image_h` both must be integers. */
        auto aspect_ratio = static_cast<double>(image_w) / static_cast<double>(image_h);
        /* Set viewport_h based on viewport_w, or vice versa, making sure that the
        aspect ratio of the viewport equals the aspect ratio of the final outputted image.
        Note that viewport widths and heights less than one are ok (unlike for `image_w`
        and `image_h`), because they are real-valued. */
        if (viewport_w < 0) {viewport_w = viewport_h * aspect_ratio;}
        else {viewport_h = viewport_w / aspect_ratio;}

        /* `x_vec` and `y_vec` are the vectors right and down across the viewport, respectively
        We use right-handed coordinates, which means the y-axis goes up, the x-axis goes right,
        and the NEGATIVE z-axis goes into the image. Thus, the vector going down across the
        viewport has a negative y-component. */
        Vec3D x_vec{viewport_w, 0, 0}, y_vec{0, -viewport_h, 0};
        pixel_delta_x = x_vec / static_cast<double>(image_w);  /* Divide by pixels per row */
        pixel_delta_y = y_vec / static_cast<double>(image_h);  /* Divide by pixels per column */

        /* The upper left point of the camera is found by starting at the camera, moving
        `focal_length` units towards the camera (so adding -`focal_length` to the z-coordinate
        of `camera_center`, due to the use of right-handed coordinates), then subtracting half of
        `x_vec` and `y_vec`. */
        upper_left_corner = camera_center + Point3D{0, 0, -focal_length} - x_vec / 2 - y_vec / 2;

        /* Pixels are inset from the edges of the camera by half the pixel-to-pixel distance.
        This ensures that the viewpoint area is evenly divided into `pixel_delta_x` by
        `pixel_delta_y`-sized regions. */
        pixel00_loc = upper_left_corner + pixel_delta_x / 2 + pixel_delta_y / 2;
    }

    /* Returns a ray from `camera_center` through a random point in the square region centered at
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
        return Ray3D(camera_center, pixel_sample -camera_center);
    }

public:

    /* NOTE: `render_to` just takes the function `ray_color` as a parameter for now, because
    `hittable` has not been fully implemented. */
    void render_to(const auto &ray_color, const std::string &file_name) {
        init();

        /* Calculate color of each pixel and write it to the file */
        auto img = ImagePPMStream::with_dimensions(image_w, image_h, file_name);
        for (size_t row = 0; row < image_h; ++row) {
            for (size_t col = 0; col < image_w; ++col) {

                /* Shoot `samples_per_pixel` random rays through the current pixel.
                The average of the resulting colors will be the color for this pixel. */
                RGB pixel_color{};
                for (size_t sample = 0; sample < samples_per_pixel; ++sample) {
                    auto ray = random_ray_through_pixel(row, col);
                    pixel_color += ray_color(ray);
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
        for (size_t row = 0; row < image_h; ++row) {
            for (size_t col = 0; col < image_w; ++col) {

                /* Shoot `samples_per_pixel` random rays through the current pixel.
                The average of the resulting colors will be the color for this pixel. */
                RGB pixel_color{};
                for (size_t sample = 0; sample < samples_per_pixel; ++sample) {
                    auto ray = random_ray_through_pixel(row, col);
                    pixel_color += ray_color(ray);
                }
                pixel_color /= static_cast<double>(samples_per_pixel);

                img[row][col] = pixel_color;
             }
        }
        return img;
    }

    /* Setters. Each returns a mutable reference to this object to create a functional interface */

    /* Sets the camera center to the point `p`. */
    auto& set_camera_center(const Point3D &p) {camera_center = p; return *this;}
    /* Sets the focal length (the distance from the camera center to the viewport) to `focal_len`. */
    auto& set_focal_length(double focal_len) {focal_length = focal_len; return *this;}
    /* Sets the width of the viewport to `width`. The height of the viewport will
    be inferred later so that the viewport's aspect ratio matches the image's aspect ratio. */
    auto& set_viewport_width(double width) {viewport_w = width; viewport_h = -1; return *this;}
    /* Sets the height of the viewport to `height`. The width of the viewport will
    be inferred later so that the viewport's aspect ratio matches the image's aspect ratio. */
    auto& set_viewport_height(double height) {viewport_h = height; viewport_w = -1; return *this;}
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
};

#endif