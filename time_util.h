#include <string>
#include <chrono>
#include <array>
#include <utility>

template<typename T>
auto seconds_diff(const std::chrono::time_point<T> &start, const std::chrono::time_point<T> &end) {
    return std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
}

/* Converts seconds into days/hours/minutes/seconds of the format
"[] days [] minutes [] hours [] seconds". If a quantity is 0 then
it and its corresponding time unit are left out. */
std::string seconds_to_dhms(long long seconds_left) {
    const static std::array<std::pair<unsigned, std::string>, 5> conv{{
        {604800, "week"}, {86400, "day"}, {3600, "hour"}, {60, "minute"}, {1, "second"}
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