/*!
@file       UIManager.cpp
@author     Ou Yukang (yukang.ou) 100%
@date       03/11/2025
@brief		Handles UI instances, notably buttons and their states such as
			onHoverEnter, onClick, onHoverExit


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "pch.h"
#include "UIManager.h"
#include "UpdateStackManager.h"
#include "SceneManager.h"
#include "Components.h"


Vec2 UIManager::gameMousePos = {};
ComponentRegistry* UIManager::compRegistry;
Registry* UIManager::registry;
EntityRegistry::Entity UIManager::hoveredButton;
bool disableController;
EntityRegistry::Entity selectedButton;
float inputCooldown;



void UIManager::Init(Registry& _registry, ComponentRegistry& _compRegistry) {
	registry = &_registry;
	compRegistry = &_compRegistry;
	CEO::Get<EventsDispatcher>()->Subscribe<Events::UpdateSelectedEntity>([](Events::UpdateSelectedEntity selectedEntity) {
		selectedButton = selectedEntity.e;
		});



	CEO::Get<EventsDispatcher>()->Subscribe<Events::SceneChanged>([](const auto&) {
		selectedButton = 0;
		});

	CEO::Get<EventsDispatcher>()->Subscribe<Events::ScenePushed>([](const auto&) {
		selectedButton = 0;
		});

	CEO::Get<EventsDispatcher>()->Subscribe<Events::ScenePop>([](const auto&) {
		selectedButton = 0;
		});


}

void UIManager::Free() {
	// empty
}

Registry::Entity UIManager::GetButtonCollision()
{

	auto buttons{ registry->GetEntitiesWithComponents<ButtonComponent, UITransformComponent>() };
#ifdef PLATFORM_WINDOWS
	Vec2 cursorPos = CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{ Input::GetX(), Input::GetY() });
	//LOGI("Screen pos: %d, %d", static_cast<int>(Input::GetX()), static_cast<int>(Input::GetY()));
	//LOGI("View pos:		%d, %d", static_cast<int>(cursorPos.x), static_cast<int>(cursorPos.y));
#endif
#ifdef PLATFORM_ANDROID
    Vec2 cursorPos = CEO::Get<CameraManager>()->ScreenToViewPos(Vec2{ Input::GetX(0), Input::GetY(0) });
    //LOGI("Screen pos: %d, %d", static_cast<int>(Input::GetX(0)), static_cast<int>(Input::GetY(0)));
    //LOGI("View pos:	  %d, %d", static_cast<int>(cursorPos.x), static_cast<int>(cursorPos.y));
#endif

	unsigned int highestPriority = 0;
	Registry::Entity collidingButton = 0;
	for (Registry::Entity buttonEntity : buttons)
	{
		// skipping the update if it is part of a lower stack
		if (CEO::Instance().GetManager<UpdateStackManager>()->ShouldNotUpdate(buttonEntity) ||
			!registry->GetComponent<ActiveComponent>(buttonEntity)->isActiveSelf || 
			!registry->GetComponent<ActiveComponent>(buttonEntity)->isActiveInHierarchy) continue;

		ButtonComponent* button = registry->GetComponent<ButtonComponent>(buttonEntity);
		(void)button;

		UITransformComponent* transform = registry->GetComponent<UITransformComponent>(buttonEntity);
		Vec2 buttonPos{ transform->transform.m[6], transform->transform.m[7] }; // get viewport position of button
		Vec2 difference{ cursorPos - buttonPos};

		unsigned int renderPriority = registry->GetComponent<LayerComponent>(buttonEntity)->renderPriority;
		// transform the difference vector into button's model space
		difference = transform->transform.Inversed().TransformVector(difference);
		if (abs(difference.x) < 0.5f && abs(difference.y) < 0.5f)
		{
			if (!collidingButton|| renderPriority > highestPriority)
			{
				collidingButton = buttonEntity;
				highestPriority = renderPriority;
			}
		}
	}

	return collidingButton;
}

void UIManager::UpdateUIState()
{
#ifdef PLATFORM_WINDOWS
	bool usingController = Input::IsGamepadConnected(0);

	// =========================
	// CONTROLLER MODE
	// =========================
	if (usingController)
	{

		// clear previous hovered button if different
		if (hoveredButton && hoveredButton != selectedButton &&
			registry->HasComponent<ButtonComponent>(hoveredButton))
		{
			ButtonComponent& prev = *registry->GetComponent<ButtonComponent>(hoveredButton);

			if (prev.onHoverExit)
				prev.onHoverExit();

			prev.state = ButtonComponent::IDLE;
		}

		// ensure we have a selected button
		SceneManager* sm = CEO::Get<SceneManager>();
		if (!sm->SceneStack().empty())
		{
			if (sm->SceneStack().top() == "Game" ) return;


		}


		if (!selectedButton)
			SelectFirstButton();

		// apply hover state to selected button
		if (selectedButton && registry->HasComponent<ButtonComponent>(selectedButton))
		{
			ButtonComponent& button = *registry->GetComponent<ButtonComponent>(selectedButton);

			if (button.state != ButtonComponent::HOVERED)
			{
				if (button.onHoverEnter)
					button.onHoverEnter();

				button.state = ButtonComponent::HOVERED;
			}
		}



		hoveredButton = selectedButton;
		return;
	}
	else

	{

		// read the object picking buffer for the entity under mouse cursor
		EntityRegistry::Entity newHoveredEntity{ GetButtonCollision() };

		if (newHoveredEntity != hoveredButton) // new entity hovered
		{
			// button not properly handled when scene changes when the entity is removed
			// a on scene changed callback should be called in the future to set hovered button to null
			// for now a good enough solution is to just check if hovered button has component
			if (hoveredButton && registry->HasComponent<ButtonComponent>(hoveredButton)) // mouse exit hover of previous button
			{
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(hoveredButton);
				if (button.onHoverExit)
					button.onHoverExit();
				button.state = ButtonComponent::IDLE;
				//LOGI("Button: %d Idle", hoveredButton);
			}

			if (registry->HasComponent<ButtonComponent>(newHoveredEntity)) // mouse entered hover on new button
			{
				hoveredButton = newHoveredEntity;
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(hoveredButton);

				if (button.onHoverEnter)
					button.onHoverEnter();
				button.state = ButtonComponent::HOVERED;
				//LOGI("Button: %d Hovered", hoveredButton);
			}
			else // new hovered entity is not a button
				hoveredButton = 0; // no buttons hovered
		}
	}
#endif

}
void UIManager::SelectFirstButton()
{
	auto buttons = registry->GetEntitiesWithComponents<ButtonComponent, UITransformComponent>();
	int highestPriority = -1;

	for (auto e : buttons)
	{
		if (CEO::Instance().GetManager<UpdateStackManager>()->ShouldNotUpdate(e) ||
			!registry->GetComponent<ActiveComponent>(e)->isActiveSelf ||
			!registry->GetComponent<ActiveComponent>(e)->isActiveInHierarchy) continue;


		unsigned int renderPriority = registry->GetComponent<LayerComponent>(e)->renderPriority;
		// transform the difference vector into button's model space

			if (static_cast<int>(renderPriority) > highestPriority)
			{
				selectedButton = e;
				highestPriority = renderPriority;
			}
		
	}
	if(highestPriority == -1)
		selectedButton = 0;
}
void UIManager::UpdateUIInput()
{
#ifdef PLATFORM_WINDOWS
	if (Input::IsGamepadConnected(0))
	{
		if (Input::IsGamepadButtonPressed(Input::GamepadButton::A))
		{
			if (selectedButton && registry->HasComponent<ButtonComponent>(selectedButton))
			{
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(selectedButton);

				if ((button.state == ButtonComponent::IDLE || button.state == ButtonComponent::HOVERED) && button.onPress)
					button.onPress();

				disableController = true;
				button.state = ButtonComponent::PRESSED;
			}
		}

		if (Input::IsGamepadButtonHeld(Input::GamepadButton::A))
		{
			if (selectedButton && registry->HasComponent<ButtonComponent>(selectedButton))
			{
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(selectedButton);

				if (button.onHeld)
					button.onHeld();

			}
		}

		if (Input::IsGamepadButtonReleased(Input::GamepadButton::A))
		{
			if (selectedButton && registry->HasComponent<ButtonComponent>(selectedButton))
			{
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(selectedButton);

				if (button.onClick)
					button.onClick();
				disableController = false;

				button.state = ButtonComponent::RELEASED;
			}
		}
	}
	else {

		if (Input::IsButtonHeld(GLFW_MOUSE_BUTTON_LEFT)) // button press
		{
			//LOGI("Mouse click in UI Input Entity: %d, position(%f, %f)", hoveredButton, gameMousePos.x, gameMousePos.y);
			if (hoveredButton && registry->HasComponent<ButtonComponent>(hoveredButton))
			{
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(hoveredButton);
				if (button.onHeld)
					button.onHeld();

				if ((button.state == ButtonComponent::IDLE || button.state == ButtonComponent::HOVERED) && button.onPress)
					button.onPress();

				button.state = ButtonComponent::PRESSED;
				//LOGI("Button: %d Pressed", hoveredButton);
			}
		}
		else if (Input::IsButtonRelease(GLFW_MOUSE_BUTTON_LEFT)) // button released
		{
			if (hoveredButton && registry->HasComponent<ButtonComponent>(hoveredButton))
			{
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(hoveredButton);
				if (button.onClick)
					button.onClick();
				button.state = ButtonComponent::RELEASED;
				//LOGI("Button: %d Released", hoveredButton);
			}
		}
	}
#else
	if (Input::IsPointerHeld(0)) // button held
	{
		EntityRegistry::Entity heldEntity = GetButtonCollision();
		//ScreenManager::screenBuffer.Bind();
		//glReadBuffer(GL_COLOR_ATTACHMENT1);
		//glReadPixels(static_cast<int>(Input::GetX(0)) + ScreenManager::screenBuffer.Width() / 2, static_cast<int>(Input::GetY(0)) + ScreenManager::screenBuffer.Height() / 2, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &heldEntity);
		//ScreenManager::screenBuffer.Unbind();

		//LOGI("Tapped Coordinate (%d, %d) Entity: %d", static_cast<int>(Input::GetX(0)), static_cast<int>(Input::GetY(0)), tappedEntity);
		if (heldEntity != hoveredButton) // new entity hovered
		{
			if (hoveredButton && registry->HasComponent<ButtonComponent>(hoveredButton)) // finger exit hover/press of previous button
			{
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(hoveredButton);
				if (button.onHoverExit)
					button.onHoverExit();
				button.state = ButtonComponent::IDLE;
				LOGI("Button: %d Idle", hoveredButton);
			}

			if (registry->HasComponent<ButtonComponent>(heldEntity)) // finger entered hover/press on new button
			{
				hoveredButton = heldEntity;
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(heldEntity);
				button.state = ButtonComponent::PRESSED;
				if (Input::IsPointerPressed(0)) // button tap
				{
					if (button.onPress)
						button.onPress();
			
					LOGI("Button: %d Pressed", hoveredButton);
				}
			}
			else
				hoveredButton = 0;
		}
		else
		{
			if (hoveredButton && registry->HasComponent<ButtonComponent>(hoveredButton)) // finger exit hover/press of previous button
			{
				ButtonComponent& button = *registry->GetComponent<ButtonComponent>(hoveredButton);
				if (button.onHeld)
					button.onHeld();
				button.state = ButtonComponent::PRESSED;
				LOGI("Button: %d Held", hoveredButton);
			}
		}
	}
	else if (Input::IsPointerRelease(0))
	{
		if (hoveredButton && registry->HasComponent<ButtonComponent>(hoveredButton))
		{
			ButtonComponent& button = *registry->GetComponent<ButtonComponent>(hoveredButton);
			if (button.state == ButtonComponent::PRESSED) // trigger button
			{
				if (button.onClick)
					button.onClick();
				button.state = ButtonComponent::RELEASED;
				LOGI("Button: %d Released", hoveredButton);
			}
		}
	}
	else // reset to idle
	{
        if ( hoveredButton && registry->HasComponent<ButtonComponent>(hoveredButton))
        {
            LOGI("Button: %d Idle", hoveredButton);
            ButtonComponent& button = *registry->GetComponent<ButtonComponent>(hoveredButton);
            button.state = ButtonComponent::IDLE;
            hoveredButton = 0;
        }
	}


#endif


}

void UIManager::UpdateUITransform(Registry& m_registry)
{
	//auto entities = registry.GetEntitiesWithComponents<UITransformComponent>();

	Vec2 parentSize{};
	Vec2 viewportSize = CEO::Instance().GetManager<CameraManager>()->GetGameViewportSize();
	Mat3 cameraProj = CEO::Instance().GetManager<CameraManager>()->GetGameProjection();
	Mat3 scale{}, rotate{}, trans{}, pivot{};
	//for (auto entity : entities)
	//{
	//	UITransformComponent& ui = *registry.GetComponent<UITransformComponent>(entity);

	//	// mdl transform
	//	pivot = Mat3::Translation(-ui.relativePivot.x * 0.5f, -ui.relativePivot.y * 0.5f);
	//	scale = Mat3::Scale(ui.size.x, ui.size.y);
	//	rotate = Mat3::Rotation(ui.rotation);

	//	trans = Mat3::Translation(ui.translate.x, ui.translate.y);

	//	ui.viewportTransform = trans * rotate * scale * pivot;
	//	// get transform to to move UI relative to parent
	//	ui.anchorTransform = Mat3::Translation(ui.min.x + (ui.anchor.x + 1) * 0.5f * (ui.max.x - ui.min.x),
	//		ui.min.y + (ui.anchor.y + 1) * 0.5f * (ui.max.y - ui.min.y));
	//}


	for (auto entity : CEO::Instance().GetManager<HierarchyManager>()->GetHierarchyList())
	{
		UITransformComponent& transform = *m_registry.GetComponent<UITransformComponent>(entity);
		HierarchyComponnent& HC = *m_registry.GetComponent<HierarchyComponnent>(entity);
		// mdl transform
		if (!&transform) continue;

		// mdl transform
		pivot = Mat3::Translation(-transform.relativePivot.x * 0.5f * transform.size.x, -transform.relativePivot.y * 0.5f * transform.size.y);
		scale = Mat3::Scale(transform.size.x, transform.size.y);
		rotate = Mat3::Rotation(transform.rotation);

		trans = Mat3::Translation(transform.relativePos.x, transform.relativePos.y);

		transform.viewportTransform = trans * rotate * pivot * scale ;
		transform.xTransform = trans * rotate * pivot;

		Vec2 min{}, max{};
		if (HC.parent == 0)
		{
			// get transform to to move UI relative to screen
			min = { transform.min.x * viewportSize.x, transform.min.y * viewportSize.y };
			max = { transform.max.x * viewportSize.x, transform.max.y * viewportSize.y };

			transform.anchorTransform = Mat3::Translation(min.x + transform.anchor.x * (max.x - min.x),
														  min.y + transform.anchor.y * (max.y - min.y));
			transform.transform = transform.anchorTransform * transform.viewportTransform;
			transform.xTransform = transform.anchorTransform * transform.xTransform;
		}
		else
		{
			UITransformComponent& parentTransform = *m_registry.GetComponent<UITransformComponent>(HC.parent);

			// get transform to to move UI relative to parent
			min = { transform.min.x * parentTransform.size.x, transform.min.y * parentTransform.size.y };
			max = { transform.max.x * parentTransform.size.x, transform.max.y * parentTransform.size.y };
		
			transform.anchorTransform = Mat3::Translation(min.x + transform.anchor.x * (max.x - min.x),
													      min.y + transform.anchor.y * (max.y - min.y));
			transform.transform = parentTransform.xTransform * transform.anchorTransform * transform.viewportTransform;
			transform.xTransform = parentTransform.xTransform * transform.anchorTransform * transform.xTransform;
		}
	}
}


void UIManager::UpdateControllerNavigation(float dt)
{
#ifdef PLATFORM_WINDOWS
	SceneManager* sm = CEO::Get<SceneManager>();
	if (!sm->SceneStack().empty())
	{
		if (sm->SceneStack().top() == "Game") return;
	}

	Vec2 nav = Input::GetLeftStick();

	if (inputCooldown > 0.0f)
	{
		inputCooldown -= dt;
		return;
	}

	if (nav.Length() > 0.5f)
	{
		if (disableController) return;
		
		EntityRegistry::Entity next = FindNextButton(nav);

		if (next)
		{
			selectedButton = next;
			inputCooldown = 0.2f;
		}
	}
#endif
}


Vec2 UIManager::GetButtonPos(EntityRegistry::Entity e)
{
	auto* transform = registry->GetComponent<UITransformComponent>(e);
	return Vec2{ transform->transform.m[6], transform->transform.m[7] };
}

EntityRegistry::Entity UIManager::FindNextButton(Vec2 navDir)
{
	auto buttons = registry->GetEntitiesWithComponents<ButtonComponent, UITransformComponent>();

	if (!selectedButton)
		return 0;



	Vec2 currentPos = GetButtonPos(selectedButton);

	EntityRegistry::Entity best = 0;
	float bestScore = FLT_MAX;

	for (auto e : buttons)
	{
		if (e == selectedButton) continue;

		if (CEO::Instance().GetManager<UpdateStackManager>()->ShouldNotUpdate(e) ||
			!registry->GetComponent<ActiveComponent>(e)->isActiveSelf ||
			!registry->GetComponent<ActiveComponent>(e)->isActiveInHierarchy) continue;

		Vec2 pos = GetButtonPos(e);
		Vec2 diff = pos - currentPos;

		// normalize direction
		Vec2 dir = navDir.Normalised();
		Vec2 toCandidate = diff.Normalised();

		// dot product = how aligned it is with stick direction
		float alignment = dir.x *toCandidate.x+ dir.y * toCandidate.y;

		// reject if not in that direction
		if (alignment < 0.5f) continue;

		float dist = diff.Length();

		// prefer closest + best aligned
		float score = dist - alignment * 50.0f;

		if (score < bestScore)
		{
			best = e;
			bestScore = score;
		}
	}

	return best;
}