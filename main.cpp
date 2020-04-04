#include <iostream>
#include <vector>
#include "Thread_Pool.h"
#include <atomic>

struct TrainState {
	double distance = 0.0;
	double fuel = 0.0;
	double divisor = 1.0;
};


double get_fuel_cost(double distance, double capacity, bool is_cached = false) {

	double divisor = 1.0;
	double distance_traveled = 0.0;
	double fuel = 0.0;

	static TrainState cached_state;

	if (is_cached) {
		if (cached_state.distance < distance) {
			divisor = cached_state.divisor;
			distance_traveled = cached_state.distance;
			fuel = cached_state.fuel;
		}
	}

	while (true) {
		double posible_distance_on_this_round = capacity / divisor;

		if (distance_traveled + posible_distance_on_this_round > distance) {
			if (is_cached) {
				cached_state.fuel = fuel;
				cached_state.distance = distance_traveled;
				cached_state.divisor = divisor;
			}

			double amount_needed = distance - distance_traveled;
			fuel += amount_needed * divisor;
			distance_traveled += amount_needed;
			break;
		} else {
			fuel += posible_distance_on_this_round * divisor;
			distance_traveled += posible_distance_on_this_round;
			divisor += 2.0;
		}
	}

	return fuel;
}

struct WorkUnit {
	double distance;
	double fuel = 0;
	std::atomic<bool> isDone{ false };

	WorkUnit(double distance) : distance(distance) {}
	
	// Allow thread unsafe copy
	WorkUnit(const WorkUnit& other) {
		distance = other.distance;
	}
};
static std::vector<WorkUnit> work;


static int NUM_THREAD = 1;
static bool USE_CACHE = (NUM_THREAD == 1) ? true : false;

void do_work(int unit) {
	//std::cout << unit << std::endl;
	const double distance = work.at(unit).distance;
	work.at(unit).fuel = get_fuel_cost(distance, 500, USE_CACHE);
	work.at(unit).isDone.store(true);
}

int main() {
	for (double distance = 100; distance <= 20010; distance += 10) {
		work.emplace_back(distance);
	}

	Thread_Pool tp(0, work.size(), do_work);
	std::thread tp_thread([&tp]() {
		tp.run(NUM_THREAD);
		});

	for (const auto& unit : work) {
		while (!unit.isDone.load()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		std::cout << "Dist: " << unit.distance << " fuel: " << unit.fuel << std::endl;
	}

	tp_thread.join();
}
