#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include "rgb.h"
#include "progressbar.h"

class Image {
    /* Image width, image height, and the 2D array of `RGB` pixels with that width and height */
    size_t w, h;
    std::vector<std::vector<RGB>> pixels;

    Image(size_t w_, size_t h_) : w{w_}, h{h_}, pixels(h, std::vector<RGB>(w)) {}
    Image(const std::vector<std::vector<RGB>> &pixels_) : w{pixels_[0].size()}, h{pixels_.size()},
                                                          pixels{pixels_} {}

public:
    auto width() const {return w;}
    auto height() const {return h;}
    auto& operator[] (size_t row) {return pixels[row];}
    const auto& operator[] (size_t row) const {return pixels[row];}

    auto aspect_ratio() const {return static_cast<double>(w) / static_cast<double>(h);}

    /* Prints this `Image` in PPM format to the file with name specified by `destination` */
    void send_as_ppm(const std::string &destination) {
        if (std::ofstream fout(destination); !fout.is_open()) {
            std::cout << "Error in `print_as_ppm(\"" << destination << "\"):"
                         "Could not open the file named " << destination << std::endl;
        } else {
            /* See https://en.wikipedia.org/wiki/Netpbm#PPM_example */
            fout << "P3\n" << w << " " << h << "\n255\n";
            for (ProgressBar<size_t> row(0, h, "Storing PPM image"); row(); ++row) {
                for (size_t col = 0; col < w; ++col) {
                    fout << pixels[row][col].as_string() << '\n';
                }
            }

            std::cout << "Image successfully saved to \"" << destination << "\"" << std::endl;
        }
    }

    /* --- NAMED CONSTRUCTORS --- */

    /* Creates an image with width `width` and height `height` */
    static auto with_dimensions(size_t width, size_t height) {
        return Image(width, height);
    }

    /* Creates an image with width `w` and aspect ratio (width / height) `aspect_ratio` */
    static auto with_width_and_aspect_ratio(size_t width, double aspect_ratio) {
        auto height = static_cast<size_t>(std::round(static_cast<double>(width) / aspect_ratio));

        /* Make sure height is at least 1 */
        return with_dimensions(width, std::max(size_t{1}, height));
    }

    /* Creates an image with height `h` and aspect ratio (width / height) `aspect_ratio` */
    static auto with_height_and_aspect_ratio(size_t height, double aspect_ratio) {
        auto width = static_cast<size_t>(std::round(static_cast<double>(height) * aspect_ratio));

        /* Make sure width is at least 1 */
        return with_dimensions(std::max(size_t{1}, width), height);
    }
    
    /* Creates an image from a two-dimensional array of `RGB` pixels.
    Requires `img` to be a rectangular array. */
    static auto from_data(const std::vector<std::vector<RGB>> &img) {
        return Image(img);
    }
};

#endif