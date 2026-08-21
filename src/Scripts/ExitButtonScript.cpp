/*!
@file       ExitButtonScript.cpp
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Implementation of exit button functionality. Sets up the button's click
event to trigger application exit.

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/

#include "ExitButtonScript.h"
#include <functional>
#include "../CoreLib/Application.h"

void ExitButtonScript::OnStart(Registry& registry)
{
	ButtonComponent& button = *GetComponent<ButtonComponent>(registry);
	
	// binding a non-member/static function that takes parameters
	button.onClick = std::bind(Application::SignalExit);					
};
void ExitButtonScript::OnUpdate(Registry& ,float , bool ){/*empty by design*/ };
void ExitButtonScript::OnFixedUpdate(Registry& ,float , bool ){/*empty by design*/ }