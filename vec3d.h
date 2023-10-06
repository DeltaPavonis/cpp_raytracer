#include <iostream>
#include <cmath>

/* Vec3D represents a 3-dimensional vector, or equivalently, a point in 3D space. */
struct Vec3D {
    /* Three `double` components, 0 by default */
    double x = 0, y = 0, z = 0;

    /* Mathematical negation, +=, -=, *=, /= operators */
    auto operator-() {return Vec3D{-x, -y, -z};}

    auto& operator+= (const Vec3D &rhs) {x += rhs.x; y += rhs.y; z += rhs.z; return *this;}
    auto& operator-= (const Vec3D &rhs) {x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;}
    auto& operator*= (double d) {x *= d; y *= d; z *= d; return *this;}
    auto& operator/= (double d) {x /= d; y /= d; z /= d; return *this;}

    /* Compute magnitude (length) and squared magnitude */
    auto mag() const {return std::sqrt(x * x + y * y + z * z);}
    auto mag_squared() const {return x * x + y * y + z * z;}

    /* Compute unit vector (forward declared) */
    auto unit_vector() const;
};

/* Utility functions; vector addition/subtraction, multiplication/division by a scalar */
auto operator+ (const Vec3D &a, const Vec3D &b) {auto ret = a; ret += b; return ret;}
auto operator- (const Vec3D &a, const Vec3D &b) {auto ret = a; ret -= b; return ret;}
auto operator* (const Vec3D &a, double d) {auto ret = a; ret *= d; return ret;}
auto operator* (double d, const Vec3D &a) {return a * d;}
auto operator/ (const Vec3D &a, double d) {auto ret = a; ret /= d; return ret;}

/* Dot and cross product of two vectors*/
auto dot(const Vec3D &a, const Vec3D &b) {return a.x * b.x + a.y * b.y + a.z * b.z;}
auto cross(const Vec3D &a, const Vec3D &b) {
    return Vec3D{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

/* Overload operator<< to allow printing `Vec3D`s to output streams */
std::ostream& operator<<(std::ostream& os, const Vec3D &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

/* Unit vector is found by dividing the vector by its length/magnitude */
auto Vec3D::unit_vector() const {return *this / this->mag();}

/* `Point3D` is a type alias for `Vec3D`, declared to improve clarity in the code */
using Point3D = Vec3D;