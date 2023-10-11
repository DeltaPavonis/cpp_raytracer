#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "vec3d.h"
#include "image.h"

/* In path tracing, the "viewport" or "image plane" is a virtual rectangle upon which the 3D scene
is projected. The class `Viewport` encapsulates the notion of a viewport along with a designated
camera/eye point. */
class Viewport {

    Viewport(double w, double h, size_t img_w, size_t img_h, const Point3D &camera_center_,
             double focal_length_)
      : width{w}, height{h},
        /* Right-handed coordinates: The y-axis goes up, the x-axis goes right, the NEGATIVE z-axis
        points from the camera toward the image. Thus, the vector going down the viewport (`y_vec`)
        has a negative y-component. */
        x_vec{width, 0, 0}, y_vec{0, -height, 0},
        pixel_delta_x{x_vec / static_cast<double>(img_w)},
        pixel_delta_y{y_vec / static_cast<double>(img_h)},
        camera_center{camera_center_}, focal_length{focal_length_},
        /* The upper left point of the viewport is found by starting at the camera, moving
        `focal_length` units towards the camera (so adding -`focal_length` to the z-coordinate
        of `camera_center`, due to the use of right-handed coordinates), then subtracting half the
        x- and y- coordinates of `x_vec` and `y_vec`.*/
        upper_left{camera_center + Point3D{0, 0, -focal_length} - x_vec / 2 - y_vec / 2},
        /* Pixels are inset from the edges of the viewport by half the pixel-to-pixel distance.
        This ensures that the viewpoint area is evenly divided into `pixel_delta_x`-times-
        `pixel_delta_y`-sized regions. */
        pixel00{upper_left + (pixel_delta_x + pixel_delta_y) / 2}
        {}
    

public:

    /* Width and height of the viewport. Note that they are real-valued. */
    double width, height;
    /* Vectors right and down across the viewport horizontal and vertical edges. */
    Vec3D x_vec, y_vec;
    /* The horizontal and vertical delta vectors from pixel to pixel. */
    Point3D pixel_delta_x, pixel_delta_y;
    /* Coordinates of the camera/eye point and its distance away from the viewport */
    Point3D camera_center;
    double focal_length;
    /* Upper left coordinate of the viewport, and the coordinates of the top-left image pixel */
    Point3D upper_left, pixel00;

    /* Compute the coordinates of the center of the pixel in row `row` and column `col` */
    auto pixel_center(size_t row, size_t col) const {
        return pixel00 + static_cast<double>(row) * pixel_delta_y
            + static_cast<double>(col) * pixel_delta_x;
    }

    static auto from_width_and_image(double w, const Image &img, const Point3D &camera_center = {},
                                     double focal_length = 1) {
        /* Viewport heights less than 1 are ok because they are real-valued, unlike Image heights.
        Also note that we must use `img.aspect_ratio()` and not the theoretical desired ratio, because
        the ideal ratio may not be the actual aspect ratio of `img`, because `img.width()` and
        `img.height()` both must be integers. */
        return Viewport(w, w / img.aspect_ratio(), img.width(), img.height(), camera_center,
                        focal_length);
    }

    static auto from_height_and_image(double h, const Image &img, const Point3D &camera_center = {},
                                      double focal_length = 1) {
        /* Viewport widths less than 1 are ok because they are real-valued, unlike Image widths
        Also note that we must use `img.aspect_ratio()` and not the theoretical desired ratio, because
        the ideal ratio may not be the actual aspect ratio of `img`, because `img.width()` and
        `img.height()` both must be integers. */
        return Viewport(h * img.aspect_ratio(), h, img.width(), img.height(), camera_center,
                        focal_length);
    }

    static auto from_height_and_image(double h, const ImagePPMStream &img, const Point3D &camera_center = {},
                                      double focal_length = 1) {
        /* Viewport widths less than 1 are ok because they are real-valued, unlike Image widths
        Also note that we must use `img.aspect_ratio()` and not the theoretical desired ratio, because
        the ideal ratio may not be the actual aspect ratio of `img`, because `img.width()` and
        `img.height()` both must be integers. */
        return Viewport(h * img.aspect_ratio(), h, img.width(), img.height(), camera_center,
                        focal_length);
    }
};

#endif