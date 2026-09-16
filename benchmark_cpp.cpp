#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <numeric>

#include "bs_pricer.hpp"


struct OptionInput {
    double S;
    double K;
    double T;
    double r;
    double vol;
};


int main() {

    const int RUNS = 10;

    std::vector<OptionInput> options;


    // --------------------------------------------------
    // Load the exact same dataset used by Python
    // --------------------------------------------------

    std::ifstream file("benchmark_inputs.csv");

    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open benchmark_inputs.csv\n";
        return 1;
    }


    std::string line;

    while (std::getline(file, line)) {

        std::stringstream ss(line);
        std::string value;

        OptionInput option;

        std::getline(ss, value, ',');
        option.S = std::stod(value);

        std::getline(ss, value, ',');
        option.K = std::stod(value);

        std::getline(ss, value, ',');
        option.T = std::stod(value);

        std::getline(ss, value, ',');
        option.r = std::stod(value);

        std::getline(ss, value, ',');
        option.vol = std::stod(value);

        options.push_back(option);
    }

    file.close();


    const int N = options.size();


    if (N != 100000) {

        std::cerr << "ERROR: Expected 100000 options but found "
                  << N << "\n";

        return 1;
    }


    // --------------------------------------------------
    // Warm-up
    // --------------------------------------------------

    for (int i = 0; i < 1000; ++i) {

        const auto& option = options[i];

        bs_price(
            option.S,
            option.K,
            option.T,
            option.r,
            option.vol,
            "call"
        );
    }


    // --------------------------------------------------
    // Benchmark
    // --------------------------------------------------

    std::vector<double> times;

    double first_price = 0.0;
    double last_price = 0.0;


    for (int run = 0; run < RUNS; ++run) {

        auto start =
            std::chrono::high_resolution_clock::now();


        for (int i = 0; i < N; ++i) {

            const auto& option = options[i];

            PriceResult result =
                bs_price(
                    option.S,
                    option.K,
                    option.T,
                    option.r,
                    option.vol,
                    "call"
                );


            if (run == 0 && i == 0) {
                first_price = result.price;
            }

            if (run == 0 && i == N - 1) {
                last_price = result.price;
            }
        }


        auto end =
            std::chrono::high_resolution_clock::now();


        std::chrono::duration<double> elapsed =
            end - start;


        times.push_back(elapsed.count());
    }


    // --------------------------------------------------
    // Calculate statistics
    // --------------------------------------------------

    double mean_time =
        std::accumulate(
            times.begin(),
            times.end(),
            0.0
        ) / RUNS;


    std::vector<double> sorted_times = times;

    std::sort(
        sorted_times.begin(),
        sorted_times.end()
    );


    double median_time;

    if (RUNS % 2 == 0) {

        median_time =
            (sorted_times[RUNS / 2 - 1]
             + sorted_times[RUNS / 2]) / 2.0;

    } else {

        median_time =
            sorted_times[RUNS / 2];
    }


    double min_time = sorted_times.front();
    double max_time = sorted_times.back();


    double median_per_option =
        (median_time / N) * 1e6;


    // --------------------------------------------------
    // Results
    // --------------------------------------------------

    std::cout
        << std::fixed
        << std::setprecision(6);


    std::cout << "\n";
    std::cout << "========== C++ BENCHMARK ==========\n";
    std::cout << "\n";

    std::cout
        << "Number of prices: "
        << N << "\n";

    std::cout
        << "Number of runs: "
        << RUNS << "\n";

    std::cout << "\n";

    std::cout
        << "Mean time:   "
        << mean_time
        << " seconds\n";

    std::cout
        << "Median time: "
        << median_time
        << " seconds\n";

    std::cout
        << "Min time:    "
        << min_time
        << " seconds\n";

    std::cout
        << "Max time:    "
        << max_time
        << " seconds\n";

    std::cout << "\n";

    std::cout
        << std::setprecision(3)
        << "Median per option: "
        << median_per_option
        << " microseconds\n";

    std::cout << "\n";

    std::cout
        << std::setprecision(6)
        << "First price: "
        << first_price
        << "\n";

    std::cout
        << "Last price:  "
        << last_price
        << "\n";

    std::cout << "\n";
    std::cout << "====================================\n";


    return 0;
}