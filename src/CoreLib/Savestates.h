//#pragma once
///**___________________________________________________________________________/
//@file          Savestates.h
//@author        j.junbo@digipen.edu
//@date          9/29/2025
//
//This saves the "state" of the frame for the last 5 seconds of the game loop.
//This is to allow back stepping of frames for when a bug appears values can 
//be examined on runtime as you backstep.
//Also supports forward stepping of frames.
//Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
//
///*____________________________________________________________________________*/
////#include "Registry.h"
//#include <deque>
//#include <map>
//#include <memory>
//#include <typeinfo>
//#include <functional>
//#include <typeindex>
//#include <array>
//#include <chrono>
//#include <vector>
//#include <algorithm>
//#include "Registry.h"
//#include "Platform.h"
//
//class Savestate {
//
//	
//	using componentMap = std::unordered_map<std::type_index, std::unique_ptr<IComponentStorage>>;
//	using registryStore = std::pair<EntityRegistry, componentMap>;
//	using Time = std::chrono::steady_clock::time_point;
//
//public:
//	Savestate() = default;
//
//	Savestate(const Savestate&) = delete;
//	Savestate& operator=(const Savestate&) = delete;
//
//	/*!
//	* \brief
//	*   Initializes the savestate's pointer to the registry
//	*
//	* \param [Registry*] pointer to the registry
//	*/
//	void InitRegistry(Registry* r);
//
//	/*!
//	* \brief
//	*   Updates the internal buffer every frame
//	*/
//	void Update();
//
//	/*!
//	* \brief
//	*   Sets the debug flag to true to pause the game by setting accumulator to 0
//	*/
//	void DebugPause() { debug = true; }
//
//	/*!
//	* \brief
//	*   The mechanism that handles debug pausing. Does nothing normally. 
//	*	If the debug flag is set, it sets the accum to 0. Then based on
//	*	the forward and the step flag it does operations to step the game
//	*	forward/backwards frame by frame
//	*
//	* \param [float&] the loop accumulator
//	*/
//	void TickControl(float& accum);
//
//	/*!
//	* \brief
//	*   Sets the internal flags to step forward
//	*/
//	void StepForward() { forward = true; step = true; }
//
//	/*!
//	* \brief
//	*   Sets the internal flags to step backwards
//	*/
//	void StepBackward() { forward = false; step = true; }
//
//	/*!
//	* \brief
//	*   Does the stepping forward at a set rate when the button is held down
//	*/
//	void StepForwardSlow();
//
//	/*!
//	* \brief
//	*   Does the stepping backward at a set rate when the button is held down
//	*/
//	void StepBackwardSlow();
//
//	/*!
//	* \brief
//	*   Set how fast the rate of the button held update is
//	*/
//	void SetSlowmodeRate();
//
//	/*!
//	* \brief
//	*   Go to a frame within the internal buffer
//	*/
//	void GotoFrame();
//
//	/*!
//	* \brief
//	*   Starts from the original paused frame.
//	*	If step backwards operations have been done, this effectively
//	*	skips forwards.
//	*/
//	void StartOriginal();
//
//	/*!
//	* \brief
//	*   Starts from the current paused frame.
//	*/
//	void StartFromFrame();
//
//	/*!
//	* \brief
//	*   Copies the current state into a fixed size array buffer
//	*/
//	void CopyState(int i);
//
//	/*!
//	* \brief
//	*   Loads active states that are stored in the fixed size array buffer
//	*/
//	void LoadState(int i);
//
//	/*!
//	* \brief
//	*   Clears the underlying storage storing the frame data
//	*/
//	void ClearState();
//
//	/*!
//	* \brief
//	*   A template function to add a function pointer that copies what is stored within the
//	*	IComponentStorage derived class by value into a new unique_ptr object. It is effectively
//	*	copying whatever that is stored within the unique pointer by value.
//	*	The template function is required as there is no other way to tell from a base class pointer
//	*	what is the derived class.
//	*/
//	template <typename T>
//	void AddComponentCast() {
//#ifdef PLATFORM_WINDOWS
//		std::cout << typeid(T).name() << '\n';
//		ComponentCast[std::type_index(typeid(T))] = [](IComponentStorage* ic) -> std::unique_ptr<IComponentStorage> {
//			return std::make_unique<T>(*static_cast<T*>(ic));
//			};
//#endif
//	}
//
//	/*!
//	* \brief
//	*   A template function to remove a function pointers when hot reload is triggered, as the hotreloaded
//	*	types become invalid after the hotreload (their properties may change)
//	*/
//	template <typename T>
//	void RemoveComponentCast() {
//		std::cout << "Removing: " << typeid(T).name() << '\n';
//		ComponentCast.erase(std::type_index(typeid(T)));
//	}
//
//	/*!
//	* \brief
//	*   Sets the frame pointer to param
//	* 
//	* \param [const int&] frame pointer value
//	*/
//	void SetFramePointer(const int&);
//
//	/*!
//	* \brief
//	*   Load the copy of the registry data currently pointed
//	*	to by the frame pointer
//	*/
//	void LoadCopy();
//
//	/*!
//	* \brief
//	*   Copies the current state of the registry
//	*/
//	void CopyRegistry();
//
//	/*!
//	* \brief
//	*   Toggles if savestates is enabled or not.
//	*	Default is inactive
//	*/
//	void ToggleEnable() { enabled = !enabled; }
//
//	/*!
//	* \brief
//	*   Runs a stored function pointer based on the index
//	*
//	* \param [int] the index to the function
//	*/
//	void RunFunction(size_t i) { if (i >= FuncPtrStore.size()) return;  FuncPtrStore[i](); }
//
//	/*!
//	* \brief
//	*   Runs all stored function pointers
//	*/
//	void RunFunctions() { std::for_each(FuncPtrStore.begin(), FuncPtrStore.end(), [](std::function<void(void)>& f) { f(); }); }
//
//	/*!
//	* \brief
//	*   A template function to store a function pointer to a class member function. 
//	*	This exists for the case where we need to run a function when the debug pause 
//	*	is active.
//	*/
//	template <typename Inst, typename Func, typename... Args>
//	int AddClassFunction(Func f, Inst i, Args&... a) {
//		int ii = FuncPtrStore.size();
//		FuncPtrStore.emplace_back(std::bind(f, i, std::reference_wrapper<Args>(a)...));
//		return ii;
//	}
//
//	/*!
//	* \brief
//	*   A template function to store a function pointer a stamdard function.
//	*	This exists for the case where we need to run a function when the debug pause
//	*	is active.
//	*/
//	template <typename Func, typename... Args>
//	size_t AddFunction(Func f, Args&... a) {
//		size_t i = FuncPtrStore.size();
//		FuncPtrStore.emplace_back(std::bind(f, std::reference_wrapper<Args>(a)...));
//		return i;
//
//	}
//
//
//private:
//	/*!
//	* \brief
//	*   Performs the copy operation on the component map within registry
//	* 
//	* \param [componentMap&] the registry's component map
//	* 
//	* \out [componentMap] a copied component map
//	*/
//	componentMap Copy(componentMap& r);
//
//	// The key is based on the typeid of the derived class (typeid is able to tell the derived class from a base class pointer), then the corresponding function pointer to cast the base class pointer
//	// to the derived class pointer to copy the unique pointer data by value.
//	std::map<std::type_index, std::function<std::unique_ptr<IComponentStorage>(IComponentStorage*)>> ComponentCast;		 
//
//	std::deque<registryStore> StateStore;													// Stores the 5 seconds of registry states
//	std::array<std::pair<bool, registryStore>, 6> FixedStateStore;							// Fixed size array able to store state from any point in time. Index 0 is reserved for editor play button
//	std::vector<std::function<void(void)>> FuncPtrStore;									// Storage of funciton pointers for the case where we need to run a function during debug pause
//	Registry* registry{ nullptr };															// pointer to the registry
//	bool debug{ false }, step{ false }, forward{ true }, print{ false }, enabled{ false };	// Flags used to determine what TickControl should do
//	int framePointer{ 0 };																	// Points to the current frame you are seeing within StateStore.
//	int slowmodeRate{ 1000000 / 30 };														// The 1 million is hardcoded as std::chrono returns time as a size_t so conversion to framerate has to include this hardcoded value.
//};