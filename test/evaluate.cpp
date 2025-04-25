#include <chrono>
#include <omp.h>
#include <print>
#include <sstream>
#include <vector>
#include <wait_free_bag.hpp>

void lock_based_insert(auto& vec, const std::size_t num_threads, const std::size_t elements_per_thread)
{
	#pragma omp parallel for
        for(std::size_t i = 0; i < num_threads; i++)
        {
                for(std::size_t j = 0; j < elements_per_thread; j++)
                {
	                #pragma omp critical
                        vec.push_back((i * elements_per_thread) + j);
                }
        }
}

void wait_free_insert(auto& bag, const std::size_t num_threads, const std::size_t elements_per_thread)
{
	#pragma omp parallel for
        for(std::size_t i = 0; i < num_threads; i++)
        {
                for(std::size_t j = 0; j < elements_per_thread; j++) bag.insert((i * elements_per_thread) + j);
        }
}

void lock_based_for_all(auto& vec)
{
        auto foo = [](std::size_t& value)
        {
                value = value * -1;
        };

	#pragma omp parallel for
        for(std::size_t& e: vec) foo(e);
}

void wait_free_for_all(auto& bag)
{
        auto foo = [](std::size_t& value)
        {
                value = value * -1;
        };

	#pragma omp parallel
        bag.for_all(foo);
}

void lock_based_extract(auto& vec)
{
	#pragma omp parallel for
        for(std::size_t i = 0; static_cast<decltype(vec.size())>(i) < vec.size(); i++)
        {
                while(vec.size() > 0)
                {
	                #pragma omp critical
                        if(vec.size() > 0) vec.pop_back();
                }
        }
}

void wait_free_extract(auto& bag, const std::size_t num_threads)
{
	#pragma omp parallel for
        for(std::size_t i = 0; i < num_threads; i++)
        {
                while(bag.size() > 0) bag.extract();
        }
}

int main(int argc, char** argv)
{
        if(argc <= 1)
        {
                std::println("[ERROR] Incorrect usage...");
                std::println("Usage: evaluate <number of elements>");
                std::exit(-1);
        }

        std::size_t num_threads = 0;
	#pragma omp parallel master
        num_threads = omp_get_num_threads();

        std::stringstream ss(argv[1]);
        std::size_t       num_elements = 0;
        ss >> num_elements;
        const std::size_t elements_per_thread = num_elements / num_threads;

        wait_free_bag::WaitFreeBag<std::size_t, 16> bag;
        std::vector<std::size_t>                    vec;

        const auto tp0 = std::chrono::high_resolution_clock::now();
        lock_based_insert(vec, num_threads, elements_per_thread);
        const auto tp1 = std::chrono::high_resolution_clock::now();
        wait_free_insert(bag, num_threads, elements_per_thread);
        const auto tp2 = std::chrono::high_resolution_clock::now();
        lock_based_for_all(vec);
        const auto tp3 = std::chrono::high_resolution_clock::now();
        wait_free_for_all(bag);
        const auto tp4 = std::chrono::high_resolution_clock::now();
        lock_based_extract(vec);
        const auto tp5 = std::chrono::high_resolution_clock::now();
        wait_free_extract(bag, num_threads);
        const auto tp6 = std::chrono::high_resolution_clock::now();

        const std::chrono::duration<double> lock_based_insert_time  = tp1 - tp0;
        const std::chrono::duration<double> wait_free_insert_time   = tp2 - tp1;
        const std::chrono::duration<double> lock_based_for_all_time = tp3 - tp2;
        const std::chrono::duration<double> wait_free_for_all_time  = tp4 - tp3;
        const std::chrono::duration<double> lock_based_extract_time = tp5 - tp4;
        const std::chrono::duration<double> wait_free_extract_time  = tp6 - tp5;

        std::println("{},{},{},{},{},{},{},{},{},{},{},{}",
                     num_threads,
                     num_elements,
                     elements_per_thread,
                     lock_based_insert_time.count(),
                     wait_free_insert_time.count(),
                     lock_based_for_all_time.count(),
                     wait_free_for_all_time.count(),
                     lock_based_extract_time.count(),
                     wait_free_extract_time.count(),
                     lock_based_insert_time / wait_free_insert_time,
                     lock_based_for_all_time / wait_free_for_all_time,
                     lock_based_extract_time / wait_free_extract_time);
}
