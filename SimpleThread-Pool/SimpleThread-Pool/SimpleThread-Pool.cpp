// SimpleThread-Pool.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <chrono>
#include <iostream>
#include"ThreadPoolWithLock.h"

int main()
{
	ThreadPoolWithLock threadPool(4);

	std::vector<std::future<unsigned long long>> futures;

	auto start = std::chrono::system_clock::now();

	for (int x = 1; x < 100; x++)
	{
		unsigned int n = 64;
		futures.emplace_back(threadPool.AddTask([n]() {
			
			unsigned long long  res = 1, i;
			for (i = 2; i <= n; i++)
				res *= i;

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			return res;

		}));

	}

	for (auto && result : futures)
		std::cout << result.get() << ' ';
	std::cout << std::endl;

	auto end = std::chrono::system_clock::now();
	double elapsed_seconds = std::chrono::duration_cast<
		std::chrono::duration<double>>(end - start).count();
	std::cout << "Total time taken: " << elapsed_seconds << '\n';
	return 0;
}

