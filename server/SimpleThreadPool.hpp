#pragma once
#include <vector>
#include <thread>

class SimpleThreadPool {
    public:
    SimpleThreadPool() = default;
        
        // Non-copyable, movable
        SimpleThreadPool(const SimpleThreadPool&) = delete;
        SimpleThreadPool& operator=(const SimpleThreadPool&) = delete;
        SimpleThreadPool(SimpleThreadPool&&) = default;
        SimpleThreadPool& operator=(SimpleThreadPool&&) = default;
        
        ~SimpleThreadPool() {
            join_all();
        }
        
        template<typename Function>
        void emplace_back(Function&& f) {
            threads_.emplace_back(std::forward<Function>(f));
        }
        
        void join_all() {
            for (auto& thread : threads_) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            threads_.clear();
        }
        
        size_t size() const { return threads_.size(); }
        bool empty() const { return threads_.empty(); }
        
    private:
        std::vector<std::thread> threads_;
    };