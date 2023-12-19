#ifndef BVH_H
#define BVH_H

#include <array>
#include <algorithm>  /* For `std::partition` */
#include <span>
#include <type_traits>  /* For `std::is_base_of_v` (to guarantee static dispatch when possible) */
#include "time_util.h"
#include "scene.h"

/* `BVH` is an abstraction over a Bounding Volume Hierarchy, which is a data structure that allows
for sublinear ray-scene intersection tests. Implementation inspired by
https://pbr-book.org/4ed/Primitives_and_Intersection_Acceleration/Bounding_Volume_Hierarchies. */
class BVH : public Hittable {

    /* `BucketInfo` holds information about a certain coordinate range (a certain "bucket")
    along a given coordinate axis. Specifically, it holds how many of the primitives'
    centroids fall into the bucket, as well as the AABB for all those centroids. */
    struct BucketInfo {
        /* `num_primitives` = the number of primitives whose centroids fall into this bucket */
        size_t num_primitives = 0;
        /* `aabb` = the AABB for all centroids that fall into this bucket */
        AABB aabb;
    };

    /* Each `BVHTreeNode` represents a node in the BVH binary tree. All nodes store an
    AABB that represents the bounds of all primitives it was built over. Every interior
    node stores pointers to its left and right children. Every interior node also stores
    the coordinate axis (x, y, or z) along which its primitives were partitioned for
    distribution to its two children; this information is eventually used to optimize the
    traversal procedure in `BVH::hit_by`. Leaf nodes record the set of primitives they
    contain, which is guaranteed to be a contiguous range in the `primitives` array, so
    we store it under a `std::span` of `primitives`. */
    struct BVHTreeNode {
        /* `aabb` = The AABB (Axis-Aligned Bounding Box) for the set of primitives represented by
        this `BVHTreeNode`. */
        AABB aabb;
        /* `left_child` and `right_child` are pointers to this `BVHTreeNode`'s left and right
        children, respectively. If this `BVHTreeNode` is a leaf node, then both `left_child`
        and `right_child` will have value `nullptr`. */
        std::unique_ptr<BVHTreeNode> left_child, right_child;
        /* If this `BVHTreeNode` is a leaf node, then `primitives` refers to all the primitives
        that this leaf node contains. */
        std::span<std::shared_ptr<Hittable>> primitives;
        /* If this `BVHTreeNode` is an interior (non-leaf) node, then `split_axis` records the
        coordinate axis along which the primitives of this node were split during the tree-building
        process. This information will eventually be used to improve the performance of the
        traversal procedure in `BVH::hit_by`. */
        uint8_t split_axis;  /* 0, 1, and 2 represent splits along the x-, y-, and z- axes */

        /* `BVHTreeNode::is_leaf_node()` returns true iff this `BVHTreeNode` is a leaf node. */
        bool is_leaf_node() const {
            /* Observe that only leaf nodes record the primitives that they contain, and
            every leaf node contains at least one primitive, so it suffices to check if
            the `primitives` field is empty.
            
            Alternatively, we could check that `left_child` or `right_child` is `nullptr`
            to determine if this `BVHTreeNode` is a leaf node. */
            return !primitives.empty();
        }

        /* --- NAMED CONSTRUCTORS --- */

        /* Returns a `std::unique_ptr` containing a leaf `BVHTreeNode`, which contains all the
        primitives in `curr_primitives`. */
        static auto leaf_node(std::span<std::shared_ptr<Hittable>> curr_primitives, AABB &&aabb_) {
            auto node = std::make_unique<BVHTreeNode>();

            /* For leaf nodes, we need to record its primitives and its AABB. */
            node->primitives = curr_primitives;
            node->aabb = aabb_;
            /* Note that `node->left_child` and `node->right_child` are default-initialized
            to `nullptr`, which is what we want for leaf nodes, which have no children. */

            return node;
        }

        /* Returns a `std::unique_ptr` containing an interior (non-leaf) `BVHTreeNode` with left
        and right children given by `left_child` and `right_child`. */
        static auto interior_node(
            std::unique_ptr<BVHTreeNode> left_child,  /* Takes ownership of `left_child` */
            std::unique_ptr<BVHTreeNode> right_child,  /* Takes ownership of `right_child` */
            AABB &&aabb,
            uint8_t split_axis
        ) {
            auto node = std::make_unique<BVHTreeNode>();

            /* For interior nodes, we need to record their AABB, their left and right children,
            and the axis along which their constituent primitives were split. */
            node->aabb = aabb;
            node->left_child = std::move(left_child);
            node->right_child = std::move(right_child);
            node->split_axis = split_axis;

            return node;
        }
    };

    /* Each `LinearBVHNode` represents a node in the flattened (linear) representation
    of the BVH tree. To accomplish that representation, `LinearBVHNode`s store all the
    information we need to traverse the BVH tree: in addition to the AABB for its set
    of primitives, leaf `LinearBVHNode`s store the starting index and length of the
    contiguous range of primitives they contain (`first_primitive_index` and
    `num_primitives` respectively), and interior `LinearBVHNode`s store the index of
    their second (right) child (`second_child_index`) as well as which coordinate
    axis its primitives were partitioned along when building the BVH tree (`split_axis`).
    
    Note that we do not need to store the index of the first child for interior
    `LinearBVHNode`s, because our flattened BVH contains the nodes of the BVH tree
    in preorder. This means the left child of any interior node will be located
    immediately after it in the tree traversal array, and so we do not need to
    store this information in `LinearBVHNode`.
    
    As for why we use `alignas(32)`, this requires a 32-byte alignment in memory
    for `LinearBVHNode`s. As explained in PBR (https://tinyurl.com/3na99zks,
    PBR 4th edition "Bounding Volume Hierarchies"), this improves performance. */
    struct alignas(32) LinearBVHNode {
        /* `aabb` = An AABB (Axis-Aligned Bounding Box) for the set of primitives
        this `LinearBVHNode` contains. */
        AABB aabb;

        union {
            /* For leaf nodes, `first_primitive_index` equals the index of the first 
            primitive the leaf node contains. Together with `num_primitives`, this
            completely specifies the range of primitives contained by the leaf node. */
            size_t first_primitive_index;
            /* For interior nodes, `second_child_index` equals the index of this
            `LinearBVHNode`'s second (right) child in the `linear_bvh_nodes` array.
            Again, the reason we don't need to store the first (left) child's index
            as well is because we store the nodes in preorder; that means the left
            child of any interior node will be located immediately after it (so its
            index will be the original interior node's index plus one). */
            size_t second_child_index;
        };
        /* ^^ Because every `LinearBVHNode` is either a leaf `LinearBVHNode` or a
        interior `LinearBVHNode`, we only need to store one of `first_primitive_index`
        and `second_child_index` at a time. Thus, we place them into an union to save
        storage. */

        /* `num_primitives` = the number of primitives contained by this `LinearBVHNode`.
        For leaf nodes, this is always a strictly positive value, and for interior nodes,
        we will always set it to 0. This allows us to distinguish between leaf and
        interior `LinearBVHNode`s; see `is_leaf_node()`. And because this requires both
        leaf and interior `LinearBVHNode`s to have this value stored, we do NOT place
        `num_primitives` in the union above (as in, we could choose to make an union
        between the pair (first_primitive_index, num_primitives) and second_child_index,
        but we don't because of this). */
        size_t num_primitives;
        /* For interior nodes, `split_axis`*/
        uint8_t split_axis;

        /* `LinearBVHNode::is_leaf_node()` returns true iff this `LinearBVHNode` is a leaf node. */
        bool is_leaf_node() const {
            /* As stated in the comments for `num_primitives`, a given `LinearBVHNode` is a leaf
            node iff its `num_primitives` field is strictly positive. */
            return num_primitives > 0;
        }
    };

    /* `primitives` = The PRIMITIVE COMPONENTS of the `Scene` which this `BVH` was built over. */
    std::vector<std::shared_ptr<Hittable>> primitives;
    /* `MAX_PRIMITIVES_IN_NODE` = the maximum number of primitives we allow to be held in a
    single `BVHTreeNode`.
    `NUM_BUCKETS` = the number of buckets to test (the number of splits to test along each
    coordinate axis) when creating every `BVHTreeNode`. */
    const size_t MAX_PRIMITIVES_IN_NODE, NUM_BUCKETS;
    /* `total_bvhnodes` = the number of `BVHTreeNode`s in this `BVH`. This is computed during
    `build_bvh_tree`. */
    size_t total_bvhnodes = 0;
    /* `linear_bvh_nodes` contains the flattened representation of the BVH Tree. Using this
    rather than a tree structure "improves cache, memory, and thus overall system performance".
    Specifically, `linear_bvh_nodes` holds the nodes of the BVH tree (all converted to
    `LinearBVHNode`s) in preorder. */
    std::vector<LinearBVHNode> linear_bvh_nodes;

    /* Build a BVH tree over the `Hittable` primitives specified by `curr_primitives`.
    Specifically, this returns the `root` of a binary tree representing the BVH built
    over all the primitives in `curr_primitives`. Each node in the tree has type
    `std::unique_ptr<BVHTreeNode>`. */
    auto build_bvh_tree(std::span<std::shared_ptr<Hittable>> curr_primitives) {
        ++total_bvhnodes;

        /* Compute the bounds for this `BVHTreeNode`'s primitives. We will eventually
        `std::move` this into one of the named constructors for `BVHTreeNode` (either
        `BVHTreeNode::leaf_node()` or `BVHTreeNode::interior_node()`). */
        auto curr_bounds = AABB::empty();
        for (const auto &primitive : curr_primitives) {
            curr_bounds.merge_with(primitive->get_aabb());
        }

        /* If there is only one primitive left, then we will return a leaf `BVHTreeNode`
        (a `BVHTreeNode` whose children are both primitives) that just contains that one primitive. */
        if (curr_primitives.size() == 1) {
            return BVHTreeNode::leaf_node(curr_primitives, std::move(curr_bounds));
        }

        /* Compute `centroids_bounds`, the AABB for the centroids of all the AABBs
        of the primitives in `curr_primitives`. */
        auto centroids_bounds = AABB::empty();
        for (const auto &primitive : curr_primitives) {
            centroids_bounds.merge_with(primitive->get_aabb().centroid());
        }

        /* The minimum split "cost" (explained below), the minimum-cost bucket (explained below)
        at which to split, and the axis along which the minimum-cost bucket splits by. */
        auto min_split_cost = std::numeric_limits<double>::infinity();
        size_t optimal_split_bucket;
        uint8_t optimal_split_axis = 0;
        /* ^^ Note: we initialize `optimal_split_axis` to silence a `-Wmaybe-uninitialized` warning
        in GCC. The compiler mistakenly thinks that in `BVHTreeNode::interior_node`, we will pass
        and subsequently use `optimal_split_axis` while it is uninitialized. This is impossible,
        because the only way `optimal_split_axis` would have been left uninitialized is when all
        iterations of the main loop are skipped (see the first comment in the main loop for when
        this would happen). But in that case, we would immediately return due to the check of
        `std::isinf(min_split_cost)`, so `optimal_split_axis` is in fact always initialized. */
        
        /* Test splits along each axis in turn. This is the main loop. */
        for (uint8_t axis = 0; axis < 3; ++axis) {

            /* When all the primitives' centroids have the same coordinate along the current axis,
            then centroids_bounds[axis].size() will equal 0, and so the calculation for `offset` below
            will result in floating-point divisions by 0. To avoid this, we just skip any such axis
            where `centroids_bounds[axis].size()` is 0. To avoid a compiler warning about using `==`
            with floating-points, we don't use `centroids_bounds[axis].size() == 0`, and instead use
            the standard library function `std::fpclassify` to check for equality with 0. */
            if (centroids_bounds[axis].is_empty()) {
                continue;
            }

            /* ...divide `centroids_bounds` into `NUM_BUCKETS` equally-sized regions along the axis,
            and for each region, compute its `BucketInfo`: the number of primitives whose centroids
            lie inside that region, and the AABB of those centroids that lie inside the region. */
            std::vector<BucketInfo> buckets(NUM_BUCKETS);
            /* To do this, iterate through all the primitives being considered. For each, determine
            which bucket (region) it lies in, and update that bucket correspondingly. */
            for (auto &i : curr_primitives) {

                /* Compute the bucket in which the centroid of the object `i` lies.
                `offset` = the ratio of*/
                auto offset = (i->get_aabb().centroid()[axis] - centroids_bounds[axis].min)
                             / centroids_bounds[axis].size();
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

            /* Update `min_split_cost`, `optimal_split_bucket`, and `optimal_split_axis` with all the
            buckets we tested along the current `axis` */
            for (size_t i = 0; i < NUM_BUCKETS - 1; ++i) {
                /* Again, we are looking for the split that leads to the minimum cost. */
                if (costs[i] < min_split_cost) {
                    min_split_cost = costs[i];
                    optimal_split_bucket = i;
                    optimal_split_axis = axis;
                }
            }
        }

        /* Edge case: When every single primitive's AABB's centroid is the same exact point, then
        we will just create a leaf node that contains all of `objects`, because "none of the
        splitting methods [learned] are effective in that (unusual) case" (from PBR 4th edition:
        see https://tinyurl.com/mpv4f4px). We can detect this edge case by checking if
        `min_split_cost` is equal to `std::numeric_limits<double>::infinity()`, because that means
        `min_split_cost` was never set throughout the above for-loop across all the axes, which
        only happens when every axes' interval in `centroids_bounds` is empty (because then we
        would have `continue`d on every loop iteration). Thus, this check suffices.
        
        As for why we don't just make `min_split_cost` a `std::optional<double>`, it's so when
        we do set `min_split_cost`, we can just check `costs[i] < min_split_cost` without
        having to write separate logic for when the `std::optional<double>` object is empty. */
        if (std::isinf(min_split_cost)) {
            return BVHTreeNode::leaf_node(curr_primitives, std::move(curr_bounds));
        }

        /* We either split the current object set into two sets, or we just make the current objects
        set its own leaf. Again, by our simplifying assumptions, the "cost" of just making the
        current objects set its own leaf is equal to the number of objects in the objects set; that
        is, objects.size(). */
        auto leaf_cost = static_cast<double>(curr_primitives.size());

        /* If the leaf cost is more than the cost of the minimum split we found, or if
        there are too many primitives left to make a single leaf node (as determined by
        the constant `MAX_PRIMITIVES_IN_NODE`), then we will choose to use the minimum-cost
        split that we found. This means we will choose to make the current node an
        INTERIOR (non-leaf) BVH node. */
        if (curr_primitives.size() > MAX_PRIMITIVES_IN_NODE || min_split_cost < leaf_cost) {

            /* Split `objects` into two sets, the first containing all objects that fall into bucket
            at most `optimal_split_bucket` along the axis `optimal_split_axis`, and the second containing
            all other objects. To do this, we use `std::partition` on `objects`. */
            auto mid = std::partition(curr_primitives.begin(), curr_primitives.end(),
            [&](const std::shared_ptr<Hittable> &object) {
                /* Copied from above; determine which bucket the current `object`'s centroid lies
                in */
                auto offset = (object->get_aabb().centroid()[optimal_split_axis]
                             - centroids_bounds[optimal_split_axis].min)
                             / centroids_bounds[optimal_split_axis].size();
                auto curr_bucket = static_cast<size_t>(static_cast<double>(NUM_BUCKETS) * offset);
                if (curr_bucket == NUM_BUCKETS) {  /* In case `offset` = 1; see previously */
                    --curr_bucket;
                }

                /* Split objects by whether or not their bucket along the axis `optimal_split_axis`
                is at most `optimal_split_bucket`. */
                return (curr_bucket <= optimal_split_bucket);
            }) - curr_primitives.begin();

            /* Recursively build the left and right children of the current node. The left and right
            children are each built over one of the two sets that partition `curr_primitives`.
            Specifically, the left child (`left_child`) will be built over the primitives to the
            left of the partition (that means over the primitives with coordinates along the
            axis `optimal_split_axis` that fall in buckets at most `optimal_split_bucket`; the
            primitives with axis coordinates at most the optimal coordinate split threshold),
            and the right child (`right_child`) will be built over the primitives to the right
            of the partition. */
            auto left_child = build_bvh_tree(curr_primitives.subspan(0, mid));  /* `std::span::subspan`! */
            auto right_child = build_bvh_tree(curr_primitives.subspan(mid));

            /* Again, in this case, the current node will be an interior node. */
            return BVHTreeNode::interior_node(std::move(left_child), std::move(right_child),
                                              std::move(curr_bounds), optimal_split_axis);
        } else {
            /* In this case, we know that simply grouping all the current remaining primitives
            into a leaf node not only costs less (with cost determined by the Surface Area
            heuristic), but that there are a small enough number of primitives (at most
            `MAX_PRIMITIVES_IN_NODE` primitives) to put them all into a single leaf node.
            So, in this case, we will just return a leaf node over all the current remaining
            primitives. */
            return BVHTreeNode::leaf_node(curr_primitives, std::move(curr_bounds));
        }
    }
    
    /* Flattens the BVH tree rooted at `tree_node`. Specifically, this converts each
    `BVHTreeNode` in the BVH (sub)tree rooted at `tree_node` to a `LinearBVHNode`, and
    writes it in preorder starting from `linear_bvh_nodes[next_index]`. This also
    consumes said BVH tree, freeing all associated memory, leaving only the flattened
    representation at the end. */
    void flatten_bvh_tree(std::unique_ptr<BVHTreeNode> tree_node, size_t &next_index) {

        /* This tree node will be placed into the next available index in `linear_bvh_nodes`. */
        auto curr_index = next_index++;
        auto &curr_linear_bvh_node = linear_bvh_nodes[curr_index];

        /* Note that we are guaranteed that `tree_node != nullptr`, because we always `return`
        upon reaching a leaf `BVHTreeNode`, so we will never reach a nullptr when calling
        this function. That is, `flatten_bvh_tree` will never be called with `tree_node
        = nullptr` given that it was initially called with `curr_node` equal to a node in a
        valid BVH tree. */

        /* Convert the current `BVHTreeNode` to a corresponding `LinearBVHNode`. */
        if (tree_node->is_leaf_node()) {
            /* If the original tree node is a leaf node, then its corresponding `LinearBVHNode`
            will be a leaf node as well. For leaf `LinearBVHNode`s, we need to set the `aabb`,
            `first_primitive_index`, and `num_primitives` fields. */
            curr_linear_bvh_node = LinearBVHNode {
                /* Take the AABB from the tree node */
                .aabb = std::move(tree_node->aabb),
                /* Leaf `LinearBVHNode`s need to know the index of the first primitive
                in `primitives` that they contain. We can recover this information from
                the `primitives` field of the original tree node. */
                .first_primitive_index = static_cast<size_t>(
                    /* Recover the actual index of the first element of the `std::span` of
                    primitives in the original `primitives` array by simply doing subtraction
                    on the raw pointers returned from `data()`. Learned from
                    https://stackoverflow.com/a/77671734/12597781. */
                    tree_node->primitives.data() - primitives.data()
                ),
                /* Also recover the number of primitives contained in this leaf node from the
                `std::span` of primitives stored in the tree node. */
                .num_primitives = tree_node->primitives.size(),
                /* While leaf `LinearBVHNode`s do not have a split axis (leaf nodes, by definition,
                were not split further), leaving out `split_axis` in this aggregate initialization
                leads to compiler warnings. We set `split_axis` to something to silence compiler
                warnings. */
                .split_axis = 0
            };
        } else {
            /* If the original tree node is an interior node, then its corresponding `LinearBVHNode`
            will be an interior node as well. */

            /* First, recurse and finish flattening all nodes in the left subtree of the
            current tree node. */
            flatten_bvh_tree(std::move(tree_node->left_child), next_index);

            /* For interior `LinearBVHNode`s, we need to set the `aabb`, `second_child_index`,
            and `split_axis` fields (and we nee to set `num_primitives` to 0). */
            curr_linear_bvh_node = LinearBVHNode {
                /* Take the AABB from the tree node */
                .aabb = std::move(tree_node->aabb),
                /* Because we just finished flattening all the nodes in the left subtree of
                this tree node, we know that this node's right child's index will just be the
                next index available to any node (because that will be the next node we visit). */
                .second_child_index = next_index,
                /* Remember that we set `num_primitives` to 0 for interior `LinearBVHNode`s
                because this allows us to distinguish leaf `LinearBVHNode`s (which have
                a strictly positive `num_primitives` value) from interior `LinearBVHNode`s. */
                .num_primitives = 0,
                /* Finally, record `split_axis`; just copy it over from the tree node. */
                .split_axis = tree_node->split_axis
            };

            /* Now, finish flattening the BVH tree by recursing to the right child of the
            current BVH tree node. */
            flatten_bvh_tree(std::move(tree_node->right_child), next_index);
        }
    }

    /* Flattens the BVH tree constructed in `build_bvh_tree` (rooted at `tree_root`), and
    stores the result in `linear_bvh_nodes`. This consumes the BVH tree in the result,
    freeing all associated memory.  */
    auto flatten_bvh_tree(std::unique_ptr<BVHTreeNode> tree_root) {
        /* Because we computed `total_bvhnodes`, we are able to allocate the exact
        number of `LinearBVHNode`s we need to store. */
        linear_bvh_nodes.resize(total_bvhnodes);

        /* Start flattening the BVH tree to `linear_bvh_nodes`. As stated above, this also
        consumes the given BVH tree. */
        size_t next_index = 0;
        return flatten_bvh_tree(std::move(tree_root), next_index);
    }

    void print_as_tree(std::ostream &os, size_t depth = 0, size_t curr_node_index = 0) const {
        const auto &curr_node = linear_bvh_nodes[curr_node_index];

        /* Prefix the current BVH node with `depth` spaces to show levels more clearly */
        std::string spaces(depth, ' ');

        if (curr_node.is_leaf_node()) {
            /* For leaf nodes, just print out all the contained primitives on separate lines */
            os << spaces << "Leaf BVH Node with " << curr_node.num_primitives
               << " primitives {\n";

            for (
                size_t i = curr_node.first_primitive_index;
                i < curr_node.first_primitive_index + curr_node.num_primitives;
                ++i
            ) {
                /* Indent all primitives by `space` + 1 spaces */
                os << spaces << " " << *primitives[i] << '\n';
            }

            os << spaces << "}\n";
        } else {
            /* For interior nodes recursively print the left and then the right child */
            os << spaces << "Interior BVH Node with left and right children {\n";
            print_as_tree(os, depth + 1, curr_node_index + 1);
            print_as_tree(os, depth + 1, curr_node.second_child_index);
        }
    }

public:

    /* Returns a `hit_info` with information about the earliest intersection of the ray `ray` with
    any primitive in this `BVH`, in the time interval `ray_times`, if any. */
    std::optional<hit_info> hit_by(const Ray3D &ray, const Interval &ray_times_) const override {
        std::optional<hit_info> result;
        
        /* The algorithm is a recursive DFS, but we perform it iteratively to reduce various
        sources of overhead from function calls.
        
        `dfs_callstack` represents our call stack in the DFS. The size of `dfs_callstack` must
        be at least the depth of any BVH tree. From my experimentation, I have never found a
        BVH with depth greater than 128 (the maximum depth I have found so far is 116). */
        std::array<size_t, 128> dfs_callstack;
        /* At each step in the `while`-loop below, `stack_next_index` represents the next
        available index in `dfs_callstack`, and `curr_node_index` equals the index of the
        current `LinearBVHNode` in `linear_bvh_nodes` that is being visited. `curr_node_index`
        starts at 0, because we always start at the root of of the BVH tree, which is the
        first `LinearBVHNode` in `linear_bvh_nodes`. */
        size_t stack_next_index = 0, curr_node_index = 0;

        /* Precompute information that will be passed to `AABB::is_hit_by_optimized()`. */
        /* `inv_ray_dir` = The inverse of the given ray's direction vector. */
        auto inv_ray_dir = Vec3D{1 / ray.dir.x, 1 / ray.dir.y, 1 / ray.dir.z};
        /* `dir_is_negative[i]` tells us if the `i`th component of `ray.dir` is negative. */
        std::array<bool, 3> dir_is_negative{
            ray.dir.x < 0,
            ray.dir.y < 0,
            ray.dir.z < 0
        };

        /* `ray_times` is a modifiable copy of `ray_times_`. We need this because we will
        update `ray_times.max` as we find earlier and earlier intersections. */
        auto ray_times = ray_times_;

        /* (Iteratively) DFS starting from the root of the BVH. */
        while (true) {
            const auto &curr_node = linear_bvh_nodes[curr_node_index];

            /* At each node, the first step is to check if the ray hits the node's AABB in
            the time interval `ray_times`. If it does not, then we immediately know that
            the ray hits no primitive contained within this node, so we can immediately
            "return" from this call to the DFS (pop the DFS call stack and continue with
            the previous node). This is the key idea behind using BVHs to achieve
            sublinear-time intersection tests. */
            if (curr_node.aabb.is_hit_by_optimized(ray, ray_times, inv_ray_dir, dir_is_negative)) {
                if (curr_node.is_leaf_node()) {
                    /* If the current `LinearBVHNode` is a leaf node, and its bounding box is
                    intersected by the given ray, then we will just need to test the ray against
                    every primitive contained in this leaf node. */

                    /* Enumerate all primitives contained by this leaf node. Remember, that
                    is the contiguous range in the `primitives` array starting at index
                    `curr_node.first_primitive_index` and with size `curr_node.num_primitives`. */
                    for (
                        size_t i = curr_node.first_primitive_index;
                        i < curr_node.first_primitive_index + curr_node.num_primitives;
                        ++i
                    ) {
                        if (auto curr = primitives[i]->hit_by(ray, ray_times); curr) {
                            /* Update `result` if an new earliest intersection is found */
                            result = curr;
                            /* We also update `ray_times.max` to the new earliest intersection
                            time. To understand why we do this, observe that the intersection test
                            with each `LinearBVHNode`'s bounding box uses `ray_times` as its time
                            interval. Thus, by shrinking `ray_times` whenever possible, we
                            potentially decrease the number of BVH nodes that have their AABB
                            hit by the ray, potentially decreasing the number of BVH nodes we
                            visit. */
                            ray_times.max = curr->hit_time;
                        }
                    }

                    /* Now that we've finished testing this leaf node, we need to return to the
                    previous node (this corresponds to a literal `return` statement in a recursive
                    DFS implementation of this algorithm). This is done by setting
                    `curr_node_index` to the index at the top of the DFS call stack, and then
                    popping the call stack. On the other hand, if the DFS call stack is empty
                    (if `stack_next_index` is 0), that means traversal is complete, and so we
                    `break` from the loop. */
                    if (stack_next_index == 0) {break;}
                    curr_node_index = dfs_callstack[--stack_next_index];
                } else {
                    /* If the current `LinearBVHNode` is an interior node, and its bounding box
                    is intersected by the given ray, then we will need to recursively visit both
                    of its children to perform ray-intersection tests on them. */

                    /* Observe that it is desirable to visit the first child that the ray passes
                    through before visiting the other child, because if the ray happens to have
                    an intersection in the closer child, then `ray_times.max` will have been
                    decreased as much as possible when we do eventually visit the further child.
                    This increases the probability that the further child's AABB will not be
                    hit by the ray in its time interval, reducing the total number of BVH nodes
                    visited.
                    
                    Now, how do we determine which child the ray passes through first? This
                    is where the `split_axis` field comes in handy. Remember that when building
                    the BVH tree, we always put the objects with larger coordinates along the
                    split axis in the right child. As a result, if the ray's direction along the
                    split axis is negative, then we should visit the child with larger coordinates
                    along that axis first; that is, we should visit the right child first if
                    `dir_is_negative[curr_node.split_axis]`. Otherwise, if the ray's direction
                    along the split axis is positive, then we should visit the left child first. */
                    if (dir_is_negative[curr_node.split_axis]) {
                        /* Visit the right child first, and push the left child onto the DFS call
                        stack. That is, set `curr_node_index` to the second child index of
                        `curr_node`, and push the first child's index (which, as you remember,
                        is just the current node's index plus one) ont onto the DFS call stack. */
                        dfs_callstack[stack_next_index++] = curr_node_index + 1;
                        curr_node_index = curr_node.second_child_index;  /* Visit right child */
                    } else {
                        /* Otherwise, visit the left child first, and push the right child onto
                        the DFS call stack. That is, set `curr_node_index` to the first child
                        index of `curr_node` (which, as you remember, is just its own index plus
                        one), and push the second child's index onto the DFS call stack. */
                        dfs_callstack[stack_next_index++] = curr_node.second_child_index;
                        curr_node_index = curr_node_index + 1;  /* Visit left child */
                    }
                }
            } else {
                /* If the ray didn't even hit the bounding box for this `LinearBVHNode`, then we
                know it will certainly not hit any primitive contained within this `LinearBVHNode`,
                and so we can immediately return to the previous node (this corresponds to a literal
                `return` statement in a recursive DFS implementation of this algorithm). As above,
                this is done by setting `curr_node_index` to the index at the top of the DFS call
                stack, and then popping the call stack. On the other hand, if the DFS call stack
                is empty (if `stack_next_index` is 0), that means traversal is complete, and so we
                `break` from the loop. */
                if (stack_next_index == 0) {break;}
                curr_node_index = dfs_callstack[--stack_next_index];
            }
        }

        return result;
    }

    /* Returns the AABB for this `BVH`. */
    AABB get_aabb() const override {
        /* A `BVH`'s AABB is equivalent to the BVH's root's AABB. Because our construction methods
        guarantee that the first node in `linear_bvh_nodes` is the root, it suffices to return the
        AABB for `linear_bvh_nodes.front()`. */
        return linear_bvh_nodes.front().aabb;
    }

    /* Prints this `BVH` to the `std::ostream` specified by `os`. */
    void print_to(std::ostream &os) const override {
        print_as_tree(os);
        os << std::flush;
    }

    /* @brief Builds a BVH (Bounding Volume Hierarchy) over the primitive components of the
    `Hittable` object specified by `world`. `num_buckets` splits will be tested along each
    of the x-, y-, and z- axes to find the optimal split. Each leaf `BVHTreeNode` in the
    tree structure of this `BVH` will be disallowed from containing more than
    `max_primitives_in_node` primitives.
    
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
    default to insertion sort when the array is small. Another consideration is storage; the
    larger `max_primitives_in_node` is, the less `BVHTreeNode`s will be needed. */
    template<typename T>
    requires std::is_base_of_v<Hittable, T>
    BVH(const T &world, size_t num_buckets = 32, size_t max_primitives_in_node = 12)
        : primitives{std::move(world.get_primitive_components())},
          /* ^^ Build the Bounding Volume Hierarchy over the primitive components of the `Hittable`
          objects in the scene, rather than just the objects themselves. This is because
          `Hittable` objects may be compound; they may contain other `Hittable`s. Building a
          `BVH` over just the `Hittable` objects themselves could therefore*/
          MAX_PRIMITIVES_IN_NODE{max_primitives_in_node},
          NUM_BUCKETS{num_buckets}
    {
        std::cout << "Building BVH over " << world.size() << " objects ("
                  << primitives.size() << " primitives)..." << std::endl;

        auto start = std::chrono::steady_clock::now();

        /* Build the BVH tree, then flatten it into an array (which consumes the
        BVH tree in the process, so only the array representation is left at the end). */
        flatten_bvh_tree(build_bvh_tree(primitives));

        std::cout << "Constructed BVH in "
                  << ms_diff(start, std::chrono::steady_clock::now())
                  << "ms (created " << total_bvhnodes << " BVHNodes total)\n" << std::endl;
    }
};

#endif