#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <queue>
#include <functional>

class ThreadPoolWithLock
{
public:

	ThreadPoolWithLock(size_t numOfThreads = 4)
	{
		//create threads and let it run indefnitely and always be looking for a task to take from queue.
		for (size_t i = 0; i < numOfThreads; ++i)
		{
			//emplace_back : more efficient. No temporary object creation, instead uses forward.
			threads.emplace_back([this]() {

				for (;;)
				{

					std::function<void()> task;
					{
						//use unique lock since we are using conditional variable.
						std::unique_lock<std::mutex> lock(this->tasksMutex);

						this->cond.wait(lock, [this]() { return !this->tasks.empty(); });

						task = std::move(this->tasks.front());
						this->tasks.pop();
					}

					task();

				}
			});
		}
	}

	~ThreadPoolWithLock()
	{
		for (size_t i = 0; i < threads.size(); ++i)
		{
			threads[i].join();
		}
	}

	template<class T, class... Args>
	auto AddTask(T&& function, Args&&... arguments)-> std::future<typename std::result_of<T(Args...)>::type>
	{
		using returnType = typename std::result_of<T(Args...)>::type;

		//create a shared pointer of type packaged task using the incoming function and it's arguments. (use std::bind to bind the parameters since we 
		//already know the arguments. use std::forward since we want to keep the objects as it was send by the user.
		//save the shared pointer as a lamda (std::function) in tasks. Get the future pointer from packaged task and return it.

		auto task = std::make_shared< std::packaged_task<returnType()> >(std::bind(std::forward<T>(function), std::forward<Args>(arguments)...));
		std::future<returnType> result = task->get_future();
		{
			std::unique_lock<std::mutex> lock(this->tasksMutex);
			
			tasks.emplace([task]() { (*task)(); });
		}
		cond.notify_one();

		return result;
	}

private:

	std::vector<std::thread> threads;

	//@todo read more about std::function.
	std::queue<std::function<void()>> tasks;

	std::mutex tasksMutex;

	std::condition_variable cond;
};


