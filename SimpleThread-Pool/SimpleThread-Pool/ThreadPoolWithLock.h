#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <queue>
#include <functional>

class ThreadPoolV1
{
public:
	ThreadPoolV1(size_t size);

	~ThreadPoolV1();

	template<class T, class... Args >
	auto AddTask(T&& t, Args&&... args)->std::future<typename std::result_of<T(Args...)>::type>;

private:

	std::vector<std::thread> workers;

	std::queue<std::function<void()>> tasks;
	std::mutex mutex;
};

ThreadPoolV1::ThreadPoolV1(size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		workers.emplace_back(
			[this]()
		{
			for (;;)
			{
				std::function<void()> task;
				{
					std::lock_guard<std::mutex> lock(this->mutex);
					task = std::move(tasks.front());
					tasks.pop();
				}

				task();
			}
		}

		);
	}
}

ThreadPoolV1::~ThreadPoolV1()
{
	for (std::thread &t : workers)
	{
		t.join();
	}
}

template<class T, class... Args >
auto ThreadPoolV1::AddTask(T&& t, Args&&... args)->std::future<typename std::result_of<T(Args...)>::type>
{
	using returnType = typename std::result_of<T(Args...)>::type;
	auto task = std::make_shared<std::packaged_task<returnType()>>(std::bind(std::forward<T>(t), std::forward<Args>(args)...));

	std::future<returnType> result = task->get_future();
	{
		std::lock_guard<std::mutex> lock(mutex);

		tasks.emplace([task]() {(*task)(); });
	}
	return result;
}


