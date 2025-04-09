#include <functional>
#include <iostream>
#include <mutex>
#include <optional>
#include <syncstream>
#include <thread>

template<typename Func>
concept invokable = requires(Func foo)
{
        foo();
};

template<typename DataType, size_t ThreadCount>
struct bag_data
{
                // TODO

                // Ref: https://stackoverflow.com/questions/3069255/singleton-multi-threading
                static bag_data& get_instance()
                {
                        static bag_data obj;
                        return obj;
                }
};

template<typename DataType, size_t ThreadCount>
class bag
{
        private:
                std::thread::id                 my_id;
                bag_data<DataType, ThreadCount> bag_impl;

        public:
                bag(): my_id(std::hash<std::thread::id>()(std::this_thread::get_id()) % ThreadCount), bag_impl(bag_data<DataType, ThreadCount>::get_instance()) {}

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
