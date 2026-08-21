/**___________________________________________________________________________/
@file          Profiler.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Profiler implementation.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "pch.h"
#include "Profiler.h"

Profiler Profiler::instance; // singleton instance

Profiler& Profiler::Instance() { return instance; }


void Profiler::Start(const std::string& name) {

	if (name == "STARTFRAME") {
		currState.push("FRAME");
		prevTime.push(std::chrono::high_resolution_clock::now());
		std::for_each(MinuteStore.begin(), MinuteStore.end(), [this](std::pair<const std::string, TimeStore>& d) { d.second.storage.push_back(0.); });
	}
	else if (name == "ENDFRAME") {
		End();
		// Additional logic
		std::for_each(data.begin(), data.end(), [this](std::pair<const std::string, ProfileData>& d) {
			TimeStore& ts = MinuteStore[d.first];
			d.second.currentTime = ts.storage.back();
			d.second.minuteAverage = ts.totalTime / ts.storage.size();
			AverageStore[d.first].push_back(d.second.minuteAverage);

			double front{ -1. };

			if (ts.totalTime > logTime) {
				front = ts.storage.front();
				ts.storage.pop_front();
				ts.totalTime -= front;
				AverageStore[d.first].pop_front();
			}

			std::multiset<double>& ts99 = PercentStore[d.first].first;
			std::multiset<double>& ts95 = PercentStore[d.first].second;

			ts99.insert(ts.storage.back());
			ts95.insert(ts.storage.back());


			if (ts99.find(front) != ts99.end()) {
				ts99.erase(ts99.find(front));
			}
			if (ts95.find(front) != ts95.end()) {
				ts95.erase(ts95.find(front));
			}

			if (ts99.size() > 2){
				while (ts99.size() > ts.storage.size() * 0.99) {
					ts99.erase(ts99.find(*ts99.rbegin()));
				}
				while (ts95.size() > ts.storage.size() * 0.95) {
					ts95.erase(ts95.find(*ts95.rbegin()));
				}

				d.second._95p = *ts95.rbegin();
				d.second._99p = *ts99.rbegin();
			}
			});
	}
	else if (name == "END") {
		End();
	}
	else {
		currState.push(name);
		prevTime.push(std::chrono::high_resolution_clock::now());
	}
}

void Profiler::End() {

	Time t = std::chrono::high_resolution_clock::now();
	double interval = MiliCast(prevTime.top(), t);

	TimeStore& ts = MinuteStore[currState.top()];

	if (ts.storage.empty()) ts.storage.push_back(0.);
	if (data.find(currState.top()) == data.end()) {
		data[currState.top()].avgs = &AverageStore[currState.top()];
		data[currState.top()].ts = &ts;
	}

	ts.storage.back() += interval;
	ts.totalTime += interval;


	currState.pop();
	prevTime.pop();
}