#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <string>
#include <chrono>
#include <array>
#include <utility>

template<typename T>
auto seconds_diff(const std::chrono::time_point<T> &start, const std::chrono::time_point<T> &end) {
    return std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
}

template<typename T>
auto ms_diff(const std::chrono::time_point<T> &start, const std::chrono::time_point<T> &end) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

/* Converts `seconds` seconds into DHMS (days, hours, minutes, seconds), in the readable
format "_d _hr _min _s". If a quantity is 0, then its corresponding time unit is omitted
(so 1 hour and 5 seconds would just be "1hr 5s", not "1hr 0min 5s"), for instance. */
auto seconds_to_dhms(unsigned long long seconds) {
    const static std::array<std::pair<unsigned, std::string>, 4> conv{{
        {86400, "d"}, {3600, "hr"}, {60, "min"}, {1, "s"}
    }};

    std::string ret;
    for (const auto &[factor, time_unit] : conv) {
        if (seconds >= factor) {
            auto num = seconds / factor;
            ret += std::to_string(num) + time_unit + " ";
            seconds %= factor;
        }
    }

    /* If seconds is 0 then ret is blank. We make ret = "0s" in that case. */
    if (ret.empty()) {
        ret = "0s";
    } else if (ret.back() == ' ') {
        ret.pop_back();
    }

    return ret;
}

#endif