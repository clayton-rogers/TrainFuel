#pragma once


#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <iostream>

class Thread_Pool {
public:
	Thread_Pool(int start, int end, void(*fn)(int)) :
		start(start), end(end), function(fn) {
	}

	void run(int requested_threads = DEFAULT_NUMBER_THREADS) {
		const int num_threads =
			(requested_threads == DEFAULT_NUMBER_THREADS) ?
			std::thread::hardware_concurrency() :
			requested_threads;

		std::vector<std::atomic<bool>> ready_for_more_work(num_threads);
		for (auto& atomic_bool : ready_for_more_work) {
			atomic_bool.store(false);
		}
		std::atomic<bool> stop{ false };
		std::vector<int> work(num_threads, 0);
		std::vector<std::thread> threads;
		for (int i = 0; i < num_threads; ++i) {
			threads.emplace_back(
				[&, i]() {
					ready_for_more_work.at(i).store(true);
					while (!stop.load()) {
						if (!ready_for_more_work.at(i).load()) {
							int local_work = work.at(i);
							//std::cout << "Thread " << i << " running work " << local_work << std::endl;
							function(local_work);
							ready_for_more_work.at(i).store(true);
						}
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
					}
				}
			);
		}

		// main thread feeds work
		int next_work = start;
		bool work_is_done = false;
		while (!work_is_done) {
			for (int i = 0; i < num_threads; ++i) {
				if (ready_for_more_work.at(i).load()) {
					work.at(i) = next_work;
					ready_for_more_work.at(i).store(false); // indicate work is ready
					++next_work;
					if (next_work == end) {
						work_is_done = true;
						break;
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		// Wait for all threads to finish their task
		for (int i = 0; i < num_threads; ++i) {
			while (!ready_for_more_work.at(i).load()) {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
		// Tell threads to exit
		stop.store(true);
		// Collect them all
		for (auto& t : threads) {
			t.join();
		}
	}

private:
	static const int DEFAULT_NUMBER_THREADS = 9999;
	int start;
	int end;
	void(*function)(int);
};