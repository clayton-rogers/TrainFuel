#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

class TrainCalculator {
public:
	TrainCalculator(double capacity, double dist_min, double dist_max, double dist_step) :
		capacity(capacity), dist_min(dist_min), dist_max(dist_max), dist_step(dist_step) {}

	struct Result {
		double distance = 0.0;
		double fuel = 0.0;
		Result(double dist, double fuel) :
			distance(dist), fuel(fuel) {}
	};

	void run() {
		for (double distance = dist_min; distance <= dist_max; distance += dist_step) {
			double fuel = get_fuel_cost(distance, capacity);
			results.emplace_back(distance, fuel);
		}
	}

	void print_results() {
		for (const auto& result : results) {
			std::cout << result.distance << " " << capacity << " " << result.fuel << std::endl;
		}
	}

private:
	const double capacity;
	const double dist_min;
	const double dist_max;
	const double dist_step;
	std::vector<Result> results;

	struct TrainState {
		double distance = 0.0;
		double fuel = 0.0;
		double divisor = 1.0;
		double capacity = 0;
	};

	TrainState cached_state;

	double get_fuel_cost(double distance, double capacity) {

		double divisor = 1.0;
		double distance_traveled = 0.0;
		double fuel = 0.0;

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
};

int main() {
	// Output format is for gnu plot
	std::cout << "# Dist Capacity fuel" << std::endl;
	std::vector<std::thread> threads;
	std::vector<TrainCalculator> tc;
	for (double capacity = 500; capacity <= 2000; capacity += 100) {
		tc.emplace_back(capacity, 100, 6500, 100);
	}

	for (int i = 0; i < (int)tc.size(); ++i) {
		threads.emplace_back([&tc, i]() {
			tc.at(i).run();
			});
	}

	for (auto& thread : threads) {
		thread.join();
	}

	for (auto& t : tc) {
		t.print_results();
		std::cout << std::endl; // blank line between sets
	}
}
