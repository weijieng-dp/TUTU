/**___________________________________________________________________________/
@file          UpdateStackManager.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple manager to skip updates on entities that are on a "lower" stack

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include "UpdateStackManager.h"

#define SkipIf
#ifndef EditorFlag
// Commented out for now since we are creating objects without update stack component
// Need to fix that before it is safe to skip the if statement.
//#undef SkipIf
#endif


void UpdateStackManager::UpdateStorages() {
	registry = CEO::Get<Registry>();
	activeCompStorage = registry->GetStorage<ActiveComponent>();
	updCompStorage = registry->GetStorage<UpdateStackComponent>();
}

bool UpdateStackManager::ShouldUpdate(EntityRegistry::Entity e) const {
	if (e == 0) return false; // invalid ID
	UpdateStackComponent* usc = updCompStorage->Get(e);
#ifdef SkipIf
	if (!usc) {
		LOGE("ENTITY %d MISSING UPDATE STACK COMPONENT", e);
		return false; // Default behavior is meant to return false, set to true for now due to this being a late addition and we don't have the time to fix all older scenes
	}
#endif
	return usc->stack == CurrStack;
}

bool UpdateStackManager::ShouldUpdate(UpdateStackComponent const* usc) const
{
#ifdef SkipIf
	if (!usc) {
		LOGE("ENTITY MISSING UPDATE STACK COMPONENT");
		return false;
	}
#endif
	return usc->stack == CurrStack;
}

bool UpdateStackManager::IsActive(EntityRegistry::Entity e) const {

	if (e == 0) return false; // invalid ID
	ActiveComponent* ac = activeCompStorage->Get(e);
#ifdef SkipIf
	if (!ac) {
		LOGE("ENTITY %d MISSING ACTIVE COMPONENT", e);
		return false; 
	}
#endif
	return ac->isActiveInHierarchy && ac->isActiveSelf;
}