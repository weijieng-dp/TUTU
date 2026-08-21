/*!
@file       UpdateStatViewScript.cpp
@author     Kaeden Tan (kaedenjiawei.tan)
@date       05/04/2026

@brief      implements the UpdateStatViewScript class. Displays current player stats
			(movement speed multiplier, attack speed, projectile damage) as a UI text element.
*/


#include "UpdateStatViewScript.h"
#include "../CoreLib/StatsManager.h"
#include <sstream>
void UpdateStatViewScript::OnStart(Registry& )
{
};


void UpdateStatViewScript::OnUpdate(Registry& registry, float, bool)
{
	StatsManager* stat = CEO::Get<StatsManager>();
	std::ostringstream ss{};
	std::string str{};
	
	ss << std::fixed << std::setprecision(1) << stat->movementSpeed.Get()/ stat->movementSpeed.Base();
	str += "x" + ss.str() +"\n";
	ss.str("");
	ss.clear();

	ss << std::fixed << std::setprecision(1) << stat->attackSpeed.Get();
	str += ss.str() + "/s\n";
	ss.str("");
	ss.clear();

	str += std::to_string((int)stat->projectileDamage.Get()) + "/shot\n";

	GetComponent<TextRendererComponent>(registry)->text = str;
};

void UpdateStatViewScript::OnFixedUpdate(Registry&, float, bool)
{
};
