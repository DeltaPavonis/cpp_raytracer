#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include "rgb.h"
#include "progressbar.h"

struct Image {
    size_t width, height;
    std::vector<std::vector<RGB>> pixels;

    auto& operator[] (size_t row) {return pixels[row];}
    const auto& operator[] (size_t row) const {return pixels[row];}

    auto aspect_ratio() const {return static_cast<double>(width) / static_cast<double>(height);}

    /* Prints this `Image` in PPM format to the file with name specified by `destination` */
    void send_as_ppm(const std::string &destination) {
        if (std::ofstream fout(destination); !fout.is_open()) {
            std::cout << "Error in `print_as_ppm(\"" << destination << "\"):"
                         "Could not open the file named " << destination << std::endl;
        } else {
            /* See https://en.wikipedia.org/wiki/Netpbm#PPM_example */
            fout << "P3\n" << width << " " << height << "\n255\n";
            for (ProgressBar<size_t> w(0, width, "Storing image as PPM"); w(); ++w) {
                for (size_t h = 0; h < height; ++h) {
                    fout << pixels[w][h].as_string() << '\n';
                }
            }

            std::cout << "Image successfully saved to \"" << destination << "\"" << std::endl;
        }
    }

    /* Creates an image with width `w` and height `h` */
    static auto with_dimensions(size_t w, size_t h) {
        return Image{
            .width = w,
            .height = h,
            .pixels = std::vector<std::vector<RGB>>(w, std::vector<RGB>(h))
        };
    }

    /* Creates an image with width `w` and aspect ratio (width / height) `aspect_ratio` */
    static auto with_width_and_aspect_ratio(size_t w, double aspect_ratio) {
        return with_dimensions(
            w, 
            static_cast<size_t>(std::round(static_cast<double>(w) * aspect_ratio))
        );
    }

    /* Creates an image with height `h` and aspect ratio (width / height) `aspect_ratio` */
    static auto with_height_and_aspect_ratio(size_t h, double aspect_ratio) {
        return with_dimensions(
            static_cast<size_t>(std::round(static_cast<double>(h) / aspect_ratio)),
            h
        );
    }
    
    /* Creates an image from a two-dimensional array of `RGB` pixels.
    Requires `img` to be a rectangular array. */
    static auto from_data(const std::vector<std::vector<RGB>> &img) {
        return Image {
            .width = img.size(),
            .height = img[0].size(),
            .pixels = img
        };
    }
};