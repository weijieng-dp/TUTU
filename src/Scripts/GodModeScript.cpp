/*!
@file       GodModeScript.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       05/04/2026

@brief      Implements the GodModeScript class. Toggles god mode via button click and updates button sprite accordingly.
*/

#include "GodModeScript.h"
#include "../CoreLib/StatsManager.h"
void GodModeScript::OnStart(Registry & r)
{
	GetComponent<ButtonComponent>(r)->onClick = [&]() {
		if (CEO::Get<StatsManager>()->GodMode()) {
			GetComponent<SpriteRendererComponent>(r)->texture = on;
		}
		else {
			GetComponent<SpriteRendererComponent>(r)->texture = off;
		}
		};
};


void GodModeScript::OnUpdate(Registry & , float , bool )
{
};

void GodModeScript::OnFixedUpdate(Registry&, float, bool)
{
};
