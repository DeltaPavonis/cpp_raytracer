#ifndef BVH_NODE_H
#define BVH_NODE_H

#include <algorithm>
#include "scene.h"

class BVHNode : public Hittable {

    /* `aabb` = The AABB (Axis-Aligned Bounding Box) for the set of objects represented by this
    `BVHNode`. */
    AABB aabb;

    /* `left`/`right` = the left and right children, respectively, of this `BVHNode` within
    the Bounding Volume Hierarchy. Remember that the hierarchy is a tree, and in our case
    it is a binary tree, because each `BVHNode` has at most two children. */
    std::shared_ptr<Hittable> left, right;

public:

    /* Return the `hit_info`, if any, representing the closest object hit by the ray `ray`
    in the time range `ray_times` in this `BVHNode`. */
    std::optional<hit_info> hit_by(const Ray3D &ray, const Interval &ray_times) const override {

        /* Key idea behind BVH's: if the ray `ray` doesn't hit the bounding volume for a set of
        objects (which can be checked by a single ray-AABB intersection test), then we immediately
        know that it will not hit any of the objects inside that bounding volume. Thus, we
        just instantly return an empty `std::optional<hit_info>` if the bounding box for the
        current `BVHNode` is not intersected by `ray` in the time interval `ray_times`.  */
        if (!aabb.is_hit_by(ray, ray_times)) {
            return {};
        }

        /* Otherwise, if `ray` does hit the bounding box, we need to recursively check the left and
        right children of the current `BVHNode`, and return the `hit_info` with the smallest
        `hit_time` resulting from the two children. */
        if (auto info_l = left->hit_by(ray, ray_times); info_l) {
            /* If `ray` hits some object in the left child, then check if it hits any object in the
            right child at an earlier time. If it does - that is, if `ray` hits some object in the
            right child of this `BVHNode` in the time range (`ray_times.min`, `info_l->hit_time`) -
            then return the `hit_info` from the right child. Otherwise, we know that the earliest
            object hit occurs in the left child, so we return `info_l` then. */
            auto info_r = right->hit_by(ray, Interval(ray_times.min, info_l->hit_time));
            return (info_r ? info_r : info_l);
        } else {
            /* If `ray` hits no object in this BVHNode's left child, then just return the `hit_info`
            from the right child. */
            return right->hit_by(ray, ray_times);
        }
    }

    /* Returns the AABB (Axis-Aligned Bounding Box) for this `BVHNode`. */
    AABB get_aabb() const override {
        return aabb;
    }

    /* Prints this `BVHNode` */
    void print_to(std::ostream &os) const override {
        os << "BVHNode {aabb: " << aabb << "} " << std::flush;  /* TODO: recursively */
    }

    BVHNode(const Scene &world) : BVHNode(world, 0, world.size()) {}

    BVHNode(const std::vector<std::shared_ptr<Hittable>> &objects, size_t start, size_t end) {

        /* Make a modifiable copy of `objects`, because we need to sort the objects */
        auto objects_copy = objects;

        /* Choose an uniformly-random axis to sort all the objects in `objects_copy` by */
        auto split_axis = rand_int(0, 2);  /* Randomly choose one of the x, y, or z axes */
        auto comp = [&](const std::shared_ptr<Hittable> &a, const std::shared_ptr<Hittable> &b){
            return a->get_aabb().get_axis(split_axis).min < b->get_aabb().get_axis(split_axis).min;
        };

        /* Divide the current set of objects (namely, `objects[start..end`]) into two subsets, and
        then assign each of those two sets to the left and right children. This is recursive, and
        it is why Bounding Volume Hierarchies are indeed hierarchies (in our case, binary trees). */
        if (end - start == 1) {
            /* When `objects` has only one object, just duplicate it in the left and right children.
            We do this because it guarantees that no `left` or `right` pointer of any valid
            `BVHNode` will be `nullptr`, saving us from having to do this check in
            `BVHNode::hit_by()`. */
            left = right = objects[start];
        } else if (end - start == 2) {
            /* When `objects` has two objects, just put one in the left child and one in the right
            child. It does not matter which object in particular is put into the left/rights child.
            */
           left = objects[start];
           right = objects[start + 1];
        } else {
            /* When the current `BVHNode` has more than 2 objects, split the half with the lesser
            coordinates (along the randomly-chosen axis) into the left child of the `BVHNode`, and
            split the half with the larger coordinates along the randomly-chosen axis into the
            right child. Note, again, that the order does not matter - we could choose to split
            the half with the lesser coordinates into the right child instead of the left; it makes
            no difference.
            
            To split `objects_copy[start..end)` into two halves by the randomly-chosen axis, we use
            the standard library algorithm `std::nth_element` with the comparator specified as 
            `comp`. The tutorial uses `std::sort` instead, but this is unnecessary; we don't need
            to sort all the elem elements, we just to find out which half each one is in. */
            auto mid = (start + end) / 2;
            std::nth_element(objects_copy.begin() + start, objects_copy.begin() + mid,
                             objects_copy.begin() + end, comp);

            /* Recursively build the left and right children of this `BVHNode`. The left
            child gets `objects_copy[start..mid)`, and the right child gets
            `objects_copy[mid..end)`. */
            left = std::make_shared<BVHNode>(objects_copy, start, mid);
            right = std::make_shared<BVHNode>(objects_copy, mid, end);
        }

        /* Compute the AABB for this `BVHNode` by combining the computed AABB of the current
        `BVHNode`s left and right children. */
        aabb = AABB::combine(left->get_aabb(), right->get_aabb());
    }
};

#endif