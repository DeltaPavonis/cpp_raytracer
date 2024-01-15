#ifndef PROGRESS_BAR_H
#define PROGRESS_BAR_H

#include <mutex>
#include "time_util.h"

/* ProgressBar displays a live progress bar for loops where the total number of iterations is
known beforehand. If DISABLE_PRINTING is true, then the progress bar will not print anything.
ProgressBar is thread-safe.

Usage:

```cpp
auto progress_bar = ProgressBar("Rendering", 100);
for (int i = 0; i < 100; ++i) {
    // Do stuff
    progress_bar.complete_iteration();
}
``` */
template <bool DISABLE_PRINTING = false>
class ProgressBar {
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = std::chrono::time_point<Clock>;

    size_t total_iterations, iterations_done;
    unsigned percent_done;
    unsigned downscale_factor; /* The progress bar will have length (100 / `downscale_factor`) */
    std::string description;   /* Description of task */
    TimePoint start_time;
    std::string progress_info; /* Printed after progress bar, contains time elapsed, % done, etc. */
    std::mutex update_progress_bar_mtx;

public:

    /* Increments the number of iterations done, and, if the next percent towards finishing has been
    achieved, also updates the progress bar and the estimated time left. Thread-safe. */
    void complete_iteration() {
        if constexpr (DISABLE_PRINTING) {
            return;
        }

        /* Allow only one thread to execute `update()` at a time. */
        std::lock_guard guard(update_progress_bar_mtx);

        ++iterations_done;

        /* Compute the current proportion and current percent of iterations completed */
        auto curr_proportion_done = static_cast<double>(iterations_done)
                                  / static_cast<double>(total_iterations);
        auto curr_percent_done = static_cast<unsigned>(100 * curr_proportion_done);

        /* Check if the next percent towards finishing has been achieved */
        if (curr_percent_done > percent_done) {

            /* Delete the progress information printed previously */
            for (size_t i = 0; i < progress_info.size(); ++i) {std::cout << "\b \b";}

            /* Update the progress bar with the necessary number of `#`s. When we are x% done, we will have
            printed (x / downscale_factor) `#`s, so we will add (curr_percent_done / downscale_factor) - 
            (percent_done / downscale_factor) `#`s. */
            std::cout << std::string(
                curr_percent_done / downscale_factor - percent_done / downscale_factor,
                '#'
            );

            /* Compute the estimated time left. To do this, we assume that the remaining iterations
            will be completed at the same average rate as the currently-finished iterations. Thus,
            the estimated time left is given by (time elapsed) * (1 - R) / R, where R is the current
            proportion of iterations completed. Note that we compute the seconds passed by dividing
            the milliseconds passed by 1000; this makes `seconds_left` more accurate. */
            auto seconds_passed = static_cast<double>(ms_diff(start_time, Clock::now())) / 1000;
            auto seconds_left = static_cast<long long>(
                seconds_passed * (1 - curr_proportion_done) / curr_proportion_done
            );

            /* Update `progress_info` with new information. `progress_info` includes the percentage
            towards finishing, the current time elapsed, and the estimated time left. */
            progress_info = " " + std::to_string(curr_percent_done) + "% done, "
                          + seconds_to_dhms(static_cast<long long>(seconds_passed)) + " elapsed, "
                          + seconds_to_dhms(seconds_left) + " left (est.)";
                
            /* As long as we are not done, print `progress_info` after the progress bar. */
            if (iterations_done != total_iterations) {
                std::cout << progress_info << std::flush;
            }

            /* Update `percent_done` */
            percent_done = curr_percent_done;
        }

        /* If completed, print a completion message with the total time elapsed */
        if (iterations_done == total_iterations) {
            std::cout << "\n" << description << ": Finished in "
                        << seconds_to_dhms(seconds_diff(start_time, Clock::now()))
                        << '\n'
                        << std::endl;
        }
    }

    /* Constructs a `ProgressBar` for the task described by `task_description_`, which requires
    `total_iterations_` iterations in total. The progress bar will be (100 / `downscale_factor_`)
    characters long; `downscale_factor_` is 2 by default, resulting in a 50-character-long progress
    bar. */
    ProgressBar(size_t total_iterations_, const std::string &task_description = "Progress",
                unsigned downscale_factor_ = 2 )
        : total_iterations{total_iterations_},
          iterations_done{0},
          percent_done{0},
          downscale_factor{downscale_factor_},
          description{task_description},
          start_time{Clock::now()}
    {
        if constexpr (!DISABLE_PRINTING) {
            std::cout << description << '\n';
            std::cout << '|' << std::string(100 / downscale_factor, ' ') << "|\n";
            std::cout << ' ' << std::flush;
        }
    }
};

#endif