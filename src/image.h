#ifndef IMAGE_H
#define IMAGE_H

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>  /* For std::exit() */
#include "rgb.h"
#include "progressbar.h"

/* The `Image` type encapsulates a 2D image as a 2D array of `RGB` pixels. It is appropriate for
images that need manipulations, because it stores and allows access to all the`RGB` pixels. If you
only need an image to be sent as PPM to a file, use `ImagePPMStream`. */
class Image {
    /* Image width, image height, and the 2D array of `RGB` pixels with that width and height */
    size_t w, h;
    std::vector<std::vector<RGB>> pixels;

    /* If given only a width and a height for the array of pixels, set all pixels to have
    color (0, 0, 0); that is, black. */
    Image(size_t w_, size_t h_) : w{w_}, h{h_}, pixels(h, std::vector<RGB>(w, RGB::zero())) {}

    /* Constructs an `Image` from the given rectangular pixel array `pixels_`. */
    Image(const std::vector<std::vector<RGB>> &pixels_) : w{pixels_[0].size()}, h{pixels_.size()},
                                                          pixels{pixels_} {}

public:
    auto width() const {return w;}
    auto height() const {return h;}
    auto& operator[] (size_t row) {return pixels[row];}
    const auto& operator[] (size_t row) const {return pixels[row];}

    auto aspect_ratio() const {return static_cast<double>(w) / static_cast<double>(h);}

    /* Prints this `Image` in PPM format to the file with name specified by `destination`. */
    void send_as_ppm(const std::string &destination) const {
        if (std::ofstream fout(destination); !fout.is_open()) {
            std::cout << "Error: In Image::print_as_ppm(), could not open the file \""
                      << destination << "\"" << std::endl;
            std::exit(-1);
        } else {
            /* See https://en.wikipedia.org/wiki/Netpbm#PPM_example */
            fout << "P3\n" << w << " " << h << "\n255\n";
            ProgressBar pb(h, "Storing PPM image to " + destination);
            for (size_t row = 0; row < h; ++row) {
                for (size_t col = 0; col < w; ++col) {
                    fout << pixels[row][col].as_string() << '\n';
                }
                pb.update();
            }

            std::cout << "Image successfully saved to \"" << destination << "\"" << std::endl;
        }
    }

    auto& outline_border() {
        for (size_t row = 0; row < h; ++row) {
            pixels[row][0] = pixels[row][w - 1] = RGB::from_mag(1);
        }

        for (size_t col = 1; col < w - 1; ++col) {
            pixels[0][col] = pixels[h - 1][col] = RGB::from_mag(1);
        }

        return *this;
    }

    /* --- NAMED CONSTRUCTORS --- */

    /* Creates an image with width `width` and height `height` */
    static auto with_dimensions(size_t width, size_t height) {
        return Image(width, height);
    }

    /* Creates an image with width `w` and width-to-height ratio `aspect_ratio` */
    static auto with_width_and_aspect_ratio(size_t width, double aspect_ratio) {
        auto height = static_cast<size_t>(std::round(static_cast<double>(width) / aspect_ratio));

        /* Make sure height is at least 1 */
        return with_dimensions(width, std::max(size_t{1}, height));
    }

    /* Creates an image with height `h` and width-to-height ratio `aspect_ratio` */
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

    /* Creates an image corresponding to the PPM file with name `file_name`. */
    static auto from_ppm_file(const std::string &file_name) {

        /* Try to open the file `file_name` */
        std::ifstream fin(file_name);
        if (!fin.is_open()) {
            std::cout << "Error: In Image::from_ppm_file(), could not find/open the file \""
                      << file_name << "\"" << std::endl;
            std::exit(-1);
        }

        /* Require that first line is "P3" */
        std::string first_line;
        std::getline(fin, first_line);
        if (first_line != "P3") {
            std::cout << "Error: In Image::from_ppm_file(\"" << file_name << "\"), first line "
                         "of file was not \"P3\", but instead was " << first_line << std::endl;
            std::exit(-1);
        }

        /* Input image width and image height */
        size_t image_width, image_height;
        if (!(fin >> image_width >> image_height)) {
            std::cout << "Error: In Image::from_ppm_file(\"" << file_name << "\"), could not "
                         "parse image width and height (two integers) on second line" << std::endl;
            std::exit(-1);
        }

        /* Input maximum RGB magnitude */
        int max_magnitude;
        if (!(fin >> max_magnitude)) {
            std::cout << "Error: In Image::from_ppm_file(\"" << file_name << "\"), could not "
                         "parse RGB max magnitude (one integer), which should occur on the third "
                         "line, right after the image width and height on the second line"
                      << std::endl;
            std::exit(-1);
        }

        /* Input RGB data for all pixels*/
        std::vector ppm_data(image_height, std::vector<RGB>(image_width, RGB::zero()));  /* CTAD */
        for (size_t row = 0; row < image_height; ++row) {
            for (size_t col = 0; col < image_width; ++col) {

                /* Read (r, g, b). If it fails, raise an error */
                int r, g, b;
                if (!(fin >> r >> g >> b)) {
                    std::cout << "Error: In Image::from_ppm_file(\"" << file_name << "\"), failed "
                                 "to parse color #" << (row * image_width) + col + 1 <<
                                 " (three integers (r, g, b))" << std::endl;
                    std::exit(-1);
                }

                /* Require that all of r, g, b are non-negative */
                if (r < 0 || g < 0 || b < 0) {
                    std::cout << "Error: In Image::from_ppm_file(\"" << file_name << "\"), found "
                                 "negative RGB channel value; color #"
                              << (row * image_width) + col + 1 << " was (" << r << ", " << g
                              << ", " << b << ")" << std::endl;
                    std::exit(-1);
                }
                
                ppm_data[row][col] = RGB::from_rgb(r, g, b, max_magnitude);
            }
        }

        return from_data(ppm_data);
    }
};

/* `ImagePPMStream` progressively takes in the `RGB` pixels of an image with a specified width and
height, in the order of top to bottom then left to right, and prints those pixels to a specified
file. Unlike `Image`, it does not allow access to the pixels of the image, because it does not store
the 2D array of pixels representing the image, which saves storage. Thus, for images that need
post-processing, the `Image` type is more appropriate. */
class ImagePPMStream {
    std::string file;
    std::ofstream fout;
    size_t w, h, curr_index;

    ImagePPMStream(const std::string &file_, size_t w_, size_t h_)
      : file{file_}, fout{file}, w{w_}, h{h_}, curr_index{0}
    {
        /* Print PPM header upon construction */
        fout << "P3\n" << w << " " << h << "\n255\n";
    } 

public:

    size_t width() const {return w;}
    size_t height() const {return h;}
    size_t size() const {return w * h;}
    auto aspect_ratio() const {return static_cast<double>(w) / static_cast<double>(h);}

    /* Redirect this `ImagePPMStream` to print to the file `file_name`. Gives an error
    if `file_name` cannot be opened, and gives a warning if the file switch takes
    place in the middle of image printing (meaning the first file will be left with
    an incomplete PPM image) */
    void set_file(const std::string &file_name) {
        if (fout.open(file_name); !fout.is_open()) {  /* Could not open `file_name` */
            std::cout << "Error: In ImagePPMStream::set_file(), could not open the file \""
                      << file_name << "\"" << std::endl;
            std::exit(-1);
        }
        
        /* Warn user if they switch in the middle of printing an image */
        if (curr_index > 0) {
            std::cout << "Warning: In ImagePPMStream::set_file(\"" << file_name << "\"), original"
                      << " file \"" << file << "\" is left incomplete; " << curr_index
                      << " out of " << w * h << " pixels printed" << std::endl;
        }

        file = file_name;
        curr_index = 0;  /* Printing starts over for the new file */
    }

    void add(const RGB &rgb) {
        if (curr_index == size()) {  /* Error, already printed every pixel */
            std::cout << "Error: Called ImagePPMStream::add() " << size() + 1 << " times for image"
                         "of size " << size() << std::endl;
            std::exit(-1);
        }
        fout << rgb.as_string() << '\n';
        ++curr_index;
    }

    /* --- NAMED CONSTRUCTORS --- */

    /* Creates an ImagePPMStream with specified width and height, to be printed to the file
    `file_name` */
    static auto with_dimensions(size_t width, size_t height, const std::string &file_name) {
        return ImagePPMStream(file_name, width, height);
    }

    /* Creates an ImagePPMStream with width `w` and width-to-height ratio `aspect_ratio`, to
    be printed to the file `file_name` */
    static auto with_width_and_aspect_ratio(size_t width, double aspect_ratio,
                                            const std::string &file_name) {
        auto height = static_cast<size_t>(std::round(static_cast<double>(width) / aspect_ratio));

        /* Make sure height is at least 1 */
        return with_dimensions(width, std::max(size_t{1}, height), file_name);
    }

    /* Creates an ImagePPMStream with height `h` and width-to-height ratio `aspect_ratio`,
    to be printed to the file `file_name` */
    static auto with_height_and_aspect_ratio(size_t height, double aspect_ratio,
                                             const std::string &file_name) {
        auto width = static_cast<size_t>(std::round(static_cast<double>(height) * aspect_ratio));

        /* Make sure width is at least 1 */
        return with_dimensions(std::max(size_t{1}, width), height, file_name);
    }

    ~ImagePPMStream() {
        if (curr_index == w * h) {  /* All w * h pixels outputted, good */
            std::cout << "Image successfully saved to \"" << file << "\"" << std::endl;
        } else {  /* Warn user if some pixel(s) are missing (were not `add`ed) */
            std::cout << "Warning: ImagePPMStream to \"" << file << "\" incomplete; " << curr_index
                      << " out of " << "(" << w << " * " << h << ") = " << w * h
                      << " RGB strings printed at time of destruction" << std::endl;
        }
    }
};

#endif