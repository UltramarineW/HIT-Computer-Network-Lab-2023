#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>

class ThreadPool
{
public:
	using Task = std::function<void()>;

	explicit ThreadPool(size_t th_cnt);

	~ThreadPool();

	void start();

	void stop();

	void add_task(const Task &task);

private:
	void work(); 

	size_t th_cnt_;
	std::atomic_bool is_running_;
	std::mutex mtx_;
	std::condition_variable cond_;
	std::thread *threads_;
	std::queue<Task> tasks_;
};

#endif