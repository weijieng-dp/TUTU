/*!
@file       NativeScriptLoader.h
@author     Ng Wei Jie (weijie.ng) 100%
@date       06/11/2025
@brief		Declares the NativeScriptLoader interface responsible for registering,
			initializing, and clearing native script components within the ECS.
			This header provides macros and helper templates used by generated
			files to bind script classes at runtime.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/


#pragma once
#include "../CoreLib/Registry.h"
#include "../CoreLib/Components.h"
#include "../CoreLib/InputManager.h"
#include "../CoreLib/Savestates.h"
#include "../CoreLib/CEO.h"

#ifndef PLATFORM_ANDROID
#ifdef NativeScripting
    #define NativeScriptingComponent_API __declspec(dllexport)
#else
    #define NativeScriptingComponent_API __declspec(dllimport)
#endif
#endif

/*!
* \brief
*	Registers and reflects a native script component type. This macro both
*	registers the component with the ECS and initializes its reflection info.
*/
#define RegisterNativeComponentMacro(type)\
 registerNativeComp<type>(registry, componentRegistry, std::string(refl::reflect<type>().name)); \
 rtr::TypeInfo::Init<type>();


/*!
* \brief
*	Registers a native script component type T into the ECS and reflection
*	systems, defining how it is created and destroyed.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
* \param
*	name - name of the component type for reflection and runtime registration
*/

template<typename T>
void registerNativeComp(Registry& registry, ComponentRegistry& componentRegistry, std::string name)
{
	componentRegistry.RegisterComponent<T>(
		name,

		// OnCreate
		[&registry, name](EntityRegistry::Entity entity) {
			if (!registry.GetComponent<NativeScriptingComponent>(entity))
			{
				registry.AddComponent<NativeScriptingComponent>(entity, {});
			}
			auto nsc = registry.GetComponent<NativeScriptingComponent>(entity);
			nsc->Bind<T>(name);
			nsc->scripts[name].OnCreateScript(nsc->scripts[name]);
			registry.AddComponent<T>(entity, {});
		},

		// OnDelete
		[&registry, name](EntityRegistry::Entity entity) {
			auto nsc = registry.GetComponent<NativeScriptingComponent>(entity);
			if (nsc->scripts.find(name) != nsc->scripts.end()) {
				nsc->scripts.find(name)->second.OnDeleteScript(nsc->scripts[name]);
				nsc->scripts.erase(nsc->scripts.find(name));
				registry.RemoveComponent<T>(entity);
				if (nsc->scripts.size() == 0)
				{
					registry.RemoveComponent<NativeScriptingComponent>(entity);

				}
			}
		}
	);
	//CEO::Instance().GetManager<Savestate>()->AddComponentCast<ComponentStorage<T>>();

};

#ifdef PLATFORM_ANDROID
/*!
* \brief
*	Initializes all native script components on Android platforms, linking
*	global systems such as Input and Savestate to the ECS.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
* \param
*	typeMap - pointer to shared reflection type map
* \param
*	Input - pointer to Input system instance
* \param
*	ss - optional pointer to Savestate instance
*/
void NativeComponentInitialise(Registry& registry, ComponentRegistry& componentRegistry,
                               std::unordered_map<std::string, rtr::TypeInfo>* typeMap, Input* Input
                                , CEO* ceo);


/*!
* \brief
*	Clears all native script components on Android platforms, unregistering
*	their reflection and ECS data.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
*/
void NativeComponentClear(Registry& registry, ComponentRegistry& componentRegistry);
#else
/*!
* \brief
*	Initializes all native script components on desktop platforms (Windows).
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
* \param
*	typeMap - pointer to shared reflection type map
* \param
*	Input - pointer to Input system instance
* \param
*	ss - pointer to Savestate instance
*/
extern "C" NativeScriptingComponent_API void NativeComponentInitialise(Registry& registry, ComponentRegistry& componentRegistry,
														std::unordered_map<std::string, rtr::TypeInfo>* typeMap, Input* Input
														, CEO* ceo);

/*!
* \brief
*	Clears and unregisters all native script components on desktop platforms.
* \param
*	registry - ECS registry instance
* \param
*	componentRegistry - global component registry
*/
extern "C" NativeScriptingComponent_API void NativeComponentClear(Registry& registry,ComponentRegistry& componentRegistry);
#endif