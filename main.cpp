#include <iostream>
#include <vector>


class FuelPump {
public:
	FuelPump(double fuel_price) : fuel_price(fuel_price) {}
	double get_price() const { return fuel_price; }
private:
	const double fuel_price;
};

class Station {
public:
	Station(int position) : distance_along_track(position), pump_with_best_price(9999999) {}
	void add_fuel_pump(FuelPump pump) {
		pumps.push_back(pump);
		const double DEFAULT_PRICE = 1E30;
		double best_price = DEFAULT_PRICE;
		for (size_t i = 0; i < pumps.size(); ++i) {
			const double pump_price = pumps.at(i).get_price();
			if (pump_price < best_price) {
				best_price = pump_price;
				pump_with_best_price = i;
			}
		}
	}
	double get_best_price() const {
		if (pumps.size() == 0) {
			return 0;
		}
		return pumps.at(pump_with_best_price).get_price();
	}
	int get_position() const {
		return distance_along_track;
	}
private:
	int distance_along_track;
	std::vector<FuelPump> pumps;
	size_t pump_with_best_price;

};

struct Price {
	double price;
	int position;
};

class RailLine {
public:
	RailLine(int length, int station_spacing, int train_capacity) :
		length(length), station_spacing(station_spacing), train_capacity(train_capacity)
	{
		// Create all the stations
		for (int i = 0; i < length; i += station_spacing) {
			stations.emplace_back(i);
		}
		stations.at(0).add_fuel_pump(FuelPump(0.0));

		// Initial trains from original free station
		{
			int current_station_id = 0;
			int current_fuel = train_capacity;
			//double price_for_full_load = stations.at(current_station_id).get_best_price() * train_capacity;

			++current_station_id;
			current_fuel -= 2 * station_spacing; // 2x because we have to make it back to the previous station to refuel too
			while (current_fuel > 0 && current_station_id < stations.size()) {
				// If the train dumped all its fuel right now, what would the station it creates look like?
				FuelPump for_this_route( static_cast<double>((station_spacing*current_station_id)) / current_fuel);
				stations.at(current_station_id).add_fuel_pump(for_this_route);

				++current_station_id;
				current_fuel -= 2 * station_spacing;
			}
		}

		// Run the trains to gen fuel prices
		int start_position = station_spacing;
		int start_station_id = 1;
		while (start_position < length) {
			//int current_train_pos = start_position;
			int current_station_id = start_station_id;
			double current_fuel = train_capacity;
			double price_for_full_load = stations.at(current_station_id).get_best_price() * train_capacity;

			++current_station_id;
			current_fuel -= 2 * station_spacing; // 2x because we have to make it back to the previous station to refuel too
			while (current_fuel > 0 && current_station_id < stations.size()) {
				// If the train dumped all its fuel right now, what would the station it creates look like?
				FuelPump for_this_route(price_for_full_load / current_fuel);
				stations.at(current_station_id).add_fuel_pump(for_this_route);

				++current_station_id;
				current_fuel -= 2 * station_spacing;
			}

			++start_station_id;
			start_position += station_spacing;
		}
	}
	std::vector<Price> get_fuel_prices() const {
		std::vector<Price> prices;
		for (const auto& station : stations) {
			Price p;
			p.position = station.get_position();
			p.price = station.get_best_price();
			prices.push_back(p);
		}
		return prices;
	}
	double get_travel_cost_for_distance(const int distance) const {
		// In case we can do the trip without refueling
		if (distance < train_capacity) {
			return distance;
		}
		// TODO this is all bad
		// Assume that the last stretch will be a whole tank
		double fuel_cost = 0.0;
		const int last_stop_pos = distance - train_capacity;
		const auto stop_id = position_to_id(last_stop_pos);
		const auto fuel_price = stations.at(stop_id).get_best_price();
		fuel_cost += fuel_price * train_capacity;

		if (last_stop_pos > train_capacity) {
			throw std::exception("Cannot reach last stop");
		}

		return fuel_cost + distance;
	}
private:
	size_t position_to_id(int position) const {
		return position / station_spacing;
	}
	int length;
	int station_spacing;
	int train_capacity;
	std::vector<Station> stations;
};

int main() {
	RailLine r(1200, 100, 500);
	for (const auto& price : r.get_fuel_prices()) {
		std::cout << "Dist: \t" << price.position << " \tPrice: \t" << price.price << std::endl;
	}
	int travel_distance = 600;
	std::cout << "Best trip cost for distance: " << travel_distance << " is " << r.get_travel_cost_for_distance(travel_distance) << std::endl;
}
