 //SimpleThread-Pool.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include "pch.h"
#include <chrono>
#include <iostream>
#include "ThreadPoolWithLock.h"
#include "ThreadPoolWithLockExtended.h"

#define Test 1

#if 0

int main()
{
	ThreadPoolWithLock threadPool(4);

	std::vector<std::future<unsigned long long>> futures;

	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));


	auto start = std::chrono::system_clock::now();

	for (int x = 1; x <= 100; x++)
	{
		unsigned int n = 12;
		futures.emplace_back(threadPool.AddTask([n]() {

			unsigned long long  res = 1, i;
			for (i = 2; i <= n; i++)
				res *= i;


			unsigned long long  res1 = 0;
			for (int i = 0; i < res; ++i)
				for (int j = 0; j < res; ++j)
					res1 += i +j;

			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			return res;

		}));

	}

	unsigned long long r = 0;
	for (auto && result : futures)
		r += result.get();// << ' ';
	std::cout << r << std::endl;

	auto end = std::chrono::system_clock::now();
	double elapsed_seconds = std::chrono::duration_cast<
		std::chrono::duration<double>>(end - start).count();
	std::cout << "Total time taken: " << elapsed_seconds << '\n';
	return 0;
}
//2.74

#else

int main()
{
	ThreadPoolWithLockExtended threadPool(4);

	std::vector<std::future<unsigned long long>> futures;

	//std::this_thread::sleep_for(std::chrono::milliseconds(2000));


	auto start = std::chrono::system_clock::now();

	for (int x = 1; x <= 100; x++)
	{
		unsigned int n = 12;
		futures.emplace_back(threadPool.AddTask([n]() {

			unsigned long long  res = 1, i;
			for (i = 2; i <= n; i++)
				res *= i;


			unsigned long long  res1 = 0;
			for (int i = 0; i < res; ++i)
				for (int j = 0; j < res; ++j)
					res1 += i + j;

			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			return res;

		}));

	}

	unsigned long long r = 0;
	for (auto && result : futures)
		r += result.get();// << ' ';
	std::cout << r << std::endl;

	auto end = std::chrono::system_clock::now();
	double elapsed_seconds = std::chrono::duration_cast<
		std::chrono::duration<double>>(end - start).count();
	std::cout << "Total time taken: " << elapsed_seconds << '\n';
	return 0;
}
//0.00119 //0.0012
//0.014 //0.0104 //0.07
//0.08 //0.09
//0.96
// 7.55 1 mill

#endif // TEST


