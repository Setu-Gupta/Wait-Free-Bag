#include <iostream>
#include <syncstream>
#include <thread>
#include <wait_free_bag.hpp>

void foo()
{
        bag<int, 2> bg;
        std::osyncstream(std::cout) << "Hi from " << std::this_thread::get_id() << '\n';
}

int main()
{
         std::jthread t1 {foo};
         std::jthread t2 {foo};
}
