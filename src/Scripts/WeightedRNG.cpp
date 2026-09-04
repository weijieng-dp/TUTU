#include "WeightedRNG.h"

std::unordered_map<std::string, int>	WeightedRNG::named_cache;
std::mt19937							WeightedRNG::gen{ std::random_device{}() };
WeightedRNG								WeightedRNG::instance;

int WeightedRNG::getRand(const std::string& s) {

	if (named_cache.find(s) == named_cache.end()) named_cache[s] = 100;

	int& weight = named_cache[s];

	std::uniform_int_distribution<int> dist(1, 200);

	if (dist(gen) <= weight) {
		if (weight >= 100) weight = 50;
		else weight /= 2;

		return 0;
	}
	else {

		if (weight >= 100) weight += (200 - weight) / 2;
		else weight = 150;

		return 1;
	}
}