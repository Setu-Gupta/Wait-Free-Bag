#include <iostream>
#include <omp.h>
#include <syncstream>
#include <vector>
#include <wait_free_bag.hpp>

void insert_test(auto& bag)
{
        #pragma omp parallel for
        for(int i = 0; i < 16; i++)
        {
                for(int j = 0; j < 16; j++) bag.insert((i * 16) + j);
        }
}

void extract_test([[maybe_unused]] auto& bag)
{
        // TODO
}

void for_all_test(auto& bag)
{
        auto printer = [](int value)
        {
                std::osyncstream(std::cout) << "Value = " << value << '\n';
        };
        bag.for_all(printer);
}

void size_test([[maybe_unused]] auto& bag)
{
        // TODO
}

int main()
{
        wait_free_bag::WaitFreeBag<int, 16> bag;

        insert_test(bag);
        std::cout << "=========== Insert Done =============" << std::endl;
        size_test(bag);
        std::cout << "===========  Size  Done =============" << std::endl;
        for_all_test(bag);
        std::cout << "=========== ForAll Done =============" << std::endl;
        extract_test(bag);
        std::cout << "=========== Extract Done =============" << std::endl;
        size_test(bag);
        std::cout << "===========  Size  Done =============" << std::endl;
        for_all_test(bag);
        std::cout << "=========== ForAll Done =============" << std::endl;
}
