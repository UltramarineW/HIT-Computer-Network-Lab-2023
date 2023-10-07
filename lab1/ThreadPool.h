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

	size_t _th_cnt; 
	std::atomic_bool _is_running; 
	std::mutex _mtx; 
	std::condition_variable _cond;
	std::thread *_threads; 
	std::queue<Task> _tasks; 
};

#endif