#pragma once
/**___________________________________________________________________________/
@file          Errorlog.h
@author        j.junbo@digipen.edu
@date          9/29/2025

This contains the class definition of the error logger. It is a singleton class
which snapshots the values variables hold within a function before said function
goes out of scope.
It also holds the capability to print out the values of the variables into either
a file or std::cout from the latest snapshot.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#define ERROR_SNAPSHOT

#include <string>
#include <any> // wait this is just python now
#include <map>

#include <functional> // for any printing 
#include <iostream>
#include <typeinfo>

/*!
* \brief
*   Error Logging singleton class. Usage:
* \brief
*	1: (Optional) SetState to set a general state that the game loop is under, i.e. Draw, Update etc.
* \brief
*	2: SetFunction within a function to start the logging process
* \brief
*	3: Create a variable of type ErrorLog::Catch initialized with the same name as the function. Make 
*	sure to initialize this after everything else within the function (last to be initialized)
* \brief
*	4: Log() each individual variable you want to track
* \brief
*	done! yippie!
*/
class ErrorLog {
private:
	ErrorLog() = default;
	static ErrorLog instance;

public:

	/*!
	* \brief
	*   Getter for the error logger instance
	*
	* \param null
	*
	* \return 
	*	[ErrorLog&] The singleton instance of the error logger
	*/
	static ErrorLog& Instance();


	ErrorLog(const ErrorLog&) = delete;
	ErrorLog& operator=(const ErrorLog&) = delete;


	/*!
	* \brief
	*   Used for logging variables.
	* \brief
	*	Name is just what the variable called within the
	*	code, so int bruh{5}; will have name "bruh"
	*
	* \param
	*    [std::string&] name of the variable
	* \param
	*    [T*] pointer to the variable
	*
	* \return null
	*/
	template<typename T>
	void Log(const std::string& name, T* var) { storage[function][name] = var; }

	/*!
	* \brief
	*   Overload for logging pointer variables.
	* \brief
	*	Name is just what the variable called within the
	*	code, so int bruh{5}; will have name "bruh"
	*
	* \param
	*    [std::string&] name of the variable
	* \param
	*    [T**] pointer to the pointer variable
	*
	* \return null
	*/
	template<typename T>
	void Log(const std::string& name, T** var) { storage[function][name] = var; }

	/*!
	* \brief
	*   Used to print out the latest snapshot
	*
	* \param null
	*
	* \return null
	*/
	void PrintLog();

	/*!
	* \brief
	*   Used to set the "state" of the logger. This 
	*	is used to cover a group of function like physics
	*	or collision
	*
	* \param
	*    [std::string&] name of the state
	* 
	* \return null
	*
	*/
	void SetState(const std::string& s) { state = s; }

	/*!
	* \brief
	*   Used within the function to set up the error logging within the function
	* \brief
	*  MUST BE CALLED BEFORE LOG()
	*
	* \param 
	*	[std::string&] name of the function
	*
	* \return null
	*/
	void SetFunction(const std::string& s) { function = s; }

	/*!
	* \brief
	*   Used to set the output location of the print. 
	*	Can also give an emtpy string to let it output to
	*	std::cout
	*
	* \param 
	*	[std::string&] Output location for the printing
	*
	* \return null
	*/
	void SetOutput(const std::string& s) { outputfile = s; }


	/*!
	* \brief
	*   Used to add a print function for custom types. If any clarification is needed, check with junbo
	*
	* \param 
	*	[Func&] The printing function. It will have a structure:  void func(type t) { // function block // }
	* \param
	*	or in lambda form: [](type t) { // function block // }
	* \param
	*	The function block will be where you define how you print your custom type, this is an example with an 
	*	already implemented primitive int: AddAnyPrint<int>( [] (int i) { std::cout << "Type: int, Value: " << i; });
	* \param
	*	[bool = false] Set this parameter to be true if you are logging a pointer to your custom type, aka I have to
	*	store a T**.
	*
	* \return null
	*/
	template <typename T, typename Func>
	void AddAnyPrint(const Func& f, bool pointer = false) {
		if (AnyPrint.find(hash(T())) == AnyPrint.end()) {
			AnyPrint.insert(AnyPrintInit<T>(f));
			AnyDowncast.insert(AnyDowncastInit<T*>([](T* t) -> std::any { if (t) return *t; else throw 0; }));
			if (pointer)
				AnyDowncast.insert(AnyDowncastInit<T**>([](T** t) -> std::any { if (t) return *t; else throw 0; }));
		}
	}

	/*!
	* \brief
	*   Dummy struct used for error logging. 
	*	MAKE SURE TO DECLARE THIS VARIABLE AFTER EVERYTHING ELSE
	*/
	struct Catch {
		Catch() = delete;

		/*!
		* \brief
		*   Make sure to use the exact same name as you used for SetFunction()
		* \brief
		*	Case sensitive.
		*
		* \param 
		*	[std::string&] the name of the function.
		*
		* \return null
		*/
		Catch(std::string s) : func{ s } {}
		~Catch() { ErrorLog::Instance().SnapshotVariables(func); }

		std::string func;
	};

private:
	
	/*!
	* \brief
	*   Used to snapshot the variables
	*/
	void SnapshotVariables(const std::string& s);

	std::string state{};
	std::string function{};
	std::string outputfile{"Assets/error.txt"}; // default output

	std::map<std::string, std::map<std::string, std::any>> storage{}; // stores pointers to the variables we are tracking
	std::map<std::string, std::any> snapshot{}; // snapshot of the variables before the function dieded

	// To allow Catch to call the snapshot variables on destruction
	friend struct Catch;

	//--------- Helper functions for any shenanigans ---------------//
	// You cannot hide template definitions within cpp files, so they have to be brought forward here unfortunately. 
	// Hopefully this doesnt add too much bloat

	/*!
	* \brief
	*	Used to have a Key for the type the any holds within the map
	*/
	static size_t hash(const std::any& a) { return a.type().hash_code(); }

	/*!
	* \brief
	*   Helper function to print std::any for a particular type
	*/
	template <typename T, class Func>
	static std::pair<size_t, std::function<void(const std::any&)>> AnyPrintInit(const Func& f) {
		try {
			return {
				hash(T()),
				[=](const std::any& a) {
					f(std::any_cast<const T&>(a));
				}
			};
		}
		catch (const std::bad_any_cast& e) {
			std::cout << e.what() << '\n';
			throw 0;
		}
	}

	/*!
	* \brief
	*	Helper function to downcast std::any for a particular type
	*/
	template <typename T, class Func>
	static std::pair<size_t, std::function<std::any(const std::any&)>> AnyDowncastInit(const Func& f) {
		try {
			return {
				hash(T()),
				[=](const std::any& a) {
					return f(std::any_cast<const T&>(a));
				}
			};
		}
		catch (const std::bad_any_cast& e) {
			std::cout << e.what() << '\n';
			throw 0;
		}
	}

	static std::unordered_map<size_t, std::function<void(const std::any&)>> AnyPrint;
	static std::unordered_map<size_t, std::function<std::any(const std::any&)>> AnyDowncast;
};