#include <omp.h>
#include <wait_free_bag.hpp>

const std::size_t THREADS             = 16;
const std::size_t ELEMENTS_PER_THREAD = (1UZ << 60);

void wait_free_insert(auto& bag)
{
        #pragma omp parallel for
        for(std::size_t i = 0; i < THREADS; i++)
        {
                for(std::size_t j = 0; j < ELEMENTS_PER_THREAD; j++) bag.insert(static_cast<int>((i * ELEMENTS_PER_THREAD) + j));
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
        for(std::size_t i = 0; i < THREADS; i++)
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
