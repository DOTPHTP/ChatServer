#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include "LockFreeQueue.h"

//线程池实现

class ThreadPool
{
    private:
        struct TaskWrapper{
            std::function<void()> task;
            //支持优先级排序
            int priority = 0;
            bool operator<(const TaskWrapper& other) const{
                //越大，优先级越高
                return priority < other.priority;
            }
        };
    std::vector<std::thread> workers;//线程池
    LockFreeQueue<TaskWrapper> task_queue;//任务队列
    std::priority_queue<TaskWrapper> pri_queue;//优先级任务队列
    std::atomic<bool> stop{false};//线程池是否停止
    std::mutex queue_mtx;//优先级任务队列互斥锁
    std::condition_variable condition;//条件变量
    void worker_thread(){
        while(!stop.load()){
            std::unique_lock<std::mutex> lock(queue_mtx);
            condition.wait(lock, [this]{
                return stop.load() || !pri_queue.empty();
            });
            //如果线程池停止，且优先级队列为空，则退出
            if(stop.load() && pri_queue.empty()){
                return;
            }
            //如果优先级队列不为空，则取出优先级最高的任务
            if(!pri_queue.empty()){
                auto task = std::move(pri_queue.top().task);
                pri_queue.pop();
                lock.unlock();
                task();
            }else{
                lock.unlock();
                //否则取出普通任务
                auto Task = std::move(task_queue.pop());
                if(Task){
                    Task->task();
                }
            }
        }
    }

    public:
        ThreadPool(size_t thread_num = std::thread::hardware_concurrency()):task_queue(),pri_queue(){
            for(size_t i = 0; i < thread_num; ++i){
                workers.emplace_back(&ThreadPool::worker_thread, this);
            }
        }
        ~ThreadPool(){
            stop.store(true);
            condition.notify_all();
            for(auto& worker : workers){
                worker.join();
            }
        }

        //提交任务
        template<typename F, typename... Args>
        auto enqueue(F&& func, int priority, Args&&... args)->std::future<decltype(func(args...))>{
            using return_type = decltype(func(args...));
           
            auto task = std::make_shared<std::packaged_task<return_type()>>(
                
                std::bind(std::forward<F>(func), std::forward<Args>(args)...)
            );
        };
};


















#endif // THREADPOOL_H

