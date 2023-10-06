#include <string>
#include <chrono>
#include <array>
#include <utility>

/* ProgressBar displays a progress bar for time-costly for-loops.
If DISABLE_PRINTING is true, then the progress bar will not print.

Usage:
for (ProgressBar<int> pb("Do stuff", 0, 100); pb(); ++pb) {
    // Do stuff
}
*/
template <typename T, bool DISABLE_PRINTING = false>
struct ProgressBar {
    using clock_type = std::chrono::high_resolution_clock;
    using instant = std::chrono::time_point<clock_type>;

    std::string desc, time_left; /* desc = name of task */
    T curr, beg, end; /* Iterate through the range [beg, end) */
    /* All ProgressBar's will use 100 / downscale_factor characters for the progress bar.
    So, the larger downscale_factor is, the smaller the progress bar is. */
    unsigned last_percent, downscale_factor;
    instant start_time;

    /* Allows convenient access to `curr` */
    operator T&() {return curr;}
    operator const T&() const {return curr;}

    /* Converts seconds into days/hours/minutes/seconds of the format
    "[] days [] minutes [] hours [] seconds". If a quantity is 0 then
    it and its corresponding time unit are left out. */
    std::string seconds_to_dhms(long long seconds_left) {
        const static std::array<std::pair<unsigned, std::string>, 4> conv{{
            {86400, "day"}, {3600, "hour"}, {60, "minute"}, {1, "second"}
        }};

        std::string ret;
        for (const auto &[factor, time_unit] : conv) {
            if (seconds_left >= factor) {
                auto num = seconds_left / factor;
                ret += std::to_string(num) + " " + time_unit + (num > 1 ? "s " : " ");
                seconds_left %= factor;
            }
        }

        /* If seconds_left is 0 then ret is blank. We make ret = "0 seconds" in that case */
        if (ret.empty()) {
            ret = "0 seconds";
        } else if (ret.back() == ' ') {
            ret.pop_back();
        }

        return ret;
    }

    /* Updates/prints the progress bar, and returns `true` if the loop is over */
    bool operator() () {
        unsigned curr_percent = static_cast<unsigned>(100 * (curr - beg) / (end - beg));
        if (curr_percent > last_percent) {

            /* Delete the last estimated time and add as many '#'s as needed */
            if constexpr (!DISABLE_PRINTING) {
                for (size_t i = 0; i < time_left.size(); ++i) {std::cout << "\b \b";}
                std::cout << std::string(curr_percent / downscale_factor
                                         - last_percent / downscale_factor, '#');
            }

            /* Compute new estimated time left and print it on the same line */
            long long seconds_left = std::chrono::duration_cast<std::chrono::milliseconds>
                                (clock_type::now() - start_time).count() *
                                (double(100. - curr_percent) / curr_percent) / 1000.;

            time_left = " (Estimated time left: " + seconds_to_dhms(seconds_left) + ")";
            if constexpr (!DISABLE_PRINTING) {std::cout << time_left << std::flush;}

            last_percent = curr_percent;
        }

        return curr < end;
    }

    ProgressBar(T beg_, T end_, const std::string &desc_ = "Progress",
                unsigned downscale_factor_ = 2)
        : desc{desc_}, time_left{}, curr{beg_}, beg{beg_},
        end{end_}, last_percent{0}, downscale_factor{downscale_factor_},
        start_time{clock_type::now()}
    {
        if constexpr (!DISABLE_PRINTING) {
            std::cout << desc << ": |" << std::string(100 / downscale_factor, ' ') << "|\n";
            std::cout << std::string(std::string(desc + ": |").size(), ' ') << std::flush;
        }
    }

    ~ProgressBar() {
        auto total_seconds = std::chrono::duration_cast<std::chrono::seconds>
                             (clock_type::now() - start_time).count();
        std::cout << "\n" << desc << ": Finished in "
                  << seconds_to_dhms(total_seconds) << std::endl;
    }
};