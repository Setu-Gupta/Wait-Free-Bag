// Ref: https://www.openmp.org/spec-html/5.1/openmpsu105.html

#include <array>
#include <atomic>
#include <iostream>
#include <omp.h>
#include <optional>
#include <syncstream>
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
                                        DataType data;
                                        node_t*  next;
                        };

                        node_t* head;
                        node_t* tail;

                        const node_t* mask = 0x0000ffffffffffff;

                public:
                        WaitFreeQueue()
                        {
                                node_t* node = new node_t();
                                node->next   = nullptr;
                                head         = node;
                                tail         = node;
                        }

                        bool enqueue(DataType data)
                        {
                                // Set up the new node
                                node_t* node = new node_t();
                                if(!node || ((node & mask) != node)) return false;
                                node->data = data;
                                node->next = nullptr;

                                node_t* tail_copy = nullptr;
                                while(true)
                                {
                                        tail_copy          = tail;
                                        const node_t* next = tail->next;
                                        if(tail == tail_copy)
                                        {
                                                if((next & mask) == nullptr) // Add the new node
                                                {
                                                        const node_t* node_ptr = node | 0x0001000000000000;
                                                        node_t*       old_next = nullptr;
                                                        if(std::atomic_compare_exchange_weak(&(tail->next), &next, node_ptr)) break;
                                                }
                                                else // Move the tail forward
                                                {
                                                        size_t count           = (tail_copy & mask) >> 48;
                                                        count                  = (count + 1) & 0x000000000000ffff;
                                                        count                  = (count << 48) & mask;
                                                        const node_t* new_tail = next | count;
                                                        std::atomic_compare_exchange_weak(&tail, &tail_copy, new_tail);
                                                }
                                        }
                                }

                                size_t count           = (tail_copy & mask) >> 48;
                                count                  = (count + 1) & 0x000000000000ffff;
                                count                  = (count << 48) & mask;
                                const node_t* new_tail = node | count;
                                std::atomic_compare_exchange_weak(&tail, &tail_copy, new_tail);
                        }

                        std::optional<DataType> dequeue()
                        {
                                std::optional<DataType> value;

                                node_t* head_copy = nullptr;
                                while(true)
                                {
                                        head_copy               = head;
                                        const node_t* tail_copy = tail;
                                        const node_t* next      = head->next;
                                        if(head_copy == head)
                                        {
                                                // Check if the queue is empty or if the tail is lagging behind
                                                if((head & mask) == (tail & mask))
                                                {
                                                        if((next & mask) == nullptr) // Queue is empty
                                                                return {};

                                                        // Advance the tail
                                                        size_t count           = (tail & mask) >> 48;
                                                        count                  = (count + 1) & 0x000000000000ffff;
                                                        count                  = (count << 48) & mask;
                                                        const node_t* new_tail = next | count;
                                                        std::atomic_compare_exchange_weak(&tail, &tail_copy, new_tail);
                                                }

                                                value = next->data;

                                                // Advance the head
                                                size_t count           = (head & mask) >> 48;
                                                count                  = (count + 1) & 0x000000000000ffff;
                                                count                  = (count << 48) & mask;
                                                const node_t* new_head = next | count;
                                                if(std::atomic_compare_exchange_weak(&head, &head_copy, new_head)) break;
                                        }
                                }

                                free(head_copy);
                                return value;
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
                                static thread_local int idx = omp_get_thread_num() % Spread;
                                data[idx].enqueue(data);
                                idx = (idx + 1) % Spread;

                                std::atomic_fetch_add(&num_elements, 1);
                        }

                        std::optional<DataType> extract()
                        {
                                static thread_local int idx     = omp_get_thread_num() % Spread;
                                std::optional<DataType> element = data[idx].enqueue(data);
                                idx                             = (idx + 1) % Spread;

                                if(element) std::atomic_fetch_sub(&num_elements, 1);

                                return element;
                        }

                        size_t size()
                        {
                                return static_cast<size_t>(num_elements);
                        }

                        template<typename Func>
                                requires invokable<Func, DataType>
                        void for_all(Func f)
                        {
                                #pragma omp barrier
                                thread_local int idx = omp_get_thread_num() % Spread;

                                // TODO

                                #pragma omp barrier
                        }
        };
} // namespace wait_free_bag
