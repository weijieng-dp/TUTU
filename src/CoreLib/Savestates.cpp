///**___________________________________________________________________________/
//@file          SaveStates.h
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
//#include "pch.h"
//#include "Savestates.h"
//
//namespace {
//	constexpr int MaxStoreCount{ 300 };
//}
//
//
//void Savestate::InitRegistry(Registry* r) {
//	registry = r;
//}
//
//void Savestate::CopyRegistry() {
//	if (!registry) return; // unintialized
//	StateStore.emplace_front(registry->entityRegistry, Copy(registry->componentStorages));
//}
//
//void Savestate::LoadCopy() {
//	if (!registry) return; // unintialized
//	if (StateStore.empty()) return;
//
//	registry->DestroyAllEntities();
//	registry->ClearAllComponentStorages();
//	registry->RestartEntityCount();
//
//	registry->entityRegistry = StateStore[framePointer].first;
//	registry->componentStorages = Copy(StateStore[framePointer].second);
//}
//
//Savestate::componentMap Savestate::Copy(componentMap& compMap) {
//	componentMap state;
//#ifdef PLATFORM_WINDOWS
//	for (auto& p : compMap) {
//		state.emplace(p.first, ComponentCast[std::type_index(typeid(*p.second.get()))](p.second.get()));
//	}
//#endif
//	return state;
//}
//
//void Savestate::Update() {
//	if (!enabled) return;
//
//	if (!debug || (framePointer == 0 && step)) {
//		if (StateStore.size() > MaxStoreCount) StateStore.pop_back();
//		CopyRegistry();
//	}
//
//	if (step) step = false;
//}
//
//void Savestate::TickControl(float& accum) {
//	if (!debug) return;
//
//	if (!print) {
//		std::cout << "Currently pointing to frame: -" << std::setw(3) << std::setfill('0') << framePointer << std::endl;
//		print = true;
//	}
//
//	RunFunction(1);
//
//	if (step) {
//		accum = 1 / 59.f; // hackey ass shit, we need to store the fps somewhere
//		if (forward) {
//			if (framePointer) {
//				LoadCopy();
//				framePointer--;
//			}
//			std::cout << "\b\b\b" << std::setw(3) << std::setfill('0') << framePointer;
//		}
//		else {
//			// backwards stepping implementation
//			if (framePointer < StateStore.size() - 2) {
//				framePointer += 2; // 2 because we are updating the loaded frame to show us the frame right before the curr frame
//				LoadCopy();
//				framePointer--; // kind of a jank setup
//				std::cout << "\b\b\b" << std::setw(3) << std::setfill('0') << framePointer;
//			}
//			else {
//				std::cout << "\nEnd of stored frames\n";
//				print = false;
//				step = false;
//				accum = 0.f;
//			}
//		}
//	}
//	else {
//		accum = 0.f;
//	}
//}
//
//void Savestate::StepForwardSlow() {
//	static Time prevTime = std::chrono::high_resolution_clock::now();
//	Time currTime = std::chrono::high_resolution_clock::now();
//
//	if (std::chrono::duration_cast<std::chrono::microseconds>(currTime - prevTime).count() > slowmodeRate) {
//		StepForward();
//		prevTime = currTime;
//	}
//}
//
//void Savestate::StepBackwardSlow() {
//	static Time prevTime = std::chrono::high_resolution_clock::now();
//	Time currTime = std::chrono::high_resolution_clock::now();
//
//	if (std::chrono::duration_cast<std::chrono::microseconds>(currTime - prevTime).count() > slowmodeRate) {
//		StepBackward();
//		prevTime = currTime;
//	}
//}
//
//void Savestate::SetSlowmodeRate() {
//	std::string s;
//	std::cout << "\nSet the slowmode rate (as an integer representing fps): ";
//	std::getline(std::cin, s);
//	try {
//		int i = std::stoi(s);
//		if (i > 60) std::cout << "Not really slowmode now eh? Might not even be able to update that fast\n";
//		slowmodeRate = 1000000 / i;
//	}
//	catch (...) {
//		std::cout << "Please Enter an Integer!!!\n";
//	}
//	print = false;
//}
//
//void Savestate::GotoFrame() {
//	std::string s;
//	std::cout << "\nEnter the frame you wish to go to: ";
//	std::getline(std::cin, s);
//
//	try {
//		int i = std::stoi(s);
//		if (i >= StateStore.size()) {
//			std::cout << "There is currently only " << StateStore.size() << " frames being stored, please enter a value between 0 and " << StateStore.size() - 1 << '\n';
//		}
//		else {
//			framePointer = i + 1;
//			LoadCopy();
//			forward = true;
//			step = true;
//		}
//	}
//	catch (...) {
//		std::cout << "Please Enter an Integer!!!\n";
//	}
//	print = false;
//}
//
//void Savestate::StartOriginal() {
//	framePointer = 0;
//	LoadCopy();
//	std::cout << std::endl;
//	debug = false;
//	print = false;
//}
//
//void Savestate::StartFromFrame() {
//	if (framePointer != 0) {
//		// remove everything infront of current frame pointer
//		StateStore.erase(StateStore.begin(), StateStore.begin() + framePointer);
//	}
//	framePointer = 0;
//	LoadCopy();
//	std::cout << std::endl;
//	debug = false;
//	print = false;
//}
//
//void Savestate::CopyState(int i) {
//	std::cout << "Savestate " << i << " Saved. Press ctrl + shift + " << i << " to load the state.\n";
//	FixedStateStore[i].first = true;
//	FixedStateStore[i].second.first = StateStore[0].first;
//	FixedStateStore[i].second.second = Copy(StateStore[0].second);
//}
//
//void Savestate::LoadState(int i) {
//	if (!FixedStateStore[i].first) {
//		std::cout << "Savestate " << i << " not saved yet. Press ctrl + " << i << " to save a state at that time\n";
//		return;
//	}
//	std::cout << "Savestate " << i << " loaded.\n";
//	StateStore.clear();
//	framePointer = 0;
//	StateStore.emplace_back(FixedStateStore[i].second.first, Copy(FixedStateStore[i].second.second));
//	LoadCopy();
//
//	if (debug) {
//		print = false;
//		forward = true;
//		step = true;
//	}
//}
//
//void Savestate::ClearState() {
//	if (!StateStore.empty())
//		StateStore.clear();
//	for (auto& p : FixedStateStore) {
//		p.first = false;
//	}
//	framePointer = 0;
//}
//
//void Savestate::SetFramePointer(int const& i) {
//	framePointer = i;
//}