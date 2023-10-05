#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "rgb.h"
#include "progressbar.h"

struct Image {
    size_t width, height;
    std::vector<std::vector<RGB>> pixels;

    auto& operator[] (size_t row) {return pixels[row];}
    const auto& operator[] (size_t row) const {return pixels[row];}

    /* Prints this `Image` in PPM format to the file with name specified by `destination` */
    void send_as_ppm(const std::string &destination) {
        if (std::ofstream fout(destination); !fout.is_open()) {
            std::cout << "Error in `print_as_ppm(\"" << destination << "\"):"
                         "Could not open the file named " << destination << std::endl;
        } else {
            /* See https://en.wikipedia.org/wiki/Netpbm#PPM_example */
            fout << "P3\n" << width << " " << height << "\n255\n";
            for (ProgressBar<size_t> w(0, width, "Storing image as PPM"); w.update(); ++w) {
                for (size_t h = 0; h < height; ++h) {
                    fout << pixels[w][h].as_string() << '\n';
                }
            }

            std::cout << "Image successfully saved to \"" << destination << "\"" << std::endl;
        }
    }

    /* Constructs an image with width `w` and height `h` */
    Image(size_t w, size_t h) : width{w},
                                height{h},
                                pixels(width, std::vector<RGB>(height)) {}
    
    /* Constructs an image from a two-dimensional array of `RGB` pixels.
    Requires `img` to be a rectangular array. */
    Image(const std::vector<std::vector<RGB>> &img) : width{img.size()},
                                                      height{img[0].size()},
                                                      pixels{img} {}
};