#include <string>

struct RGB {
    double r = 0, g = 0, b = 0;
    
    static RGB from_mag(double red, double green, double blue) {
        return RGB{red, green, blue};
    }

    static RGB from_mag(double val) {
        return from_mag(val, val, val);
    }

    static RGB from_rgb(double red, double green, double blue, double max_magnitude = 255) {
        return RGB{red / max_magnitude, green / max_magnitude, blue / max_magnitude};
    }

    static RGB from_rgb(double val, double max_magnitude = 255) {
        return from_rgb(val, val, val, max_magnitude);
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