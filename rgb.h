#include <string>

struct RGB {
    double r = 0, g = 0, b = 0;

    static RGB from_mag(double val) {
        return RGB{val, val, val};
    }

    static RGB from_mag(double red, double green, double blue) {
        return RGB{red, green, blue};
    }

    static RGB from_rgb_255(int val) {
        return RGB{val / 255., val / 255., val / 255.};
    }

    static RGB from_rgb_255(int red, int green, int blue) {
        return RGB{red / 255., green / 255., blue / 255.};
    }

    auto as_string(std::string delimiter = " ", std::string surrounding = "",
                   double scale = 255.999999)
    {
        return (surrounding.empty() ? "" : std::string{surrounding[0]})
                + std::to_string(static_cast<int>(scale * r)) + delimiter
                + std::to_string(static_cast<int>(scale * g)) + delimiter
                + std::to_string(static_cast<int>(scale * b))
                + (surrounding.empty() ? "" : std::string{surrounding[1]});
    }
};