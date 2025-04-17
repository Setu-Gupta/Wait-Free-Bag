#include <omp.h>
#include <wait_free_bag.hpp>

const int THREADS             = 16;
const int ELEMENTS_PER_THREAD = 65536 * 32;

void wait_free_insert(auto& bag)
{
#pragma omp parallel for
        for(int i = 0; i < THREADS; i++)
        {
                for(int j = 0; j < ELEMENTS_PER_THREAD; j++) bag.insert((i * ELEMENTS_PER_THREAD) + j);
        }
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

        wait_free_insert(bag);
        wait_free_for_all(bag);
        wait_free_extract(bag);
}
