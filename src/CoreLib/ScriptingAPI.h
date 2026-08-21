/*!
@file       ScriptingAPI.h
@author     Ng Wei Jie (weijie.ng) 100%
@date       06/11/2025
@brief		Declares the scripting system for native C++ hot-reloadable
			components. Provides functions for scanning source files,
			generating code, compiling script DLLs, dynamically loading
			and unloading libraries, and updating script lifecycle states.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "Registry.h"
#include <iostream>
#include <string>
#include <thread>
#include <functional>
#include <fstream>
#include <regex>
#include <filesystem>
#include "InputManager.h"
#include "Savestates.h"

#define UPROPERTY
#define UCLASS

/*!
* \brief
*	Scans the specified directory for C++ header files containing classes
*	that inherit from ScriptInstance, and extracts UPROPERTY metadata for
*	code generation.
* \param
*	directoryPath - path to the directory containing script header files
*/
void  ScanDirectoryForClasses(const char*);


/*!
* \brief
*	Recompiles all native script files, regenerates the NativeScriptLoader.cpp,
*	and reloads the resulting Scripts DLL into memory at runtime.
* \param
*	directoryPath - path to scan for class definitions
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
* \param
*	typeMap - pointer to shared reflection type map
* \param
*	input - pointer to global Input manager
* \param
*	ss - pointer to Savestate instance
*/
void  RecompileScript(const char* directoryPath, Registry& registry, ComponentRegistry& componentRegistry,
	std::unordered_map<std::string, rtr::TypeInfo>* typeMap,Input* input,
	CEO* ss);

/*!
* \brief
*	Compiles the Scripts project, generates required component registration
*	code, and loads the compiled DLL into the engine at runtime.
* \param
*	directoryPath - path to scan for script class definitions
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
* \param
*	typeMap - pointer to shared reflection type map
* \param
*	input - pointer to global Input manager
* \param
*	ss - pointer to Savestate instance
*/
void  CompileScript(const char* directoryPath, Registry& registry, ComponentRegistry& componentRegistry,
	std::unordered_map<std::string, rtr::TypeInfo>* typeMap,Input* input,
	CEO* ss);

/*!
* \brief
*	Dynamically loads the compiled Scripts DLL and initializes all registered
*	native script components for use within the ECS.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
* \param
*	typeMap - pointer to shared reflection type map
* \param
*	input - pointer to global Input manager
* \param
*	ss - pointer to Savestate instance
*/
void LoadScript( Registry& registry, ComponentRegistry& componentRegistry,
	std::unordered_map<std::string, rtr::TypeInfo>* typeMap,Input* input,
	CEO* ss);

/*!
* \brief
*	Frees all loaded script components and unloads the Scripts DLL from memory.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
*/
void CreateScriptInstance(Registry& registry, ComponentRegistry& componentRegistry);

/*!
* \brief
*	Frees all loaded script components and unloads the Scripts DLL from memory.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
*/
void FreeScript(Registry& registry,ComponentRegistry& componentRegistry);

/*!
* \brief
*	Calls the OnFixedUpdate function for all active script instances at a
*	fixed timestep interval.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
* \param
*	dt - fixed timestep delta
* \param
*	firstframe - true if this is the first fixed update after initialization
*/
void FixedUpdateScriptInstance(Registry& registry, ComponentRegistry& componentRegistry, float dt, bool& firstframe);


/*!
* \brief
*	Invokes the OnUpdate function for all active script instances every frame.
*	Creates or starts scripts as needed if they have not been initialized.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
* \param
*	dt - delta time for frame update
* \param
*	firstframe - true if this is the first frame after initialization
*/
void UpdateScriptInstance(Registry& registry, ComponentRegistry& componentRegistry, float dt, bool& firstframe);

/*!
* \brief
*	Deletes all script instances from entities and clears all associated
*	component data from the registry.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
*/
void DeleteScriptInstance(Registry& registry, ComponentRegistry& componentRegistry);