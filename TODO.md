# TODO

### Features
- [ ] Spatial and surface textures
- [ ] Instancing (This enables easy object translation and/or rotation)
- [ ] Volumes (smoke, fog, etc)
    - Idea: Procedurally generate volumes, so that they have a more natural shape. Look into Perlin noise.

### Optimizations
- [ ] Optimize BVH by compressing the binary tree to an array. This improves cache locality.
    - Don't forget to build the BVH over a copy of the primitives, as we already do. This is what they do in PBR.
- [ ] Use C++20 concepts to guarantee that `Camera::render()` uses compile-time polymorphism rather than runtime polymorphism and dynamic dispatch, when possible.

### Documentation
- [x] Add comments to the `aabb` fields of Sphere and Parallelogram.
