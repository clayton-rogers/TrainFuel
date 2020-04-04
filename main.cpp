#include <iostream>

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

int main() {
	// Output format is for gnu plot
	std::cout << "# Dist Capacity fuel" << std::endl;
	for (double capacity = 500; capacity <= 2000; capacity += 100) {
		for (double distance = 100; distance <= 6000; distance += 100) {
			double fuel = get_fuel_cost(distance, capacity);
			std::cout << distance << " " << capacity << " " << fuel << std::endl;
		}
		std::cout << std::endl; // blank line between sets
	}
}
