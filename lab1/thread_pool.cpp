#include <thread>
#include "thread_pool.h"
#include <iostream>

ThreadPool::ThreadPool(size_t th_cnt) : th_cnt_(th_cnt), is_running_(false) {
    threads_ = new std::thread[th_cnt];
}

ThreadPool::~ThreadPool() {
    if (is_running_)
        stop(); // 终止所有进程

    delete[] threads_;
}

void ThreadPool::start() {
    is_running_ = true;
    for (size_t i = 0; i < th_cnt_; i++) // 创建所有进程
        threads_[i] = std::thread(&ThreadPool::work, this);
}

void ThreadPool::stop() {
    is_running_ = false;
    cond_.notify_all(); // 唤醒所有挂起进程，否则在回收时可能卡住

    for (size_t i = 0; i < th_cnt_; i++) // 回收所有线程
    {
        std::thread &t = threads_[i];
        if (t.joinable())
            t.join();
    }
}

void ThreadPool::add_task(const ThreadPool::Task &task) {
    {
        std::lock_guard<std::mutex> lg(mtx_);
        tasks_.push(task);
        std::cout << "task_number: " << tasks_.size() << std::endl;

        if (is_running_)
            cond_.notify_one(); // wake a thread to do the task
    }
}

void ThreadPool::work() {
    while (is_running_) {
        Task task;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            if (!tasks_.empty()) {
                task = tasks_.front();
                tasks_.pop();
            } else if (is_running_) // 无任务，挂起
                cond_.wait(lk);
        }
        if (task)
            task(); // do the task
    }
}