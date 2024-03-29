#ifndef RAND_UTIL_H
#define RAND_UTIL_H

#include <iostream>
#include <random>
#include <optional>
#include <mutex>

/* `SeedSeqGenerator` is a singleton class whose sole instance generates the sequence of random
seeds which supplies random seeds to the `thread_local` RNGs used in the `rand_double` function. */
class SeedSeqGenerator {
    using seed_type = uint32_t;  /* Determines the LCG's period (it equals 2^WIDTH); read below */

    /* `custom_seed` = the unsigned integer seed for the sequence of seeds generated by this
    `SeedSeqGenerator`. If `custom_seed` is not explicitly set by the user, then a seed will
    later be automatically generated. */
    std::optional<seed_type> custom_seed;
    /* `generate_next_seed_mtx` is used to ensure that only one thread at a time can advance
    the seed sequence generated by this `SeedSeqGenerator`. */
    std::mutex generate_next_seed_mtx;

    /* Because `SeedSeqGenerator` is a singleton class, we make its default constructor private,
    but we do not delete it like we delete the copy constructor, copy assignment operator, move
    constructor, and move assignment operator. Making the default constructor private (and not
    declaring any other constructor) ensures this class may not be instantiated outside of this
    class, which is necessary for a singleton class. However, it is vital that we do not delete
    the default constructor, because this would mean that no instance of the class can be created
    at all, not even within the class! A singleton class needs one instance exactly, so it still
    needs to have SOME constructor defined for it. */
    SeedSeqGenerator() = default;

public:

    /* Returns the sole `SeedSeqGenerator` instance. */
    static auto& get_instance() {
        /* I use the Meyer Singleton to implement this Singleton class. */
        static SeedSeqGenerator seeds;
        return seeds;
    }

    /* Delete the copy constructor, copy assignment operator, move constructor, and
    move assignment operator. This ensures that no other instance of the `SeedSeqGenerator`
    class may be created (via copying the instance returned from `get_instance`), and also
    ensures that the instance returned by `get_instance()` will not be `std::move`d away. */
    SeedSeqGenerator(const SeedSeqGenerator&) = delete;
    SeedSeqGenerator& operator= (const SeedSeqGenerator&) = delete;
    SeedSeqGenerator(SeedSeqGenerator&&) = delete;
    SeedSeqGenerator& operator= (SeedSeqGenerator&&) = delete;

    /* Generates and returns the next random seed. */
    auto next_seed() {

        /* Ensure thread safety; allow only one thread to advance the state of this
        `SeedSeqGenerator` at a time. */
        std::lock_guard guard(generate_next_seed_mtx);

        /* If no seed was provided for this `SeedSeqGenerator`, use `std::random_device` to generate
        one, and notify the user. */
        if (!custom_seed) {
            custom_seed = std::random_device{}();
            std::cout << "SeedSeqGenerator: No random seed provided, using " << *custom_seed
                      << " (Use SeedSeqGenerator::get_instance().set_seed([custom seed]) "
                         "to set a custom seed)"
                      << std::endl;
        }

        /* The seed sequence is simply the output of a Linear Congruential Generator starting from
        `custom_seed`. See `rand_double` for a discussion about the importance of the specific
        constants chosen in LCGs. */
        custom_seed = 2'483'477 * (*custom_seed) + 2'987'434'823;
        return *custom_seed;
    }

    /* Seeds the seed generator with `seed`. */
    void set_seed(seed_type seed) {
        std::cout << "SeedSeqGenerator: Using user-provided random seed " << seed
                  << '\n' << std::endl;
        custom_seed = seed;
    }
};

/* Generates an uniformly-random `double` in the range [`min`, `max`]
(by default [0, 1]). Now trades quality for speed; we no longer use `<random>`
in favor of a Linear Congruential Generator. */
auto rand_double(double min = 0, double max = 1) {
    /* I use a Linear Congruential Generator to generates random integers, which I will then
    use to generate the random `double`s in the range [min, max].
    
    The LCG is defined by the recurrence relation X_{n + 1} = (A * X_n + C) % MOD, where X is the
    sequence of pseudorandom integers, X_0 is equal to the seed generated by the `SeedSeqGenerator`
    for this thread, A = 1664525, C = 1013904223, and MOD = 2^WIDTH, where the type `seed_type`
    of the generated seed is defined as `uintWIDTH_t` for some WIDTH.
    
    By the Hull-Dobell Theorem, this choice of A, C, and MOD guarantee that this LCG has period
    MOD (which is the maximum possible period length), no matter what X_0 we choose. Specifically,
    this is because the three conditions that (a) MOD and C are relatively prime, (b) A - 1 is
    divisible by all prime factors of M (which is just 2), and (c) 4 divides (A - 1) if 4 divides
    M, are all satisfied. See http://tinyurl.com/mwb8fwac.
    
    Note that the modulo 2^WIDTH operation is done implicitly, because the type of `seed` is
    `uintWIDTH_t` (so all arithmetic operations on `seed` are computed modulo 2^WIDTH; unsigned
    integers are awesome). This is a common trick, and is a big reason why many LCGs use a modulo
    which equals the word size (according to the Wikipedia page linked above). */

    /* Generate a new seed for this thread's LCG */
    thread_local auto seed = SeedSeqGenerator::get_instance().next_seed();
    seed = 1'664'525 * seed + 1'013'904'223;  /* The first random integer used is X_1, not X_0. */
    /* The LCG generates uniformly random INTEGERS from 0 to (MOD - 1), inclusive, where
    MOD = (1 << 32) here. To generate uniformly random `double`s in the range [min, max],
    it suffices to normalize the generated integer to the range [0, 1] (by dividing it by
    (MOD - 1)), and then using that as the linear interpolation parameter between `min`
    and `max` (so we will be returning min + (max - min) * (seed / (MOD - 1)). */
    constexpr auto SCALE = 1 / static_cast<double>(
        std::numeric_limits<decltype(seed)>::max() - 1  /* To handle different `seed_type`s */
    );
    return min + (max - min) * static_cast<double>(seed) * SCALE;
}

/* Generates an uniformly-random `int` in the range [`min`, `max`] ([0, 1] by default). */
auto rand_int(int min = 0, int max = 1) {
    /* Is just use a `std::mt19937` for now. If performance becomes an issue I'll switch to
    a LCG like I did with `rand_double()`. */
    thread_local std::mt19937 generator{SeedSeqGenerator::get_instance().next_seed()};
    thread_local std::uniform_int_distribution<> dist;
    dist.param(std::uniform_int_distribution<>::param_type{min, max});
    return dist(generator);
}

#endif