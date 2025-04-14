#include <iostream>
#include <omp.h>
#include <syncstream>
#include <vector>
#include <wait_free_bag.hpp>

void insert_test(wait_free_bag::WaitFreeBag<int, 1>& bag)
{
#pragma omp parallel for
        for(int i = 0; i < 1; i++)
        {
                for(int j = 0; j < 10; j++) bag.insert(j);
        }
}

void extract_test([[maybe_unused]] wait_free_bag::WaitFreeBag<int, 1>& bag)
{
        // TODO
}

void for_all_test(wait_free_bag::WaitFreeBag<int, 1>& bag)
{
        auto printer = [](int value)
        {
                std::osyncstream(std::cout) << "Value = " << value << '\n';
        };
        bag.for_all(printer);
}

int main()
{
        wait_free_bag::WaitFreeBag<int, 1> bag;

        insert_test(bag);
        extract_test(bag);
        for_all_test(bag);
}
