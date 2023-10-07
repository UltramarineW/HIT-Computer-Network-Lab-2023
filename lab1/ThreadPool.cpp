#include <thread>
#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t th_cnt) : _th_cnt(th_cnt), _is_running(false) {
    _threads = new std::thread[th_cnt];
}

ThreadPool::~ThreadPool() {
    if (_is_running)
        stop(); // 终止所有进程

    delete[] _threads;
}

void ThreadPool::start() {
    _is_running = true;
    for (size_t i = 0; i < _th_cnt; i++) // 创建所有进程
        _threads[i] = std::thread(&ThreadPool::work, this);
}

void ThreadPool::stop() {
    _is_running = false;
    _cond.notify_all(); // 唤醒所有挂起进程，否则在回收时可能卡住

    for (size_t i = 0; i < _th_cnt; i++) // 回收所有线程
    {
        std::thread &t = _threads[i];
        if (t.joinable())
            t.join();
    }
}

void ThreadPool::add_task(const ThreadPool::Task &task) {
    {
        std::lock_guard<std::mutex> lg(_mtx);
        _tasks.push(task);

        if (_is_running)
            _cond.notify_one(); // wake a thread to do the task
    }
}

void ThreadPool::work() {
    while (_is_running) {
        Task task;
        {
            std::unique_lock<std::mutex> lk(_mtx);
            if (!_tasks.empty()) {
                task = _tasks.front();
                _tasks.pop();
            } else if (_is_running) // 无任务，挂起
                _cond.wait(lk);
        }
        if (task)
            task(); // do the task
    }
}