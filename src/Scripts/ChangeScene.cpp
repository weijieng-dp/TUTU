/*!
@file       ChangeScene.cpp
@author     Ou Yukang (yukang.ou) (100%)
@date       04/02/2026

Simple script to change scene on button press

Copyright (C) 2026 DigiPen Institute of Technology.
All rights reserved.
/*____________________________________________________________________________*/
#include "ChangeScene.h"
#include <functional>
#include "../CoreLib/CEO.h"
#include "../CoreLib/SceneManager.h"

void ChangeScene::OnStart(Registry& registry)
{
	resourceManager = CEO::Instance().GetManager<ResourceManager>();
	compRegistry = CEO::Instance().GetManager<ComponentRegistry>();
	reg = &registry;

	//SceneChange();
	ButtonComponent& button = *GetComponent<ButtonComponent>(registry);
	//LOGI("Binding ChangeScene() to button");
	button.onClick = std::bind(&ChangeScene::SceneChange, this);
};
void ChangeScene::OnUpdate(Registry& ,float , bool ){
};
void ChangeScene::OnFixedUpdate(Registry& ,float , bool ){/*empty by design*/ }

void ChangeScene::SceneChange(/*Registry& registry*/)
{
	SceneManager::QueueSceneAction(GetComponent<ChangeScene>(*CEO::Instance().GetManager<Registry>())->sceneName, SceneManager::CHANGE);
}
