/**___________________________________________________________________________/
@file       ParticleScript.cpp
@author     d.lorenzoyongoyong@digipen.edu
@date       03/03/2026   (DD/MM/YYYY)
@brief      Script that synchronizes the active state of a GameObject with
            its ParticleEmitterComponent's enabled state, ensuring the object
            is only active while the particle emitter is running.
Copyright (C) 2025 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "ParticleScript.h"
void ParticleScript::OnStart(Registry &)
{
};


void ParticleScript::OnUpdate(Registry & registry, float , bool )
{
	GetComponent<ActiveComponent>(registry)->isActiveSelf = GetComponent<ParticleEmitterComponent>(registry)->enabled;
};

void ParticleScript::OnFixedUpdate(Registry&, float, bool)
{
};
