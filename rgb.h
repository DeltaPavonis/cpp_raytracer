#ifndef RGB_H
#define RGB_H

#include <string>

/* `RGB` encapsulates the notion of color as three real-valued numbers in the range [0, 1],
representing the magnitudes of the red, green, and blue components, respectively. */
class RGB {
    RGB(double r_, double g_, double b_) : r{r_}, g{g_}, b{b_} {}

public:

    /* Real-valued red, green, and blue components (each ranging from 0.0 to 1.0) */
    double r, g, b;
    
    /* --- NAMED CONSTRUCTORS --- */

    /* Creates an image, given red, green, and blue components (each in the range 0.0 to 1.0) */
    static RGB from_mag(double red, double green, double blue) {
        return RGB(red, green, blue);
    }

    /* Creates an image with red, green, and blue components all set to `val` (where
    0.0 <= `val` <= 1.0) */
    static RGB from_mag(double val) {
        return from_mag(val, val, val);
    }

    /* Creates an image where the red, green, and blue components range from `0` to
    `max_magnitude` */
    static RGB from_rgb(double red, double green, double blue, double max_magnitude = 255) {
        return RGB(red / max_magnitude, green / max_magnitude, blue / max_magnitude);
    }

    /* Creates an image where the red, green, and blue components range from `0` to
    `max_magnitude`, with all three components equal to `val` */
    static RGB from_rgb(double val, double max_magnitude = 255) {
        return from_rgb(val, val, val, max_magnitude);
    }

    /* Returns this `RGB` object as a string.
    `delimiter`: What is printed between the red, green, and blue components. A space by default.
    `surrounding`: What is printed at the beginning and the end; if empty, then nothing is printed.
    Otherwise, the first and second characters are printed directly before and after the numbers,
    respectively.
    `max_magnitude`: Represents the "full" magnitudes of red, green, and blue. 255 by default.
    */
    auto as_string(std::string delimiter = " ", std::string surrounding = "",
                   double max_magnitude = 255)
    {
        auto scale = max_magnitude + 0.999999;  /* To allow truncating to `max_magnitude` itself */
        return (surrounding.empty() ? "" : std::string{surrounding[0]})
                + std::to_string(static_cast<int>(scale * r)) + delimiter
                + std::to_string(static_cast<int>(scale * g)) + delimiter
                + std::to_string(static_cast<int>(scale * b))
                + (surrounding.empty() ? "" : std::string{surrounding[1]});
    }

    /* Default constructor sets `r` = `g` = `b` = 0. */
    RGB() : r{0}, g{0}, b{0} {}
};

/* Returns a color linearly interpolated, with a proportion of `1 - d` of `a` and
a proportion of `d` of `b`. */
auto lerp(const RGB &a, const RGB &b, double d) {
    return RGB::from_rgb(
        (1 - d) * a.r + d * b.r,
        (1 - d) * a.g + d * b.g,
        (1 - d) * a.b + d * b.b
    );
}

#endif