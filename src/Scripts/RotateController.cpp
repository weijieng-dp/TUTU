/*!
@file       RotateController.cpp
@author     Ng Wei Jie (weijie.ng) 100%
@date       06/11/2025
@brief		Implements the RotateController script which allows entities to
            rotate based on user input. The script modifies the rotation value
            of the TransformComponent each frame based on key presses (Z, X).

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/


#include "RotateController.h"
#include "../CoreLib/InputManager.h"


/*!
* \brief
*	Called once when the script is initialized. Logs startup information and
*	sets up any necessary rotation state for the entity.
* \param
*	registry - ECS registry instance used to access entity components
*/
void RotateController::OnStart(Registry& )
{


};

/*!
* \brief
*	Per-frame update that handles rotation input and applies rotation changes
*	to the entityÅfs TransformComponent. Pressing Z rotates clockwise while
*	X rotates counterclockwise.
* \param
*	registry - ECS registry instance used to access entity components
* \param
*	dt - delta time between frames
* \param
*	firstframe - true if this is the first frame after startup
*/
void RotateController::OnUpdate(Registry& ,float , bool)
{
    // std::cout << dt <<std::endl;


};

/*!
* \brief
*	Called at a fixed timestep for time-independent rotation updates.
*	This implementation currently performs no fixed updates.
* \param
*	registry - ECS registry instance used to access entity components
* \param
*	dt - fixed timestep delta
* \param
*	firstframe - true if this is the first fixed update after startup
*/
void RotateController::OnFixedUpdate(Registry& ,float , bool )
{
};
