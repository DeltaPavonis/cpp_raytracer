#include "image.h"

using namespace std;

int main()
{
    auto img = Image::with_dimensions(256, 256);
    for (ProgressBar<size_t> j(0, img.height(), "Creating image"); j(); ++j) { /* rows from top to bottom */
        for (size_t i = 0; i < img.width(); ++i) { /* cols from left to right */

            /* black at top left, more red as you go down, more green as you go right */
            img[j][i] = RGB::from_mag(
                static_cast<double>(i) / static_cast<double>(img.width() - 1),
                static_cast<double>(j) / static_cast<double>(img.height() - 1),
                0
            );
        }
    }

    img.send_as_ppm("part_1_basic_image.ppm");

    return 0;
}