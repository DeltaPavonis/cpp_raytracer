# TODO

### Design
- [ ] Consider major design overhaul of `Hittable`
    - Ideas: Let the user forgo using `std::make_shared<>` everywhere by making `Hittable` itself a tagged pointer (tagged to handle polymorphism); this is what is done in PBR.

### Features
- [ ] Spatial and surface textures
- [ ] Instancing (This enables easy object translation, rotation, and scaling)
- [ ] Volumes (smoke, fog, etc)
    - Idea: Procedurally generate volumes, so that they have a more natural shape. Look into Perlin noise.

### Optimizations
- [X] Optimize BVH by compressing the binary tree to an array. This improves cache locality.
    - Don't forget to build the BVH over a copy of the primitives, as we already do. This is what they do in PBR.
- [X] Use C++20 concepts to guarantee that `Camera::render()` and other related functions use compile-time polymorphism rather than runtime polymorphism and dynamic dispatch to handle calls to `hit_by`, when possible.
- [ ] Parallelize BVH build


### Documentation
- [x] Add comments to the `aabb` fields of Sphere and Parallelogram.
