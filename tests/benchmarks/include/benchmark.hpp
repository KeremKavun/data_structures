#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <chrono>
#include <string>
#include <iostream>
#include <iomanip>
#include <vector>
#include <functional>

class BenchmarkTimer {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = std::chrono::nanoseconds;

    void start() {
        start_time = Clock::now();
    }

    void stop() {
        end_time = Clock::now();
    }

    double elapsed_ms() const {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);
        return duration.count() / 1000.0;
    }

    double elapsed_us() const {
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end_time - start_time);
        return duration.count();
    }

private:
    TimePoint start_time;
    TimePoint end_time;
};

struct BenchmarkResult {
    std::string name;
    double avl_time_ms;
    double map_time_ms;
    double speedup;
    size_t operations;
    
    void print() const {
        std::cout << std::left << std::setw(35) << name 
                  << std::right << std::setw(12) << std::fixed << std::setprecision(3) 
                  << avl_time_ms << " ms"
                  << std::setw(12) << map_time_ms << " ms"
                  << std::setw(10) << std::setprecision(2) << speedup << "x";
        
        if (speedup > 1.0) {
            std::cout << " (AVL faster)";
        } else if (speedup < 1.0) {
            std::cout << " (map faster)";
        }
        std::cout << std::endl;
    }
};

class BenchmarkSuite {
public:
    void add_result(const BenchmarkResult& result) {
        results.push_back(result);
    }

    void print_summary() const {
        std::cout << "\n" << std::string(80, '=') << std::endl;
        std::cout << "BENCHMARK RESULTS SUMMARY" << std::endl;
        std::cout << std::string(80, '=') << std::endl;
        std::cout << std::left << std::setw(35) << "Test Name" 
                  << std::right << std::setw(15) << "AVL Time"
                  << std::setw(15) << "map Time"
                  << std::setw(15) << "Speedup" << std::endl;
        std::cout << std::string(80, '-') << std::endl;

        for (const auto& result : results) {
            result.print();
        }

        std::cout << std::string(80, '=') << std::endl;
    }

private:
    std::vector<BenchmarkResult> results;
};

// Helper macros
#define BENCHMARK_START(timer) timer.start()
#define BENCHMARK_STOP(timer) timer.stop()

#endif // BENCHMARK_H