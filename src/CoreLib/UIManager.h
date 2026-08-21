/*!
@file       UIManager.h
@author     Ou Yukang (yukang.ou) 100%
@date       03/11/2025
@brief		Handles UI instances, notably buttons and their states such as
			onHoverEnter, onClick, onHoverExit


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "Registry.h"
#include "MathLib.h"

class UIManager {


	UIManager(const UIManager&) = delete;
	UIManager& operator=(const UIManager&) = delete;

	static Vec2 GetButtonPos(EntityRegistry::Entity e);

public:

	UIManager() = default;
	~UIManager() = default;

	/*!
	* \brief
	*	initialize the singleton instance
	* \param
	*	_registry - registry of ecs
	* \param
	*	_compRegistry - component registry of ecs
	*/
	static void Init(Registry& _registry, ComponentRegistry& _compRegistry);

	/*!
	* \brief
	*	free any resources used by UI Manager
	*/
	static void Free();

	/*!
	* \brief
	*	get a list of button collision candidates
	*/
	static Registry::Entity GetButtonCollision();

	/*!
	* \brief
	*	update button states(enterHover, exitHover, etc..) based on mouse position
	*/
	static void UpdateUIState();

	/*!
	* \brief
	*	handle input IO for UI such as mouse click and screen touches
	*/
	static void UpdateUIInput();

	/*!
	* \brief
	*	calculate the concatenated ui tansform
	*/
	static void UpdateUITransform(Registry& registry);

	static EntityRegistry::Entity FindNextButton(Vec2 dir);
	static void UpdateControllerNavigation(float dt);
	static void SelectFirstButton();
	static void UpdateSelectedButton(EntityRegistry::Entity ent);


	static Vec2 gameMousePos; // mouse pos relative to game window, translated from relative position in editor window if in editor mode
	static EntityRegistry::Entity hoveredButton; // 0 is no button, current button that is hovered by mouse
	static ComponentRegistry* compRegistry;
	static Registry* registry;
};