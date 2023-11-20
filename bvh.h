#ifndef BVH_H
#define BVH_H

#include <algorithm>
#include <span>
#include <cassert>
#include <chrono>
#include "scene.h"

/* `BVH` is an abstraction over a Bounding Volume Hierarchy, which is a data structure that allows
for sublinear ray-scene intersection tests. Implementation inspired by
https://pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies. */
class BVH : public Hittable {

    /* `BucketInfo` */
    struct BucketInfo {
        /* `num_primitives` = the number of primitives whose centroids fall into this bucket */
        size_t num_primitives = 0;
        /* `aabb` = the AABB for all centroids that fall into this bucket */
        AABB aabb;
    };

    struct BVHNode : public Hittable {
        /* `aabb` = The AABB (Axis-Aligned Bounding Box) for the set of objects represented by
        this `BVHNode`. */
        AABB aabb;

        /* `left`/`right` = the left and right children, respectively, of this `BVHNode` within
        the Bounding Volume Hierarchy. Remember that the hierarchy is a tree, and in our case
        it is a binary tree, because each `BVHNode` has at most two children.
        
        The reason why `left` and `right` are pointers to `Hittable` rather than `BVHNode` is
        because the leaves of a Bounding Volume Hierarchy are the primitives themselves. Thus,
        `BVHNode`s may need to point to general `Hittable`s as well as `BVHNode`s, and so
        `left` and `right` must be pointers to `Hittable`, not just `BVHNode`. */
        std::shared_ptr<Hittable> left, right;

        /* Return the `hit_info`, if any, representing the closest object hit by the ray `ray`
        in the time range `ray_times` in this `BVHNode`. */
        std::optional<hit_info> hit_by(const Ray3D &ray, const Interval &ray_times) const override {
            
            /* Key idea behind BVH's: if the ray `ray` doesn't hit the bounding volume for a set of
            objects (which can be checked by a single ray-AABB intersection test), then we
            immediately know that it will not hit any of the objects inside that bounding volume.
            Thus, we just instantly return an empty `std::optional<hit_info>` if the bounding box
            for the current `BVHNode` is not intersected by `ray` in the time interval `ray_times`.
            */
            if (!aabb.is_hit_by(ray, ray_times)) {
                return {};
            }

            /* If `left == right`, then this `BVHNode` is a leaf node in the `BVH`, and we can just
            return the result of the ray-intersection test with `left` (we don't need to test if the
            ray intersects `right` afterwards because `left == right`). */
            if (left == right) {
                return left->hit_by(ray, ray_times);
            }

            /* Otherwise, if `ray` does hit the bounding box, we need to recursively check the left
            and right children of the current `BVHNode`, and return the `hit_info` with the smallest
            `hit_time` resulting from the two children. */
            if (auto info_l = left->hit_by(ray, ray_times); info_l) {
                /* If `ray` hits some object in the left child, then check if it hits any object in
                the right child at an earlier time. If it does - that is, if `ray` hits some object
                in the right child of this `BVHNode` in the time range (`ray_times.min`,
                `info_l->hit_time`) - then return the `hit_info` from the right child. Otherwise, we
                know that the earliest object hit occurs in the left child, so we return `info_l`
                then. */
                auto info_r = right->hit_by(ray, Interval(ray_times.min, info_l->hit_time));
                return (info_r ? info_r : info_l);
            } else {
                /* If `ray` hits no object in this BVHNode's left child, then just return the
                `hit_info` from the right child. */
                return right->hit_by(ray, ray_times);
            }
        }

        /* Returns the AABB (Axis-Aligned Bounding Box) for this `BVHNode`. */
        AABB get_aabb() const override {
            return aabb;
        }

        /* Prints this `BVHNode` */
        void print_to(std::ostream &os) const override {
            os << "BVHNode {aabb: " << aabb << "} " << std::flush;
        }

        /* --- NAMED CONSTRUCTORS --- */

        /* Returns a `std::shared_ptr` containing a leaf `BVHNode`, which itself contains all the
        primitives in `objects`. */
        static auto as_leaf_node(std::span<std::shared_ptr<Hittable>> objects) {
            auto ret = std::make_shared<BVHNode>();
            /* Leaf `BVHNode`s have their `left` and `right` pointers set to the same memory
            address, so that traversing through a `BVH` tree structure never leads to `nullptr`,
            simplifying the code. This trick is from TNW. */
            ret->left = ret->right = std::make_shared<Scene>(objects);  /* Then, `left == right` */
            ret->aabb = ret->left->get_aabb();  /* And so ret->aabb is just left/right's AABB */
            return ret;
        }
    };

    /* `root` = the root `BVHNode` of this `BVH` tree. */
    std::shared_ptr<BVHNode> root;
    /*
    `MAX_PRIMITIVES_IN_NODE` = the maximum number of primitives we allow to be held in a single
    `BVHNode`.
    `NUM_BUCKETS` = the number of buckets to test
    */
    const size_t MAX_PRIMITIVES_IN_NODE, NUM_BUCKETS;
    /* `total_bvh_nodes` = the number of `BVHNode`s in this `BVH` tree. This does not count
    the `Scene`s made in `BVHNode::as_leaf_node()`, which the leaf `BVHNodes` point to. */
    size_t total_bvh_nodes = 0;

    /* Used to have "free on memory that was not `malloc`ed" error. Fixed it with this change:
    before, `objects` was a vector of `Hittable*`, and in the `start == end - 1` condition seems
    to be where the error begun. I thought the Hittable shared_ptr constructor would work though
    with the raw pointer? Guess not. It's weird. After I changed the `objects` parameter to just
    a vector of shared_ptr<Hittable>'s, it worked.
    
    We learned why the error occurred. It's because when we construct a std::shared_ptr from a raw
    memory address, it creates a new reference counter, because a new shared_ptr doesn't know that
    there is a previously-existing reference counter to the same location. So, what will happen is a
    double-free error; when one of the reference counts drops to 0, it calls the destructor on the
    memory, and when the second reference counter drops to 0, it tries to destroy the object again,
    leading to the double-free error. */
    auto build(std::span<std::shared_ptr<Hittable>> objects) {
        ++total_bvh_nodes;

        /* If there is only one primitive left, then we will return a leaf `BVHNode`
        (a `BVHNode` whose children are both primitives) that just contains that one primitive. */
        if (objects.size() == 1) {
            return BVHNode::as_leaf_node(objects);
        }

        /* Compute `centroids_aabb`, the AABB of all the centroids of all the AABBs of
        objects[start..end). */
        auto centroids_aabb = AABB::empty();
        for (auto &i : objects) {
            centroids_aabb.merge_with(i->get_aabb().centroid());
        }

        /* The minimum split "cost" (explained below), the minimum-cost bucket (explained below)
        at which to split, and the axis along which the minimum-cost bucket splits by. */
        auto min_split_cost = std::numeric_limits<double>::infinity();
        size_t min_split_bucket, min_split_axis;
        
        /* Along each of the x-, y-, and z-axes... */
        for (size_t axis = 0; axis < 3; ++axis) {

            /* When all the primitives' centroids have the same coordinate along the current axis,
            then centroids_aabb[axis].size() will equal 0, and so the calculation for `offset` below
            will result in floating-point divisions by 0. To avoid this, we just skip any such axis
            where `centroids_aabb[axis].size()` is 0. To avoid a compiler warning about using `==`
            with floating-points, we don't use `centroids_aabb[axis].size() == 0`, and instead use
            the standard library function `std::fpclassify` to check for equality with 0. */
            if (centroids_aabb[axis].is_empty()) {
                continue;
            }

            /* ...divide `centroids_aabb` into `NUM_BUCKETS` equally-sized regions along the axis,
            and for each region, compute its `BucketInfo`: the number of primitives whose centroids
            lie inside that region, and the AABB of those centroids that lie inside the region. */
            std::vector<BucketInfo> buckets(NUM_BUCKETS);
            /* To do this, iterate through all the primitives being considered. For each, determine
            which bucket (region) it lies in, and update that bucket correspondingly. */
            for (auto &i : objects) {

                /* Compute the bucket in which the centroid of the object `i` lies.
                `offset` = the ratio of*/
                auto offset = (i->get_aabb().centroid()[axis] - centroids_aabb[axis].min)
                             / centroids_aabb[axis].size();
                auto curr_bucket = static_cast<size_t>(static_cast<double>(NUM_BUCKETS) * offset);

                /* In case `offset` = 1 exactly, we would have `curr_bucket` equal to `NUM_BUCKETS`,
                which would cause an out-of-bounds access in `buckets`. This happens whenever this
                object's centroid has the maximum coordinate along this axis among all the objects.
                When this happens, we place the current object in the very last bucket. */
                if (curr_bucket == NUM_BUCKETS) {
                    --curr_bucket;  /* Decrement `curr_bucket`; place `i` in the very last bucket */
                }

                /* Update the currents bucket's `BucketInfo` */
                buckets[curr_bucket].num_primitives++;
                buckets[curr_bucket].aabb.merge_with(i->get_aabb());
            }


            /* In building a BVH using the SAH (Surface Area Heuristic), a greedy algorithm is used,
            with the goal of minimizing the current node's "cost" (expected time needed to find
            the closest object the ray intersects in the current node's objects). The idea is
            simple: we can either just make the current set of objects its own leaf node (so the
            cost would then be the sum, across all objects for the current node, of the expected
            time needed for a ray-object intersection test. If we assume the expected time is
            equal for all objects, we can say WLOG that the expected time is always 1, and so
            the cost would just be equal to the number of objects). Alternatively, we can split
            the current node into two leaf nodes A and B (note that we don't account for
            possibly splitting the nodes A and B; the greedy algorithm only looks at splitting
            the set of objects for the current node).
            
            Now, remember that the "cost" (expected time needed to find the closest object the ray
            intersects) when choosing to splitting the primitive set S into two leaf nodes A and B
            is the sum of
            1. The time needed to check if the ray intersects the AABB of S (because we always
            check this first; if the ray doesn't intersect the AABB of S then we immediately
            know it doesn't intersect any object in S).
            2. The time needed to check if the ray intersects any primitive in A
                This equals (probability that ray intersects the AABB of A given that it intersects
                the AABB of S) * (time needed to check if the ray intersects everything in A)
            3. The time needed to check if the ray intersects any primitive in B; the same as (2).

            Now, by Crofton's formula, we have that the probability that the ray intersects the
            AABB of A given that it intersects the AABB of S is just
            (surface area of AABB of A) / (surface area of AABB of S)
            This is identical for B, and it is very useful to us, because the surface area of AABBs
            are really easy to compute. Note: see https://en.wikipedia.org/wiki/Crofton_formula.

            Furthermore, we note that (1) is a constant, so we can ignore it, since we only care
            about the minimum cost (so we only care about relative costs, not absolute costs).
            Also, note that we don't need to divide by the surface area of the AABB of S, since
            that is invariant across all possible partitions of S. Finally, by our initial
            assumption that all ray-object intersections take the same time, the cost becomes
            (surface area of AABB of A) * (number of objects in A)
            + (surface area of AABB of B) * (number of objects in B)

            Now, for each bucket, compute the above cost that results from splitting the current
            set of objects into all objects after this bucket, and all other objects. That
            is, for bucket i, we have A = set of all objects in buckets 0..i, and B = set of
            all objects in buckets i + 1..NUM_BUCKETS - 1. Thus, `costs[i]` = the cost resulting
            from splitting after bucket `i`. Note that we don't consider splitting after the very
            last bucket because that wouldn't be splitting anything at all. We then choose the
            bucket with the minimum cost. */
            std::vector<double> costs(NUM_BUCKETS - 1);

            /* Compute the (surface area of AABB of A) * (number of objects in A) term for all
            buckets in O(`NUM_BUCKETS`) time, and add it to the correspond `costs[i]`. */
            auto aabb_before_split = AABB::empty();
            size_t num_primitives_before_split = 0;
            for (size_t split_after = 0; split_after < NUM_BUCKETS - 1; ++split_after) {
                aabb_before_split.merge_with(buckets[split_after].aabb);
                num_primitives_before_split += buckets[split_after].num_primitives;

                costs[split_after] = aabb_before_split.surface_area()
                                   * static_cast<double>(num_primitives_before_split);
            }

            /* Compute the (surface area of AABB of B) * (number of objects in B) term for all
            buckets in O(`NUM_BUCKETS`) time, and add it to the correspond `costs[i]`. */
            auto aabb_after_split = AABB::empty();
            size_t num_primitives_after_split = 0;
            /* Iterate in reverse order */
            for (int i = static_cast<int>(NUM_BUCKETS) - 2; i >= 0; --i) {
                aabb_after_split.merge_with(buckets[i].aabb);
                num_primitives_after_split += buckets[i].num_primitives;

                costs[i] += aabb_after_split.surface_area()
                          * static_cast<double>(num_primitives_after_split);
            }

            /* Update `min_split_cost`, `min_split_bucket`, and `min_split_axis` with all the
            buckets we tested along the current `axis` */
            for (size_t i = 0; i < NUM_BUCKETS - 1; ++i) {
                /* Again, we are looking for the split that leads to the minimum cost. */
                if (costs[i] < min_split_cost) {
                    min_split_cost = costs[i];
                    min_split_bucket = i;
                    min_split_axis = axis;
                }
            }
        }

        /* Edge case: When every single primitive's AABB's centroid is the same exact point, then
        we will just create a leaf node that contains all of `objects`, because "none of the
        splitting methods [learned] are effective in that (unusual) case" (from PBR 4th edition:
        see https://tinyurl.com/mpv4f4px). We can detect this edge case by checking if
        `min_split_cost` is equal to `std::numeric_limits<double>::infinity()`, because that means
        `min_split_cost` was never set throughout the above for-loop across all the axes, which
        only happens when every axes' interval in `centroids_aabb` is empty (because then we
        would have `continue`d on every loop iteration). Thus, this check suffices.
        
        As for why we don't just make `min_split_cost` a `std::optional<double>`, it's so when
        we do set `min_split_cost`, we can just check `costs[i] < min_split_cost` without
        having to write separate logic for when the `std::optional<double>` object is empty. */
        if (std::isinf(min_split_cost)) {
            return BVHNode::as_leaf_node(objects);
        }

        /* We either split the current object set into two sets, or we just make the current objects
        set its own leaf. Again, by our simplifying assumptions, the "cost" of just making the
        current objects set its own leaf is equal to the number of objects in the objects set; that
        is, objects.size(). */
        auto leaf_cost = static_cast<double>(objects.size());

        /* If the leaf cost is more than the cost of the minimum split we found, or if
        there are too many primitives left to make a single leaf node (as determined by
        the constant `MAX_PRIMITIVES_IN_NODE`), then we will choose to use the minimum-cost
        split that we found. */
        if (objects.size() > MAX_PRIMITIVES_IN_NODE || min_split_cost < leaf_cost) {

            /* Split `objects` into two sets, the first containing all objects that fall into bucket
            at most `min_split_bucket` along the axis `min_split_axis`, and the second containing
            all other objects. To do this, we use `std::partition` on `objects`. */
            auto mid = std::partition(objects.begin(), objects.end(),
            [&](const std::shared_ptr<Hittable> &object) {
                /* Copied from above; determine which bucket the current `object`'s centroid lies
                in */
                auto offset = (object->get_aabb().centroid()[min_split_axis]
                             - centroids_aabb[min_split_axis].min)
                             / centroids_aabb[min_split_axis].size();
                auto curr_bucket = static_cast<size_t>(static_cast<double>(NUM_BUCKETS) * offset);
                if (curr_bucket == NUM_BUCKETS) {  /* In case `offset` = 1; see previously */
                    --curr_bucket;
                }

                /* Split objects by whether or not their bucket along the axis `min_split_axis`
                is at most `min_split_bucket`. */
                return (curr_bucket <= min_split_bucket);
            }) - objects.begin();

            /* Create the `BVHNode` to be returned; we are splitting `objects`, so this will
             be an interior `BVHNode` (not a leaf `BVHNode`). */
            auto ret = std::make_shared<BVHNode>();

            /* Recursively build the left and right children BVHNode's of `ret`; each of `ret->left`
            and `ret->right` will be built over one of the two sets that resulted from splitting
            `objects`. Here, we just build `ret->left` over the set A (all primitives that fall in
            or before the bucket `min_split_bucket`). */
            ret->left = build(objects.subspan(0, mid));  /* `std::span::subspan` is convenient */
            ret->right = build(objects.subspan(mid));

            /* Calculate AABB of `ret` by merging the AABBs of its two children `BVHNode`s */
            ret->aabb = AABB::merge(ret->left->get_aabb(), ret->right->get_aabb());

            return ret;
        } else {
            /* It costs less just to make `ret` a leaf node, and so we will do just that */
            return BVHNode::as_leaf_node(objects);
        }

    }

    /* Recursively prints the `BVH` rooted at `node` to the output stream `os`. */
    void print_to(std::ostream &os, const std::shared_ptr<Hittable> &node, size_t depth = 0) const {
        auto curr = std::dynamic_pointer_cast<BVHNode>(node);
        assert(curr != nullptr);  /* Since we always return before we reach the leaves,
                                     we are guaranteed that `node` indeed contains a `BVHNode`. */
        os << std::string(depth, ' ') << *curr << '\n';
        if (curr->left == curr->right) {
            os << std::string(depth + 1, ' ') << "Primitive: " << *(curr->left) << std::endl;
        } else {
            print_to(os, curr->left, depth + 1);
            print_to(os, curr->right, depth + 1);
        }
    }

public:

    /* Return the `hit_info`, if any, from the closest object in this `BVH` that is hit by the 3D
    ray `ray` in the time interval `ray_times`. */
    std::optional<hit_info> hit_by(const Ray3D &ray, const Interval &ray_times) const override {
        return root->hit_by(ray, ray_times);
    }

    /* Returns the AABB for this `BVH`. */
    AABB get_aabb() const override {
        return root->get_aabb();
    }

    /* Prints this `BVH` to the `std::ostream` specified by `os`. */
    void print_to(std::ostream &os) const override {
        print_to(os, root);
    }

    /* @brief Builds a BVH (Bounding Volume Hierarchy) from the set of objects specified in `world`.
    `num_buckets` splits will be tested along each of the x-, y-, and z- axes to find the optimal
    split. Each leaf `BVHNode` in the tree structure of this `BVH` will be required to contain at
    most `max_primitives_in_node` primitives.
    
    @param `world`: A `Scene` containing the primitives which this `BVH` will be built over.
    @param `num_buckets`: The number of different splits to test along each of the x-, y-, and
    z-axes when creating a new node in the BVH tree structure. The higher the number of buckets,
    the more likely it is that the split is optimal, but a higher number of buckets also leads to
    longer `BVH` construction times.
    @param `max_primitives_in_node`: The maximum number of primitives that a single leaf node
    in the `BVH` is allowed to contain. That is, it is guaranteed that no leaf node in the `BVH`
    will itself contain more than `max_primitives_in_node` primitives; if a lead node would have
    contained more than `max_primitives_in_node` primitives, then we would have split the
    primitives into separate sets and continued building the `BVH`. The idea behind this is that
    at some point, it is no longer beneficial to keep splitting the objects; a simple linear
    scan through all the objects will be quicker for various reasons (cache, simplicity, etc).
    `max_primitives_in_node` is the boundary between the higher efficiency of using a bounding
    volume hierarchy and the simplicity/cache-friendliness of just doing a ray intersection test
    which every primitive in turn. This exists for the same reason that quicksort implementations
    default to insertion sort when the array is small. */
    BVH(const Scene &world, size_t num_buckets = 32, size_t max_primitives_in_node = 12)
        : MAX_PRIMITIVES_IN_NODE{max_primitives_in_node},
          NUM_BUCKETS{num_buckets}
    {
        std::cout << "Building BVH over " << world.size() << " primitives..." << std::endl;
        auto start = std::chrono::steady_clock::now();

        /* Create a copy of the objects list of `world`, because `world` is passed as `const&`, but
        we need to modify the objects list when building the `BVH` (specifically, we may need to
        rearrange it using `std::partition`) */
        std::vector<std::shared_ptr<Hittable>> objects_copy = world;
        root = build(objects_copy);

        std::cout << "Constructed BVH in "
                  << ms_diff(start, std::chrono::steady_clock::now())
                  << "ms (created " << total_bvh_nodes << " BVHNodes total)\n" << std::endl;
    }
};

#endif