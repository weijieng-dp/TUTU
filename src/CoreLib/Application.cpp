/*!
@file       Application.cpp
@author  	yukang.ou@digipen.edu(yukang)
@date       07/10/2025
@brief      Handles the life cycle of the application

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
______________________________________________________________________*/

#include "pch.h"
#include "Application.h"
#include "ScriptingAPI.h"
#include "SceneManager.h"
#include "UpdateStackManager.h"
#include "Pathfind.h"
#include "ParticleSystem.h"
#include "MapManager.h"
#include "StatsManager.h"
#include "ItemManager.h"
#include "PersistentDataManager.h"
#include "AchievementManager.h"
#include "UserSettingsManager.h"
void Application::SignalExit()
{
	CEO::Instance().GetManager<EventsDispatcher>()->Dispatch(Events::CloseWindow{});
	CEO::Instance().GetManager<Application>()->exitSignal= true;
}

void Application::PostRenderUpdate()
{
}

void Application::InitCoreManagers() {
	// Init Core Managers
#ifdef	EditorFlag
	CEO::Instance().AddManager<Logger>();
#endif
	CEO::Instance().AddManager<Registry>();
	CEO::Instance().AddManager<ComponentRegistry>();
	CEO::Instance().AddManager<ResourceManager>();
	CEO::Instance().AddManager<TextureManager>();
	CEO::Instance().AddManager<FileManager>();
	CEO::Instance().AddManager<FontManager>();
	CEO::Instance().AddManager<AudioManager>();
	CEO::Instance().AddManager<SceneManager>();
	CEO::Instance().AddManager<ScreenManager>();
	CEO::Instance().AddManager<UIManager>();
	CEO::Instance().AddManager<UserSettingsManager>();
	//CEO::Instance().AddManager<Savestate>();
	CEO::Instance().AddManager<CameraManager>();
	CEO::Instance().AddManager<Pathfind>();
	CEO::Instance().AddManager<LayerManager>();
	CEO::Instance().AddManager<UpdateStackManager>();
	CEO::Instance().AddManager<HierarchyManager>();
    CEO::Instance().AddManager<EventsDispatcher>();
	CEO::Instance().AddManager<ParticleSystem>();
	CEO::Instance().AddManager<GameObjectTracker>();
	CEO::Instance().AddManager<MapManager>();
	CEO::Instance().AddManager<AchievementManager>();
	CEO::Instance().AddManager<StatsManager>();
	CEO::Instance().AddManager<ItemManager>();
	CEO::Instance().AddManager<RenderUtils>();
	CEO::Instance().AddManager<PersistentDataManager>();
}

bool Application::IsExitSignalled()
{
	return CEO::Instance().GetManager<Application>()->exitSignal;
}

void Application::Free() {
	// empty for now
}
