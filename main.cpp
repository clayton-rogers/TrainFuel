#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <thread>

struct TrainState {
	double distance = 0.0;
	double fuel = 0.0;
	double divisor = 1.0;
	double capacity = 0;
};

double get_fuel_cost(double distance, double capacity) {

	double divisor = 1.0;
	double distance_traveled = 0.0;
	double fuel = 0.0;

	static TrainState cached_state;

	// If we're moving to a new capacity, then reset cached data
	if (cached_state.capacity != capacity) {
		cached_state = TrainState();
	}

	if (cached_state.distance < distance) {
		divisor = cached_state.divisor;
		distance_traveled = cached_state.distance;
		fuel = cached_state.fuel;
	}

	while (true) {
		double posible_distance_on_this_round = capacity / divisor;

		if (distance_traveled + posible_distance_on_this_round > distance) {

			cached_state.fuel = fuel;
			cached_state.distance = distance_traveled;
			cached_state.divisor = divisor;

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

class Work {
public:
	Work(double capacity) : capacity(capacity) {}
	struct Result {
		double distance = 0.0;
		double fuel = 0.0;
		Result(double distance, double fuel) : distance(distance), fuel(fuel) {}
	};

	std::vector<Result> results;

	void run() {
		bool exceeded_time = false;
		double fuel = 0.0;
		for (double distance = 0.1; distance <= 5; distance += 0.1) {
			auto start_time = std::chrono::high_resolution_clock::now();
			if (!exceeded_time) {
				fuel = get_fuel_cost(distance, capacity);
			}
			results.emplace_back(distance, fuel);
			std::cout << distance << " " << capacity << " " << std::fixed << fuel << std::endl;
			if (std::chrono::high_resolution_clock::now() - start_time > std::chrono::seconds(10)) {
				exceeded_time = true;
			}
		}
		//std::cout << std::endl; // blank line between sets
	}

	void print_result() {
		for (const auto& r : results) {
			std::cout << r.distance << " " << capacity << " " << std::fixed << r.fuel << std::endl;
		}
	}

	const double capacity;

private:
	
};

int main() {
	// Output format is for gnu plot
	std::cout << "# Dist Capacity fuel" << std::endl;
	std::vector<Work> works;
	for (double capacity = 1; capacity <= 1; capacity += 100) {
		works.emplace_back(capacity);
		//Work w(capacity);
		//w.run();
		//w.print_result();
		//std::cout << std::endl; // blank line between sets

		//bool exceeded_time = false;
		//double fuel = 0.0;
		//for (double distance = 100; distance <= 20000; distance += 100) {
		//	auto start_time = std::chrono::high_resolution_clock::now();
		//	if (!exceeded_time) {
		//		fuel = get_fuel_cost(distance, capacity);
		//	}
		//	std::cout << distance << " " << capacity << " " << std::fixed << fuel << std::endl;
		//	if (std::chrono::high_resolution_clock::now() - start_time > std::chrono::seconds(2)) {
		//		exceeded_time = true;
		//	}
		//}
		
	}

	std::vector<std::thread> threads;
	for (int i = 0; i < (int)works.size(); ++i) {
		threads.emplace_back([&works, i]() {
			works.at(i).run();
			});
	}
	for (auto& t : threads) {
		t.join();
	}

	for (auto& w : works) {
		//w.print_result();
		//std::cout << std::endl; // blank line between sets
	}

}
