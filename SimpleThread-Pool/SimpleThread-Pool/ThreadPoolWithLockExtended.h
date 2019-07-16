#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <future>
#include <queue>
#include <functional>

class WorkerThread
{
public:
	explicit WorkerThread() : totalTaskExecution(0){}

	//Delete the copy constructor
	WorkerThread(const WorkerThread&) = delete;

	//Delete the Assignment opeartor
	WorkerThread& operator=(const WorkerThread&) = delete;

	WorkerThread(WorkerThread&& rhs) noexcept;

	WorkerThread& operator=(WorkerThread&& rhs) noexcept;

	~WorkerThread();
	
	void StartExecution(size_t id, WorkerThread* neighbour = nullptr);

	void StopExecution();

	template<class T>
	void AddTask(T&& task);

	template<class R>
	void RemoveTask(R& task);

	static size_t getID();

private:

	std::thread thread;

	std::queue<std::function<void()>> tasks;

	std::mutex tasksMutex;

	std::condition_variable cond;

	size_t totalTaskExecution = 0;

	size_t thread_id;
};

WorkerThread::WorkerThread(WorkerThread&& rhs) noexcept
{
	thread = std::move(rhs.thread);	
	{
		std::unique_lock<std::mutex> lock(rhs.tasksMutex);
		tasks = std::move(rhs.tasks);
	}
	rhs.cond.notify_all();
}
	
WorkerThread& WorkerThread::operator=(WorkerThread&& rhs) noexcept
{
	if (this != &rhs)
	{
		if (thread.joinable())
			thread.join();
		thread = std::move(rhs.thread);
		{
			std::unique_lock<std::mutex> l_lock(tasksMutex, std::defer_lock);
			std::unique_lock<std::mutex> r_lock(rhs.tasksMutex, std::defer_lock);
			std::lock(l_lock, r_lock);

			tasks = std::move(rhs.tasks);
		}
		rhs.cond.notify_all();
		cond.notify_all();
	}

	return *this;
}

WorkerThread::~WorkerThread()
{
}

void WorkerThread::StartExecution(size_t id, WorkerThread* neighbour)
{
	thread_id = id;

	thread = std::thread([this, id, neighbour]() {
	
		//@todo to something with id and neighbour.
		//this should be the thread function.
		for (;;)
		{
			std::function<void()> task;
			{
				//use unique lock since we are using conditional variable.
				std::unique_lock<std::mutex> lock(this->tasksMutex);

				this->cond.wait(lock, [this]() { return !this->tasks.empty(); });

				task = tasks.front();
				tasks.pop();
				
			}

			task();
		}
	
	});

}

void WorkerThread::StopExecution()
{
	//std::cout <<"Thread number: " << thread_id <<  ", Total task execution: " << totalTaskExecution << std::endl;
 	if (thread.joinable())
		thread.join();
}

template<class T>
void WorkerThread::AddTask(T&& task)
{
	{
		std::unique_lock<std::mutex> lock(this->tasksMutex);
		tasks.emplace(std::forward<T>(task));
	}
	cond.notify_one();
	this->totalTaskExecution++;
}

template <class R>
void WorkerThread::RemoveTask(R& task)
{
	std::unique_lock<std::mutex> lock(this->tasksMutex);
	task = tasks.front();
	tasks.pop();
	this->totalTaskExecution--;
}


class ThreadPoolWithLockExtended
{
public:

	ThreadPoolWithLockExtended(size_t numOfThreads = 2)
		:counter(0)
	{
		for(int i=0; i< numOfThreads; ++i)
			threads.emplace_back(WorkerThread());

		for (int i = 0; i < threads.size(); ++i)
			threads[i].StartExecution(i);
	}

	~ThreadPoolWithLockExtended()
	{
		for (int i = 0; i < threads.size(); ++i)
		{
			threads[i].StopExecution();
		}
	}

	template<class T, class... Args>
	auto AddTask(T&& function, Args&&... arguments)->std::future<typename std::result_of<T(Args...)>::type>
	{
		if (counter >= threads.size())
			counter = 0;

		using returnType = typename std::result_of<T(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<returnType()> >(std::bind(std::forward<T>(function), std::forward<Args>(arguments)...));
		std::future<returnType> result = task->get_future();
		{
			threads[counter].AddTask([task]() { (*task)(); });
			counter++;

			//std::cout << counter << std::endl;
		}

		return result;
	}

private:

	size_t counter;
	std::vector<WorkerThread> threads;

};