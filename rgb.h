#ifndef RGB_H
#define RGB_H

#include <cmath>
#include <string>
#include "rand_util.h"

/* Returns the gamma-encoded value of the magnitude `d`, under a gamma of `gamma`. */
auto linear_to_gamma(double d, double gamma = 2) {
    /* See https://stackoverflow.com/a/16521337/12597781. */
    return std::pow(d, 1 / gamma);
}

/* `RGB` encapsulates the notion of color as three real-valued numbers in the range [0, 1],
representing the magnitudes of the red, green, and blue components, respectively. */
class RGB {
    RGB(double r_, double g_, double b_) : r{r_}, g{g_}, b{b_} {}

public:

    /* Real-valued red, green, and blue components, each ranging from 0.0 to 1.0
    (if representing a valid color). */
    double r, g, b;
    
    /* --- NAMED CONSTRUCTORS --- */

    /* Creates a RGB color, given red, green, and blue components (each in the range 0.0 to 1.0) */
    static RGB from_mag(double red, double green, double blue) {
        return RGB(red, green, blue);
    }

    /* Creates a RGB color with red, green, and blue components all set to `val` (where
    0.0 <= `val` <= 1.0) */
    static RGB from_mag(double val) {
        return from_mag(val, val, val);
    }

    /* Creates a RGB color with red, green, and blue components, where each ranges from
    `0` to `max_magnitude` */
    static RGB from_rgb(double red, double green, double blue, double max_magnitude = 255) {
        return RGB(red / max_magnitude, green / max_magnitude, blue / max_magnitude);
    }

    /* Creates a RGB color where the red, green, and blue components range from `0` to
    `max_magnitude`, with all three components equal to `val` */
    static RGB from_rgb(double val, double max_magnitude = 255) {
        return from_rgb(val, val, val, max_magnitude);
    }

    /* Creates a RGB color with red, green, and blue components set to 0 */
    static RGB zero() {return from_mag(0);}

    /* Creates a RGB with random red, green, and blue components, each a real number
    in the range [`min`, `max`] */
    static RGB random(double min = 0, double max = 1) {
        return from_mag(rand_double(min, max), rand_double(min, max), rand_double(min, max));
    }

    /* Mathematical operators (since anti-aliasing requires finding the average of
    multiple colors, so we need += and /=) */
    auto& operator+= (const RGB &rhs) {r += rhs.r; g += rhs.g; b += rhs.b; return *this;}
    auto& operator*= (double d) {r *= d; g *= d; b *= d; return *this;}
    auto& operator/= (double d) {r /= d; g /= d; b /= d; return *this;}

    /* Returns this `RGB` object gamma-encoded, and as a string.
    `delimiter`: What is printed between the red, green, and blue components. A space by default.
    `surrounding`: What is printed at the beginning and the end; if empty, then nothing is printed.
    Otherwise, the first and second characters are printed directly before and after the numbers,
    respectively.
    `max_magnitude`: Represents the "full" magnitudes of red, green, and blue. 255 by default.
    `gamma`: The encoding gamma for gamma correction. 2 by default. If the raw values of the RGB
    intensities are desired, set `gamma` to 1.
    */
    auto as_string(std::string delimiter = " ", std::string surrounding = "",
                   double max_magnitude = 255, double gamma = 2) const
    {
        /* Add 0.999999 to `max_magnitude` to allow truncating to `max_magnitude` itself */
        auto scale = max_magnitude + 0.999999;
        return (surrounding.empty() ? "" : std::string{surrounding[0]})
              + std::to_string(static_cast<int>(scale * linear_to_gamma(r, gamma))) + delimiter
              + std::to_string(static_cast<int>(scale * linear_to_gamma(g, gamma))) + delimiter
              + std::to_string(static_cast<int>(scale * linear_to_gamma(b, gamma)))
              + (surrounding.empty() ? "" : std::string{surrounding[1]});
    }
};

/* Mathematical utility functions */
auto operator* (const RGB &a, double d) {auto ret = a; ret *= d; return ret;}
auto operator* (double d, const RGB &a) {return a * d;}
auto operator* (const RGB &a, const RGB &b) {return RGB::from_mag(a.r * b.r, a.g * b.g, a.b * b.b);}

/* Returns a color linearly interpolated, with a proportion of `1 - d` of `a` and
a proportion of `d` of `b`. */
auto lerp(const RGB &a, const RGB &b, double d) {
    return RGB::from_mag(
        (1 - d) * a.r + d * b.r,
        (1 - d) * a.g + d * b.g,
        (1 - d) * a.b + d * b.b
    );
}

#endif