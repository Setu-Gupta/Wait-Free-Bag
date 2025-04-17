#include <chrono>
#include <omp.h>
#include <print>
#include <vector>
#include <wait_free_bag.hpp>

const int THREADS             = 16;
const int ELEMENTS_PER_THREAD = 65536 * 32;

void lock_based_insert(auto& vec)
{
	#pragma omp parallel for
        for(int i = 0; i < THREADS; i++)
        {
                for(int j = 0; j < ELEMENTS_PER_THREAD; j++)
                {
	                #pragma omp critical
                        vec.push_back((i * ELEMENTS_PER_THREAD) + j);
                }
        }
}

void wait_free_insert(auto& bag)
{
	#pragma omp parallel for
        for(int i = 0; i < THREADS; i++)
        {
                for(int j = 0; j < ELEMENTS_PER_THREAD; j++) bag.insert((i * ELEMENTS_PER_THREAD) + j);
        }
}

void lock_based_for_all(auto& vec)
{
        auto foo = [](int& value)
        {
                value = value * -1;
        };

	#pragma omp parallel for
        for(int& e: vec) foo(e);
}

void wait_free_for_all(auto& bag)
{
        auto foo = [](int& value)
        {
                value = value * -1;
        };

	#pragma omp parallel
        bag.for_all(foo);
}

void lock_based_extract(auto& vec)
{
	#pragma omp parallel for
        for(int i = 0; static_cast<decltype(vec.size())>(i) < vec.size(); i++)
        {
                while(vec.size() > 0)
                {
	                #pragma omp critical
                        if(vec.size() > 0) vec.pop_back();
                }
        }
}

void wait_free_extract(auto& bag)
{
	#pragma omp parallel for
        for(int i = 0; i < THREADS; i++)
        {
                while(bag.size() > 0) bag.extract();
        }
}

int main()
{
        wait_free_bag::WaitFreeBag<int, THREADS> bag;
        std::vector<int>                         vec;

        const auto tp0 = std::chrono::high_resolution_clock::now();
        lock_based_insert(vec);
        const auto tp1 = std::chrono::high_resolution_clock::now();
        wait_free_insert(bag);
        const auto tp2 = std::chrono::high_resolution_clock::now();
        lock_based_for_all(vec);
        const auto tp3 = std::chrono::high_resolution_clock::now();
        wait_free_for_all(bag);
        const auto tp4 = std::chrono::high_resolution_clock::now();
        lock_based_extract(vec);
        const auto tp5 = std::chrono::high_resolution_clock::now();
        wait_free_extract(bag);
        const auto tp6 = std::chrono::high_resolution_clock::now();

        const std::chrono::duration<double> lock_based_insert_time  = tp1 - tp0;
        const std::chrono::duration<double> wait_free_insert_time   = tp2 - tp1;
        const std::chrono::duration<double> lock_based_for_all_time = tp3 - tp2;
        const std::chrono::duration<double> wait_free_for_all_time  = tp4 - tp3;
        const std::chrono::duration<double> lock_based_extract_time = tp5 - tp4;
        const std::chrono::duration<double> wait_free_extract_time  = tp6 - tp5;

        int num_threads = -1;
	#pragma omp parallel
        {
	        #pragma omp master
                num_threads = omp_get_num_threads();
        }

        std::println("{},{},{},{},{},{},{},{},{},{}",
                     num_threads,
                     lock_based_insert_time,
                     wait_free_insert_time,
                     lock_based_for_all_time,
                     wait_free_for_all_time,
                     lock_based_extract_time,
                     wait_free_extract_time,
                     lock_based_insert_time / wait_free_insert_time,
                     lock_based_for_all_time / wait_free_for_all_time,
                     lock_based_extract_time / wait_free_extract_time);
}
