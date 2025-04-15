#include <array>
#include <atomic>
#include <omp.h>
#include <optional>
#include <stdexcept>
#include <thread>

namespace wait_free_bag
{
        template<typename Func, typename DataType>
        concept invokable = requires(Func foo, DataType elem) { foo(elem); };

        // Ref: https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf
        template<typename DataType>
        class WaitFreeQueue
        {
                private:
                        struct node_t
                        {
                                public:
                                        DataType             data;
                                        std::atomic<node_t*> next;
                        };

                        std::atomic<node_t*> head;
                        std::atomic<node_t*> tail;

                        const size_t mask = 0x0000ffffffffffff;

                public:
                        WaitFreeQueue()
                        {
                                node_t* const node = new node_t();
                                node->next.store(nullptr);
                                head.store(node);
                                tail.store(node);
                        }

                        bool enqueue(const DataType& data)
                        {
                                // Set up the new node
                                node_t* const node = new node_t();
                                if(!node || ((reinterpret_cast<size_t>(node) & mask) != reinterpret_cast<size_t>(node))) return false;
                                node->data = std::move(data);
                                node->next.store(nullptr);

                                node_t* tail_copy = nullptr;
                                while(true)
                                {
                                        tail_copy                   = tail.load();
                                        node_t* const tail_copy_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(tail_copy) & mask);
                                        node_t*       next          = (tail_copy_ptr->next).load();
                                        if(tail.load() == tail_copy)
                                        {
                                                if((reinterpret_cast<size_t>(next) & mask) == 0x0) // Add the new node
                                                {
                                                        node_t* const node_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(node) | 0x0001000000000000);
                                                        if(std::atomic_compare_exchange_weak(&(tail_copy_ptr->next), &next, node_ptr)) break;
                                                }
                                                else // Move the tail forward
                                                {
                                                        const size_t int_tail_copy = reinterpret_cast<size_t>(tail_copy);
                                                        size_t       count         = (int_tail_copy & mask) >> 48;
                                                        count                      = (count + 1) & 0x000000000000ffff;
                                                        count                      = (count << 48) & mask;
                                                        node_t* const new_tail     = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(next) | count);
                                                        std::atomic_compare_exchange_weak(&tail, &tail_copy, new_tail);
                                                }
                                        }
                                }

                                const size_t int_tail_copy = reinterpret_cast<size_t>(tail_copy);
                                size_t       count         = (int_tail_copy & mask) >> 48;
                                count                      = (count + 1) & 0x000000000000ffff;
                                count                      = (count << 48) & mask;
                                node_t* const new_tail     = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(node) | count);
                                std::atomic_compare_exchange_weak(&tail, &tail_copy, new_tail);

                                return true;
                        }

                        std::optional<DataType> dequeue()
                        {
                                std::optional<DataType> value;

                                node_t* head_copy = nullptr;
                                while(true)
                                {
                                        head_copy                         = head.load();
                                        const node_t* const head_copy_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(head_copy) & mask);
                                        node_t*             tail_copy     = tail.load();
                                        const node_t* const tail_copy_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(tail_copy) & mask);
                                        const node_t* const next          = head_copy_ptr->next;
                                        const node_t* const next_ptr      = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(next) & mask);
                                        if(head_copy == head.load())
                                        {
                                                // Check if the queue is empty or if the tail is lagging behind
                                                if(head_copy_ptr == tail_copy_ptr)
                                                {
                                                        if((reinterpret_cast<size_t>(next) & mask) == 0x0) // Queue is empty
                                                                return {};

                                                        // Advance the tail
                                                        size_t count           = (reinterpret_cast<size_t>(tail_copy_ptr) & mask) >> 48;
                                                        count                  = (count + 1) & 0x000000000000ffff;
                                                        count                  = (count << 48) & mask;
                                                        node_t* const new_tail = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(next) | count);
                                                        std::atomic_compare_exchange_weak(&tail, &tail_copy, new_tail);
                                                }

                                                value = std::move(next_ptr->data);

                                                // Advance the head
                                                size_t count           = (reinterpret_cast<size_t>(head_copy_ptr) & mask) >> 48;
                                                count                  = (count + 1) & 0x000000000000ffff;
                                                count                  = (count << 48) & mask;
                                                node_t* const new_head = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(next) | count);
                                                if(std::atomic_compare_exchange_weak(&head, &head_copy, new_head)) break;
                                        }
                                }

                                const node_t* const head_copy_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(head_copy) & mask);
                                delete head_copy_ptr;
                                return value;
                        }

                        template<typename Func>
                                requires invokable<Func, DataType>
                        void for_all(Func f)
                        {
                                const node_t* const  tail_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(tail.load()) & mask);
                                std::atomic<node_t*> iterator(head.load());
                                const node_t*        iterator_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(iterator.load()) & mask);
                                while(iterator_ptr != tail_ptr)
                                {
                                        const node_t* const raw_next_ptr = (iterator_ptr->next).load();
                                        node_t* const       next_ptr     = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(raw_next_ptr) & mask);

                                        f(next_ptr->data);
                                        iterator.store(next_ptr);
                                        iterator_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(iterator.load()) & mask);
                                }
                        }

                        ~WaitFreeQueue()
                        {
                                const node_t* const  tail_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(tail.load()) & mask);
                                std::atomic<node_t*> iterator(head.load());
                                const node_t*        iterator_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(iterator.load()) & mask);
                                while(iterator_ptr != tail_ptr)
                                {
                                        const node_t* const raw_next_ptr = (iterator_ptr->next).load();
                                        node_t* const       next_ptr     = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(raw_next_ptr) & mask);

                                        delete iterator_ptr;
                                        iterator.store(next_ptr);

                                        iterator_ptr = reinterpret_cast<node_t*>(reinterpret_cast<size_t>(iterator.load()) & mask);
                                }

                                delete tail_ptr;
                        }
        };

        template<typename DataType, size_t Spread>
        class WaitFreeBag
        {
                private:
                        std::array<WaitFreeQueue<DataType>, Spread> data;
                        std::atomic_int_least64_t                   num_elements = 0;

                public:
                        void insert(DataType element)
                        {
                                thread_local int idx     = omp_get_thread_num() % Spread;
                                bool             success = data[idx].enqueue(element);
                                idx                      = (idx + 1) % Spread;

                                std::atomic_fetch_add(&num_elements, 1);

                                if(!success)
                                {
                                        throw std::logic_error("Could not insert object\n");
                                }
                        }

                        std::optional<DataType> extract()
                        {
                                thread_local int idx = omp_get_thread_num() % Spread;

                                std::optional<DataType> element;
                                for(int i = 0; static_cast<size_t>(i) < Spread; i++)
                                {
                                        element = data[idx].dequeue();
                                        idx     = (idx + 1) % Spread;
                                        if(element) break;
                                }
                                if(element) std::atomic_fetch_sub(&num_elements, 1);

                                return element;
                        }

                        size_t size() const
                        {
                                return static_cast<size_t>(num_elements);
                        }

                        template<typename Func>
                                requires invokable<Func, DataType>
                        void for_all(Func f)
                        {
                                #pragma omp barrier
                                int idx = omp_get_thread_num();

                                while(static_cast<size_t>(idx) < Spread)
                                {
                                        data[idx].for_all(f);
                                        idx = (idx + omp_get_num_threads());
                                }
                                #pragma omp barrier
                        }
        };
} // namespace wait_free_bag
