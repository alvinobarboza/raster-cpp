#pragma once
#include <chrono>
#include <iostream>
#include <utility>

struct TimerSample {
    std::string name{};
    long long ms;
    long long us;
    long long ns;
};

class Timer {
    std::string t_name;
    std::chrono::time_point<std::chrono::steady_clock> start {};
    bool stopped {false};
    TimerSample t{};
public:
    explicit Timer(std::string name): t_name(std::move(name))
    {
        start = std::chrono::steady_clock::now();
    };

    ~Timer()
    {
        if (!stopped) stop();
    }

    void stop()
    {
        if (stopped) return;
        stopped = true;
        const auto end = std::chrono::steady_clock::now();
        const auto diff = end - start;
        const auto duration_milli = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
        const auto duration_micro = std::chrono::duration_cast<std::chrono::microseconds>(diff);
        const auto duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(diff);

        t.name = t_name;
        t.ms = duration_milli.count();
        t.us = duration_micro.count();
        t.ns = duration_nano.count();

        std::cout << t_name << ": \t" << duration_milli.count() << " ms ";
        std::cout << duration_micro.count() << " us ";
        std::cout << duration_nano.count() << " ns" << '\n';
    }

    TimerSample stop_and_sample()
    {
        if (stopped) return t;

        stopped = true;
        const auto end = std::chrono::steady_clock::now();
        const auto diff = end - start;
        const auto duration_milli = std::chrono::duration_cast<std::chrono::milliseconds>(diff);
        const auto duration_micro = std::chrono::duration_cast<std::chrono::microseconds>(diff);
        const auto duration_nano = std::chrono::duration_cast<std::chrono::nanoseconds>(diff);

        t.name = t_name;
        t.ms = duration_milli.count();
        t.us = duration_micro.count();
        t.ns = duration_nano.count();

        return t;
    }
};
