#include <array>
#include <atomic>
#include <optional>
#include <thread>

namespace wait_free_bag
{
        template<typename Func>
        concept invokable = requires(Func foo) { foo(); };

        // Ref: https://www.cs.rochester.edu/~scott/papers/1996_PODC_queues.pdf
        template<typename DataType>
        class WaitFreeQueue
        {
                private:
                        struct node_t
                        {
                                        DataType              data;
                                        std::atomic_uintptr_t next;
                        };

                        std::atomic_uintptr_t head;
                        std::atomic_uintptr_t tail;

                public:
                        void enqueue(DataType data)
                        {
                                // TODO
                        }

                        std::optional<DataType> dequeue()
                        {
                                // TODO
                        }
        };

        template<typename DataType, size_t ThreadCount>
        class WaitFreeBag
        {
                private:
                        struct WaitFreeBagImpl
                        {
                                std::array<WaitFreeQueue<DataType>, ThreadCount> data;

                                // Ref: https://stackoverflow.com/questions/3069255/singleton-multi-threading
                                static WaitFreeBagImpl& get_instance()
                                {
                                        static WaitFreeBagImpl obj;
                                        return obj;
                                }
                        };

                        std::thread::id my_id;
                        WaitFreeBagImpl impl;

                public:
                        WaitFreeBag(): my_id(std::hash<std::thread::id>()(std::this_thread::get_id()) % ThreadCount), impl(WaitFreeBagImpl::get_instance()) {}

                        void insert(DataType element)
                        {
                                // TODO
                        }

                        std::optional<DataType> extract()
                        {
                                // TODO
                                return {};
                        }

                        std::size_t size()
                        {
                                // TODO
                                return 0;
                        }

                        void for_all(invokable auto f)
                        {
                                f();
                                // TODO
                        }
        };
} // namespace wait_free_bag
