/*!
@file       RotateController.h
@author     Ng Wei Jie (weijie.ng) 100%
@date       06/11/2025
@brief		Declares the RotateController script class responsible for rotating
			an entity over time. The controller applies continuous rotation
			to the TransformComponent each frame or fixed update.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/

#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"
#include "../CoreLib/GameObjects.h"

/*!
* \brief
*	Applies continuous rotation to an entityÅfs TransformComponent over time.
*	The RotateController script supports both frame-based and fixed-timestep
*	rotation updates.
*/
class RotateController : public ScriptInstance
{
public:
	void BindFrom() { 
		GetComponent<RotateController>(*CEO::Get<Registry>())->entity = entity;
		*this = *GetComponent<RotateController>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<RotateController>(*CEO::Get<Registry>()) = *this; };


	/*!
	* \brief
	*	Called once when the script is initialized. Used for setting up any
	*	initial state or rotation parameters.
	* \param
	*	registry - ECS registry instance used to access entity components
	*/
	void OnStart(Registry& registry);

	/*!
	* \brief
	*	Called every frame to apply rotational updates to the entity.
	* \param
	*	registry - ECS registry instance used to access entity components
	* \param
	*	dt - delta time for frame update
	* \param
	*	firstframe - true if this is the first frame after initialization
	*/
	void OnUpdate(Registry& registry, float dt, bool firstframe);

	/*!
	* \brief
	*	Called at a fixed timestep for deterministic rotation or physics-based
	*	rotation updates. Typically used when the rotation should not depend
	*	on variable frame rates.
	* \param
	*	registry - ECS registry instance used to access entity components
	* \param
	*	dt - fixed timestep delta
	* \param
	*	firstframe - true if this is the first fixed update after startup
	*/
	void OnFixedUpdate(Registry& registry, float dt, bool firstframe);

	REFLECTABLE_PROPERTIES;
};

/*!
* \brief
*	Registers RotateController with the reflection system for runtime access.
*/
REFL_AUTO(
	type(RotateController)
)
