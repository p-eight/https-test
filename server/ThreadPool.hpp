#pragma once  
#include <functional>  
#include <vector>  
#include <thread>  
#include <queue>  
#include <mutex>  
#include <condition_variable>  
#include <atomic>  

class ThreadPool {  
public:  
    explicit ThreadPool(size_t thread_count) : stop(false) {  
        for (size_t i = 0; i < thread_count; ++i) {  
            workers.emplace_back([this] {  
                while (true) {  
                    std::function<void()> task;  
                    {  
                        std::unique_lock<std::mutex> lock(this->queue_mutex);  
                        this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });  
                        if (this->stop && this->tasks.empty())  
                            return;  
                        task = std::move(this->tasks.front());  
                        this->tasks.pop();  
                    }  
                    task();  
                }  
            });  
        }  
    }  

    ~ThreadPool() {  
        {  
            std::unique_lock<std::mutex> lock(queue_mutex);  
            stop = true;  
        }  
        condition.notify_all();  
        for (std::thread &worker : workers)  
            worker.join();  
    }  

    ThreadPool(const ThreadPool&) = delete;  
    ThreadPool& operator=(const ThreadPool&) = delete;  

    ThreadPool(ThreadPool&& other) noexcept :  
        workers(std::move(other.workers)),  
        tasks(std::move(other.tasks)),  
        stop(other.stop.load()) {}  

    ThreadPool& operator=(ThreadPool&& other) noexcept {  
        if (this != &other) {  
            {  
                std::unique_lock<std::mutex> lock(queue_mutex);  
                stop = other.stop.load();  
                tasks = std::move(other.tasks);  
            }  
            workers = std::move(other.workers);  
        }  
        return *this;  
    }  

    template<class F, class... Args>  
    void enqueue(F&& f, Args&&... args) {  
        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);  
        {  
            std::unique_lock<std::mutex> lock(queue_mutex);  
            tasks.emplace(std::move(task));  
        }  
        condition.notify_one();  
    }  

private:  
    std::vector<std::thread> workers;  
    std::queue<std::function<void()>> tasks;  
    std::mutex queue_mutex;  
    std::condition_variable condition;  
    std::atomic<bool> stop;  
};