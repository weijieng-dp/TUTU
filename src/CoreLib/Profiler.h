#pragma once
/**___________________________________________________________________________/
@file          Profiler.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Basic profiler to find out how long a function takes to run

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include <string>
#include <map>
#include <deque>
#include <chrono>
#include <set>
#include <stack>


// If enable profiler is defined, then the functionality of profiling functions will work
#ifdef ENABLE_PROFILER__
/*!
* \brief
*   Macro to reduce ifdef bloat in main
* \param
*	A string literal or a std::string would work. Leave param empty to signify the end
*/
#define PROFILE_FUNCTION__(...)		Profiler::Instance().Start(__VA_ARGS__);
#else
#define PROFILE_FUNCTION__(...) // do nothing
#endif

/*!
* \brief
*   Function profiler to know how long they take to run. Usage:
* \brief
*	Use PROFILE_FUNCTION__( name ) to start the profiling
* \brief 
*   And end with PROFILE_FUNCTION__()
* \brief 
*	use Profile() to get the stored data
*/
class Profiler {
private:
	Profiler() = default;
	static Profiler instance;
	struct TimeStore;
public:
	struct ProfileData; // forward declaration
public:

	/*!
	* \brief
	*   Getter for singleton instance
	*
	* \param null
	* 
	* \return
	*	The instance for the singleton
	*/
	static Profiler& Instance();

	~Profiler() = default;
	Profiler(const Profiler&) = delete;
	Profiler& operator=(const Profiler&) = delete;

	/*!
	* \brief
	*   Start profiling the function. If called after a previous Start() call, it will effectively
	*	call End() and then Start().
	*
	* \param
	*	[std::string&] name of the function
	* 
	* \return null
	*/
	void Start(const std::string& s = "END");

	/*!
	* \brief
	*	Stop profiling the function
	*
	* \param null
	*
	* \return null
	*/
	void End();

	/*!
	* \brief
	*   Getter for underlying data storage
	*
	* \param null
	*
	* \return
	*	[std::map<std::string, ProfileData>&] the data storing the time it takes each function profiled
	*/
	const std::map<std::string, ProfileData>& Profile() const noexcept { return data; }

	/*!
	* \brief
	*   Setter for how long to store. Default is 60 000. (60s for 1 min)
	*
	* \param [double] the new value to set it to
	*/
	void SetLogTime(double d) { logTime = d; }


	/*!
	* \brief
	*   Simple struct containing 4 values.
	* \brief 
	*	minuteAverage stores the 1 minute running average
	* \brief
	*	currentTime stores the most recent time taken for the function to run
	* \brief
	*	_99p is the minuteAverage but we discard the bottom 1% of the values
	* \brief
	*	_95p is a similar concept but we discard the bottom 5% this time
	*/
	struct ProfileData {
		double minuteAverage;	// 1 minute average of how long the function took
		double currentTime;		// The time it took to run for the current iteration, in ms
		double _99p;			// The longest amount of time taken 99% of the time
		double _95p;			// The longest amount of time taken 95% of the time
		TimeStore* ts;			
		std::deque<double>* avgs;
	};

private:
	// chrono is so painfully long icant bro
	using Time = std::chrono::steady_clock::time_point;

	/*!
	* \brief
	*   Helper function to turn std::chrono's time to a double
	*
	* \param 
	*	[Time&] the first timepoint
	* \param
	*	[Time&] the second (later) timepoint
	*
	* \return
	*	[double] the amount of time passed in miliseconds, in double
	*/
	double constexpr MiliCast(const Time& t1, const Time& t2) { return std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - t1).count() / 1000000.; }


	/*!
	* \brief
	*   Simple struct to store time data
	*/
	struct TimeStore {
		std::deque<double> storage;
		double totalTime{ 0. };
	};

private:

	std::map<std::string, ProfileData> data;
	
	// This could eat up alot of heap space potentially so lower the maximum store values if needed
	std::map<std::string, TimeStore> MinuteStore;
	std::map<std::string, std::deque<double>> AverageStore;		// running average
	std::map<std::string, std::pair<std::multiset<double>, std::multiset<double>>> PercentStore;

	std::stack<std::string> currState;
	std::stack<Time> prevTime{};

	double logTime{ 60000. }; // 60s aka 1 min of logging
};