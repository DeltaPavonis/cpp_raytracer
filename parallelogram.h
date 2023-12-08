#ifndef PARALLELOGRAM_H
#define PARALLELOGRAM_H

#include "vec3d.h"
#include "hittable.h"
#include "material.h"

/* `Parallelogram` is an abstraction over a 2D parallelogram in 3D space. */
class Parallelogram : public Hittable {
    /* A 2D parallelogram in 3D space is represented by a given vertex, and two vectors
    corresponding to the two sides of the parallelogram _ */

    /* `vertex` = A given vertex of the parallelogram. */
    Point3D vertex;
    /* `side1` = A vector representing the first side of the parallelogram, starting at the given
    vertex `vertex`. As a result, `vertex + side1` yields the vertex adjacent to `vertex`
    along side 1.

    `side2` = A vector representing the second side of the parallelogram, starting at the given
    vertex `vertex`. As a result, `vertex + side2` yields the vertex adjacent to `vertex`
    along side 2.

    And by the Parallelogram Vector Addition Rule, the vertex opposite to `vertex` is equivalent
    to `vertex + side1 + side2`. */
    Vec3D side1, side2;
    /* `material` = The material of this `Parallelogram` object. */
    std::shared_ptr<Material> material;

    /* `unit_plane_normal` = n.unit_vector(), where `n = cross(side1, side2)`, a normal vector
    to the plane containing this `Parallelogram`. We precompute this quantity, because it is
    returned as a part of the `hit_info` for every ray-`Parallelogram` intersection in
    `Parallelogram::hit_by()` as the `outward_unit_surface_normal`.
    
    Note that for a flat object, "outside" and "inside" is arbitrary. We just keep it simple
    here and say that the direction of `cross(side1, side2)` is the outside normal. You could
    say instead that the direction of `cross(side1, side2)` is the INSIDE normal; it'd make
    no difference, except for flipping the `hit_from_outside` for all `hit_info`s returned
    from `Parallelogram::hit_by()`. */
    Vec3D unit_plane_normal;
    /* `scaled_plane_normal` = n / dot(n, n), where `n = cross(side1, side2)`, a normal vector
    to the plane containing this `Parallelogram`. We precompute this quantity to be used in
    `Parallelogram::hit_by()`. */
    Vec3D scaled_plane_normal;

    AABB aabb;

public:

    /* There are three steps to perform a ray-parallelogram intersection check:
    1. Find the plane that contains the parallelogram
    2. Find the intersection point of the ray with that parallelogram-containing plane
    3. Determine if the hitpoint of the ray on the plane lies within the parallelogram itself.
    
    STEP 1: Find the plane that intersects the parallelogram.
    Remember that a plane is determined by a normal vector `n` to that plane, as well as a
    point on the plane. We want to find the plane that contains the parallelogram. Now, a
    normal vector to the parallelogram (and so also a normal vector to the plane containing
    that parallelogram) can be found by taking the cross product of the two side vectors;
    `cross(side, side2)`. Then, we need an arbitrary point on the plane; for this, we can just
    take the given vertex `vertex`, which we know to be on the parallelogram (and so we know
    it to be on the parallelogram-containing plane as well).
    
    Now, we have that a normal to the plane that contains this parallelogram is
    `n = cross(side1, side2)`, and a point on the plane is `vertex`. As a result, the plane
    equation can be written as `dot(n, P - vertex) = 0` for points `P (so a point `P` is on
    the plane iff the vector from Q to it is normal to the plane's normal vector). An equivalent
    and more useful formulation of this equation that we will use is `dot(n, P) = dot(n, vertex)`.
    This completes the first step.
    
    STEP 2: Find the intersection point of the ray with the parallelogram-containing plane.
    Let the ray be defined by R(t) = O + td. First, we solve for the hit time of the ray
    with the plane; that is, we find the `t` that satisfies `dot(n, R(t)) = dot(n, vertex)`.
    Because R(t) = O + td, this becomes `dot(n, O + td) = dot(n, vertex)`, and so
    `dot(n, O) + dot(n, td) = dot(n, vertex)`. Solving, we find that
    `t = dot(n, vertex - O) / dot(n, d)`.

    Observe that it is possible for a ray to not intersect the plane; this happens when the
    ray is parallel to the plane and the ray's origin is not on the plane. When the ray is
    parallel to the plane, then it will be perpendicular to the plane's normal vector, and
    so `dot(n, d)` will equal 0, allowing us to detect such cases. In practice, we will
    just reject all rays where `dot(n, d)` is small, to avoid numerical issues. In my
    implementation below, I reject all rays where `|dot(n, d)| < 1e-9`.

    We now have shown that the hit time is `t = dot(n, vertex - O) / dot(n, d)` (assuming
    that `dot(n, d)` is not too small; if it is, then again, we just assume the ray does
    not hit this parallelogram to avoid numerical issues). To find the hit point, just
    find `O + td` with that value of `t`. This completes the second step.

    STEP 3: Determine if the hit point of the ray with the parallelogram-containing plane also
    lies within the parallelogram itself.

    First, observe that {`side1`, `side2`} forms a basis for the parallelogram-containing plane,
    because `side1` and `side2` are linearly independent (since they are not parallel), and because
    they are two vectors in a space (a plane) of dimension two. Now, we choose the "origin" of
    the parallelogram-containing plane to be `vertex`. Then, we know that
    (a) Because {`side1`, `side2`} is a basis of the plane, there exist unique scalars alpha/beta
    such that `hit_point = vertex + alpha * side1 + beta * side2`.
    (b) Because of the definition of a parallelogram, and because we chose the origin of the plane
    to be `vertex` itself, we know that the hit point is inside the parallelogram if and only if
    0 <= alpha, beta <= 1, where `hit_point = vertex + alpha * side1 + beta * side`, as stated
    above.

    Thus, it remains to solve the equation `hit_point = vertex + alpha * side1 + beta * side2`.
    This is equivalent to solving `hit_point - vertex = alpha * side1 + beta * side2`. We can
    solve for `beta` by taking the cross product of both sides with `side1`: then we get
    `cross(side1, hit_point - vertex) = cross(side1, alpha * side1) + cross(side1, beta * side2)`.
    Then, `cross(side1, alpha * side1) = alpha * cross(side1, side1) = alpha * 0 = 0`, and so
    we have `cross(side1, hit_point - vertex) = cross(side1, beta * side2)
    = beta * cross(side1, side2)`. Now, because we cannot divide by vectors in vector math, we
    solve for `beta` by taking the dot product of both sides with `n` (wait until (*) to see why
    we specifically choose to take the dot product of both sides with `n`), the normal to the plane
    (which equals `cross(side1, side2)`). This yields `dot(n, cross(side1, hit_point - vertex))
    = dot(n, beta * cross(side1, side2)) = beta * dot(n, cross(side1, side2))`. Now, because the
    dot product is a scalar, we are allowed to divide by both sides of the equation by
    `dot(n, cross(side1, side2))` (*). Finally, this yields
    `beta = dot(n, cross(side1, hit_point - vertex)) / dot(n, cross(side1, side2))`. Finally,
    because `n = cross(side1, side2)` by definition, we can simplify this to
    `beta = dot(n / dot(n, n), cross(side1, hit_point - vertex)).

    To find alpha` instead, go back to the original equation `hit_point - vertex = alpha * side1
    + beta * side2`, and take the cross product of both sides with `side2` instead of `side1`.
    This yields
    `alpha = dot(n / dot(n, n), cross(hit_point - vertex, side2))`.

    We will precompute `n / dot(n, n)` (we save this in `scaled_plane_normal`; it is equal to
    `n / |n|^2`, not sure if there's a better name for this) to increase performance. Finally,
    it suffices to check that `0 <= alpha, beta <= 1` to see if the hit point is inside the
    parallelogram itself. This completes the final step, and we are done. Now, check out the
    implementation below.

    (*) We would not be allowed to divide both sides of the equation by dot(n, cross(side1, side2))
    if it was equal to 0. However, because `n` is the normal to the plane, it in fact is EQUAL to
    `cross(side1, side2)`, and so `dot(n, cross(side1, side2)) = dot(n, n) = |n|^2 > 0`, so we
    can always divide by `dot(n, cross(side1, side2))`. Note that this assumes that `n != 0`, but
    this is fine, because the only way that `n = 0` is if `side1` and `side2` are parallel, or when
    one of the sides has magnitude 0, both of which mean that the parallelogram itself is invalid.
    We assume that the parallelogram is valid before doing a ray-parallelogram intersection, so
    this will not affect our algorithm.
    */
 
    /* `Parallelogram::hit_by(ray)` returns a `std::optional<hit_info>` object representing the
    minimum time of intersection (well, for parallelograms there's at most one time of intersection
    except when the ray is coplanar; we actually consider that case as a non-intersection to avoid
    numerical issues) in the time range specified by `ray_times`, of `ray` with this Parallelogram.
    If `ray` does not hit this Parallelogram in the interval `ray_times`, an empty `std::optional`
    object is returned. */
    std::optional<hit_info> hit_by(const Ray3D &ray, const Interval &ray_times) const override {

        /* The hit time is equal to dot(n, vertex - ray.origin) / dot(n, ray.dir), where `n` is
        any normal to the parallelogram-containing plane. Since we have already precomputed
        `scaled_plane_normal`, we just use that. All that matters is that the `n` used in the
        numerator and denominator of the fraction is the same `n`.
        
        Now, as explained above, we first check if the ray is parallel (or very close to parallel)
        to the parallelogram-containing plane. If so, we just return an empty
        `std::optional<hit_info>`, signifying that the ray did not intersect this parallelogram. */
        auto hit_time_denominator = dot(scaled_plane_normal, ray.dir);
        if (std::fabs(hit_time_denominator) < 1e-9) {
            return {};
        }

        /* If the ray is not parallel/very close to parallel to the plane, compute the hit time.
        First, make sure that the hit time is in the desired time range `ray_times`. */
        auto hit_time = dot(scaled_plane_normal, vertex - ray.origin) / hit_time_denominator;
        if (!ray_times.contains_exclusive(hit_time)) {  /* Remember that `ray_times` is exclusive */
            return {};
        }

        /* Compute the hit point by evaluating the ray at `hit_time`. */
        auto hit_point = ray(hit_time);
        /* Store `hit_point - vertex` in a variable. Note that `hit_point - vertex` is a planar
        vector, because both `hit_point` and `vertex` are in the parallelogram-containing plane.
        Specifically, it is the vector from the plane's origin (which we set to `vertex`) to
        the hitpoint of this ray with the plane. Thus, we call it `planar_hitpoint_vector`. */
        auto planar_hitpoint_vector = hit_point - vertex;

        /* Compute the basis coordinates (using the basis {`side1`, `side2`}) of `hit_point`
        in this plane, again, using `vertex` as the origin of this plane. */
        auto alpha = dot(scaled_plane_normal, cross(planar_hitpoint_vector, side2));
        auto beta = dot(scaled_plane_normal, cross(side1, planar_hitpoint_vector));

        /* The ray's hitpoint on the parallelogram-containing plane is in the parallelogram itself,
        iff `0 <= alpha, beta <= 1`. */
        if (auto i = Interval(0, 1); i.contains_inclusive(alpha) && i.contains_inclusive(beta)) {
            /* We just pass in `unit_plane_normal`, which we precomputed, as the
            `outward_unit_surface_normal`. See the comments above the definition of
            `unit_plane_normal` for more explanation on why we do this.*/
            return hit_info(hit_time, hit_point, unit_plane_normal, ray, material);
        }

        /* The ray hit the plane, but the hitpoint was not in the parallelogram itself, so we will
        return an empty `std::optional<hit_info>`.*/
        return {};
    }

    /* Returns the AABB (Axis-Aligned Bounding Box) for this `Parallelogram`. */
    AABB get_aabb() const override {
        return aabb;
    }

    /* Prints this `Parallelogram` to the `std::ostream` specified by `os`. */
    void print_to(std::ostream &os) const override {
        os << "Parallelogram {vertex: " << vertex << ", side 1 vector: " << side1
           << ", side 2 vector: " << side2 << " } " << std::flush;
    }

    /* --- CONSTRUCTORS --- */

    /* @brief Returns the parallelogram specified by a vertex `vertex_` (a `Point3D`) and 
    sides `side1_` and `side2_` (which are `Vec3D`s starting from the vertex `vertex_`),
    with material specified by `material_`.
    
    @param vertex_ Some vertex of the parallelogram. 
    @param side1_ The first side of the parallelogram as a `Vec3D`. `vertex_ + side1_` thus yields
    the vertex adjacent to `vertex_` along the first side of the parallelogram.
    @param side2_ The second side of the parallelogram as a `Vec3D`. `vertex_ + side2_` thus yields
    the vertex adjacent to `vertex_` along the second side of the parallelogram.
    @param material_ The material of the resulting parallelogram.
    
    @note This used to be a named constructor called `with_vertex_and_sides`; however, a named
    constructor would not work here because `Parallelogram` objects need to be stored within
    `std::shared_ptr`s. */
    Parallelogram(const Point3D &vertex_, const Vec3D &side1_, const Vec3D &side2_,
                  std::shared_ptr<Material> material_)
        : vertex{vertex_}, side1{side1_}, side2{side2_}, material{std::move(material_)}
    {
        /* Precompute `unit_plane_normal` and `scaled_plane_normal`; see the comments above
        their respective definitions. */
        auto plane_normal = cross(side1, side2);  /* This is `n` in those comments */
        unit_plane_normal = plane_normal.unit_vector();
        /* Note that dot(n, n) = |n|^2, so `scaled_plane_normal` = n / dot(n, n) = n / |n|^2. */
        scaled_plane_normal = plane_normal / plane_normal.mag_squared();

        /* The `AABB` for a `Parallelogram` is simply the minimum-size `AABB` that contains all the
        vertices of the parallelogram; that is, the `AABB` containing `vertex`, `vertex + side1`,
        `vertex + side2`, and the opposite vertex (which, remember, is calculated by
        `vertex + side1 + side2` for parallelograms).

        Then, because parallelograms are 2D, the resulting AABB may have zero thickness in one
        of its dimensions if it is parallel to the xy-/xz-/yz-plane, which can result in numerical
        issues. To avoid this, we pad the axis intervals of the AABB, making sure that every
        axis interval has length at least some small constant (1e-4 here). */
        aabb = AABB::from_points({
            vertex,                  /* The given vertex of this parallelogram */
            vertex + side1,          /* The vertex opposite to `vertex` along the first side */
            vertex + side2,          /* The vertex opposite to `vertex` along the second side */
            vertex + side1 + side2   /* The vertex opposite to `vertex` in this  parallelogram */
        }).ensure_min_axis_length(1e-4);  /* Make sure each axis of the AABB has length >= 1e-4 */
    }
};

#endif