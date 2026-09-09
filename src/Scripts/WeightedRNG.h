#pragma once
#include <unordered_map>
#include <string>
#include <random>

class WeightedRNG {
	static WeightedRNG instance;
public:
	
	static int getRand(const std::string&);
private:

	static std::unordered_map<std::string, int> named_cache;
	static std::mt19937 gen;
};