#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <mutex>
#include "time_util.h"

/* ProgressBar displays a progress bar for time-costly for-loops
that have a number of iterations that is known beforehand (i.e.
the loop variable is updated by a constant amount each iteration).
If DISABLE_PRINTING is true, then the progress bar will not print.
ProgressBar is thread-safe.

Usage:

auto progress_bar = ProgressBar("Rendering", 100);
for (int i = 0; i < 100; ++i) {
    // Do stuff
    progress_bar.update();
}

Old usage: Deprecated because `#pragma omp parallel for` requires `for`-loops
to have "canonical form", which, among other things, requires the type of the
iterated object to be an integer, pointer, or random-access iterator type
(see https://www.openmp.org/wp-content/uploads/OpenMP-API-Specification-5-2.pdf,
specifically, the bottom of page 85, the bottom of page 87, and the top of page
88).
for (ProgressBar<int> row("Do stuff", 0, 100); row(); ++row) {
    // Do stuff, you can use `row` directly
}
This was much more convenient; the `ProgressBar` only needs to be alive inside the
`for`-loop, and it is. Also, you only have to write the beginning and end of the loop
(0 and 100 here) once.

*/
template <bool DISABLE_PRINTING = false>
class ProgressBar {
    using clock_type = std::chrono::high_resolution_clock;
    using instant = std::chrono::time_point<clock_type>;

    size_t iter_done, total_iter;
    unsigned last_percent, downscale_factor;
    std::string desc;
    instant start_time;
    std::string time_left;
    std::mutex mtx;

public:

    /* Increments the number of iterations done, and, if the next percent towards finishing has been
    achieved, also updates the progress bar and the estimated time left. Thread-safe. */
    void update() {
        std::lock_guard guard(mtx);  /* Ensure only one thread can execute `update()` at a time */

        ++iter_done;

        /* Check if the next percent towards finishing has been achieved */
        auto curr = static_cast<double>(iter_done) / static_cast<double>(total_iter);
        auto curr_percent = static_cast<unsigned>(100 * curr);

        if (curr_percent > last_percent) {
            /* Delete the last estimated time and add as many '#'s as needed */
            if constexpr (!DISABLE_PRINTING) {
                for (size_t i = 0; i < time_left.size(); ++i) {std::cout << "\b \b";}
                std::cout << std::string(curr_percent / downscale_factor
                                            - last_percent / downscale_factor, '#');
            }

            /* Compute estimated time left and print it on the same line.
            Estimated time left = (time passed so far) * (iterations left / iterations done)
            = (time passed so far) * (total_iter - iter_done) / iter_done
            = (time passed so far) * (1/curr - 1) */
            auto seconds_now = static_cast<double>(ms_diff(start_time, clock_type::now())) / 1000;
            auto seconds_left = static_cast<long long>(seconds_now * (1/curr - 1));

            time_left = " (Estimated time left: " + seconds_to_dhms(seconds_left) + ")";
            if constexpr (!DISABLE_PRINTING) {
                if (iter_done != total_iter) {
                    std::cout << time_left << std::flush;
                }
            }

            last_percent = curr_percent;
        }

        if constexpr (!DISABLE_PRINTING) {
            if (iter_done == total_iter) {
                std::cout << "\n" << desc << ": Finished in "
                          << seconds_to_dhms(seconds_diff(start_time, clock_type::now()))
                          << '\n'
                          << std::endl;
            }
        }
    }

    ProgressBar(size_t total_iterations, std::string description = "Progress",
                unsigned downscale_factor_ = 2)
        : iter_done{0}, total_iter{total_iterations}, last_percent{0},
          downscale_factor{downscale_factor_}, desc{description}, start_time{clock_type::now()}
    {
        if constexpr (!DISABLE_PRINTING) {
            std::cout << desc << '\n';
            std::cout << '|' << std::string(100 / downscale_factor, ' ') << "|\n";
            std::cout << ' ' << std::flush;
        }
    }
};

// template <typename T, bool DISABLE_PRINTING = false>
// struct ProgressBar {
//     using clock_type = std::chrono::high_resolution_clock;
//     using instant = std::chrono::time_point<clock_type>;

//     std::string desc, time_left; /* desc = name of task */
//     T curr, beg, end; /* Iterate through the range [beg, end) */
//     /* All ProgressBar's will use 100 / downscale_factor characters for the progress bar.
//     So, the larger downscale_factor is, the smaller the progress bar is. */
//     unsigned last_percent, downscale_factor;
//     instant start_time;

//     /* Allows convenient access to `curr` */
//     operator T&() {return curr;}
//     operator const T&() const {return curr;}

//     /* Updates/prints the progress bar, and returns `true` if the loop is NOT over */
//     bool operator() () {
//         unsigned curr_percent = static_cast<unsigned>(100 * (curr - beg) / (end - beg));
//         if (curr_percent > last_percent) {

//             /* Delete the last estimated time and add as many '#'s as needed */
//             if constexpr (!DISABLE_PRINTING) {
//                 for (size_t i = 0; i < time_left.size(); ++i) {std::cout << "\b \b";}
//                 std::cout << std::string(curr_percent / downscale_factor
//                                          - last_percent / downscale_factor, '#');
//             }

//             /* Compute new estimated time left and print it on the same line */
//             long long seconds_left = std::chrono::duration_cast<std::chrono::milliseconds>
//                                 (clock_type::now() - start_time).count() *
//                                 (double(100. - curr_percent) / curr_percent) / 1000.;

//             time_left = " (Estimated time left: " + seconds_to_dhms(seconds_left) + ")";
//             if constexpr (!DISABLE_PRINTING) {std::cout << time_left << std::flush;}

//             last_percent = curr_percent;
//         }

//         return curr < end;
//     }

//     ProgressBar(T beg_, T end_, const std::string &desc_ = "Progress",
//                 unsigned downscale_factor_ = 2)
//         : desc{desc_}, time_left{}, curr{beg_}, beg{beg_},
//         end{end_}, last_percent{0}, downscale_factor{downscale_factor_},
//         start_time{clock_type::now()}
//     {
//         if constexpr (!DISABLE_PRINTING) {
//             std::cout << desc << '\n';
//             std::cout << '|' << std::string(100 / downscale_factor, ' ') << "|\n";
//             std::cout << ' ' << std::flush;
//         }
//     }

//     ~ProgressBar() {
//         auto total_seconds = std::chrono::duration_cast<std::chrono::seconds>
//                              (clock_type::now() - start_time).count();
//         std::cout << "\n" << desc << ": Finished in "
//                   << seconds_to_dhms(total_seconds) << std::endl;
//     }
// };

#endif