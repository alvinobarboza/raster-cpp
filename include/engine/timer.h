#pragma once
#include <chrono>
#include <iostream>
#include <utility>

class Timer {
    std::string t_name;
    std::chrono::time_point<std::chrono::steady_clock> start {};
public:
    explicit Timer(std::string  name): t_name(std::move(name))
    {
        start = std::chrono::steady_clock::now();
    };

    ~Timer()
    {
        const auto end = std::chrono::steady_clock::now();
        const auto diff = end - start;
        const auto duration_milli = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
        const auto duration_micro = std::chrono::duration_cast<std::chrono::microseconds>(diff);
        const auto duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(diff);
        std::cout << t_name << "\t: " << duration_milli.count() << " ms ";
        std::cout << duration_micro.count() << " us ";
        std::cout << duration_nano.count() << " ns" << std::endl;
    }
};
