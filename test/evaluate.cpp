#include <chrono>
#include <iostream>
#include <omp.h>
#include <thread>
#include <vector>
#include <wait_free_bag.hpp>

void lock_based_insert(auto& vec)
{
        #pragma omp parallel for
        for(int i = 0; i < 1024; i++)
        {
                for(int j = 0; j < 1024; j++)
                {
                        #pragma omp critical
                        vec.push_back((i * 1024) + j);
                }
        }
}

void wait_free_insert(auto& bag)
{
        #pragma omp parallel for
        for(int i = 0; i < 1024; i++)
        {
                for(int j = 0; j < 1024; j++) bag.insert((i * 1024) + j);
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
        bag.for_all(foo);
}

void lock_based_extract(auto& vec)
{
        #pragma omp parallel for
        for(std::size_t i = 0; i < vec.size(); i++)
        {
                #pragma omp critical
                vec.pop_back();
        }
}

void wait_free_extract(auto& bag)
{
        #pragma omp parallel for
        for(int i = 0; i < 1024; i++)
        {
                for(int j = 0; j < 2048; j++) bag.extract();
        }
}

int main()
{
        wait_free_bag::WaitFreeBag<int, 32> bag;
        std::vector<int>                    vec;

        // TODO
}
