/*!
@file       Camera.cpp
@author     Ou Yukang (yukang.ou) 100%
@date       06/10/2025
@brief		Camera Manager for both editor and in game


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "pch.h"
#include "Camera.h"
#include "Components.h"
#include "EventsDispatcher.h"

bool CameraManager::Init(int _screenWidth, int _screenHeight)
{
	static bool isInit = false;
	if (isInit)
		return true;
	LOGI("Initializing CameraManager");
	aspectRatio = static_cast<float>(_screenWidth) / _screenHeight;

	// find main camera in new scene when scene changes
	CEO::Get<EventsDispatcher>()->Subscribe<Events::SceneChanged>([this](Events::SceneChanged) {
		FindAndSetMainCamera();
		IsViewportDirty(true);
	});

#ifdef EditorFlag
	CEO::Get<EventsDispatcher>()->Subscribe<Events::MouseScroll>([this](Events::MouseScroll event) {
		HandleMouseScroll(event.scrollOffset); });

	editorCamera.cam.viewportSize = { static_cast<float>(_screenWidth), static_cast<float>(_screenHeight) };
	CEO::Get<EventsDispatcher>()->Subscribe<Events::EditorViewportMoved>([this](Events::EditorViewportMoved event) { OnEditorViewportMoved(event.newViewportPos); });
	CEO::Get<EventsDispatcher>()->Subscribe<Events::EditorViewportResized>([this](Events::EditorViewportResized event) { OnEditorViewportResized(event.newViewportSize); });
	CEO::Get<EventsDispatcher>()->Subscribe<Events::ImGuiHidden>([this](const Events::ImGuiHidden& event) { OnImGuiHidden(event.hidden); });
#endif

	return true;
}

bool CameraManager::HasMainCamera()
{
	return mainCamera;
}

void CameraManager::SetMainCamera(Registry::Entity newMainCam)
{
	Registry& registry = *CEO::Get<Registry>();
	if (!registry.HasComponent<CameraComponent>(newMainCam))
	{
		LOGE("Tried to set entity with id:[%d] with no camera component as main camera", newMainCam);
		return;
	}

	// transfer main camera status to new camera
	if (mainCamera)
		registry.GetComponent<CameraComponent>(mainCamera)->isMainCam = false;
	registry.GetComponent<CameraComponent>(newMainCam)->isMainCam = true;
	mainCamera = newMainCam;
}

bool CameraManager::FindAndSetMainCamera()
{
	Registry& registry = *CEO::Get<Registry>();
	auto entities = registry.GetEntitiesWithComponents<TransformComponent, CameraComponent>();
	bool mainCamFound = false;

	for (auto cameraEntity : entities)
	{
		CameraComponent& camera = *registry.GetComponent<CameraComponent>(cameraEntity);
		if (camera.isMainCam)
		{
			if (!mainCamFound) // set the first instance of main camera to main cam
			{
				mainCamera = cameraEntity;
				mainCamFound = true;
			}
			else // set all other instances to not main cam
			{
				camera.isMainCam = false;
			}
		}
	}

	if (!mainCamFound)
		mainCamera = 0;

	return mainCamFound;
}

Vec2 CameraManager::GetPosition()
{
#ifdef EditorFlag
	if (isEditorCamera)
		return editorCamera.position;
	else if (!mainCamera && !FindAndSetMainCamera()) // no main camera
	{
		return editorCamera.position;
	}
#endif //  EditorFlag

	Registry& registry = *CEO::Get<Registry>();
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		TransformComponent* transform = registry.GetComponent<TransformComponent>(mainCamera);
		if (transform)
			return transform->translate;
	}

	return Vec2{};
}

Mat3 CameraManager::GetView()
{
#ifdef EditorFlag
	if (isEditorCamera)
		return editorCamera.cam.view;
	else if (!mainCamera && !FindAndSetMainCamera()) // no main camera
	{
		//LOGE("No main camera found in scene");
		return editorCamera.cam.view;
	}
#endif //  EditorFlag

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if(cam)
			return cam->view;
	}

	return Mat3{};
}
Mat3 CameraManager::GetGameProjection()
{
#ifdef EditorFlag
	if (!mainCamera && !FindAndSetMainCamera()) // no main camera
	{
		//LOGE("No main camera found in scene");
		return editorCamera.cam.proj;
	}
#endif //  EditorFlag

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam)
			return cam->proj;
	}

	return Mat3{};
}
Mat3 CameraManager::GetProjection()
{
#ifdef EditorFlag
	if (isEditorCamera)
		return editorCamera.cam.proj;
	else if (!mainCamera && !FindAndSetMainCamera()) // no main camera
	{
		//LOGE("No main camera found in scene");
		return editorCamera.cam.proj;
	}
#endif //  EditorFlag

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam)
			return cam->proj;
	}
	return Mat3{};
}
Mat3 CameraManager::GetViewProjection()
{
#ifdef EditorFlag
	if (isEditorCamera)
		return editorCamera.cam.vp;
	else if (!mainCamera && !FindAndSetMainCamera()) // no main camera
	{
		//LOGE("No main camera found in scene");
		return editorCamera.cam.vp;
	}
#endif //  EditorFlag

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam)
			return cam->vp;
	}
	
	return Mat3{};
}
Vec2 CameraManager::GetGameViewportSize()
{
#ifdef EditorFlag
	if (!mainCamera && !FindAndSetMainCamera()) // no main camera
	{
		//LOGE("No main camera found in scene");
		return editorCamera.cam.viewportSize;
	}
#endif //  EditorFlag

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam)
			return cam->viewportSize;
	}

	return Vec2{};
}
Vec2 CameraManager::GetViewportSize()
{
#ifdef EditorFlag
	if (isEditorCamera)
		return editorCamera.cam.viewportSize * 1.f / editorCamera.zoom;
	else if (!mainCamera && !FindAndSetMainCamera()) // no main camera
	{
		//LOGE("No main camera found in scene");
		return editorCamera.cam.viewportSize;
	}
#endif //  EditorFlag

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam)
			return cam->viewportSize;
	}
	
	return Vec2{};
}
void CameraManager::ChangeAspectRatio(float ar)
{
	LOGI("Changing camera aspect ratio: %f", ar);
	aspectRatio = ar;
}
void CameraManager::ChangeScreenSize(Vec2 newSize)
{
	screenSize = newSize;
}



bool CameraManager::SetMainCamMask(GLuint index, bool show) {
	if (!mainCamera) return false;
#ifdef EditorFlag
	if (isEditorCamera) return false;
#endif

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam) {
			if(show) cam->cullingMask |= CEO::Instance().GetManager<LayerManager>()->GetLayerMask(index);
			else cam->cullingMask &= ~CEO::Instance().GetManager<LayerManager>()->GetLayerMask(index);
			return true;
		}

	}
	return false;
}

uint64_t CameraManager::GetMainCamMask() {
	if (!mainCamera) return ~uint64_t{};

#ifdef EditorFlag
	if (isEditorCamera) return ~uint64_t{};
#endif //  EditorFlag

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam) return cam->cullingMask;
			
	}

	return ~uint64_t{};
}

#ifdef EditorFlag
bool CameraManager::IsEditorCam()
{
	return isEditorCamera;
}
void CameraManager::IsEditorCam(bool isEditor)
{
	isEditorCamera = isEditor;
}
void CameraManager::ToggleEditorCam()
{
	isEditorCamera = !isEditorCamera;
}
void CameraManager::UpdateEditorCameraKeyboardInput(float dt)
{
	static Vec2 currPos{}, prevPos{};

	if (!isEditorCamera) // dont bother updating if not editor camera anyways
		return;
	if (suppressEditorCameraKeyboardMovement)
		return; //movement is suppressed, return
	Vec2 moveVector{0,0};
	if (Input::IsKeyHeld(GLFW_KEY_W))
		moveVector.y += 1.f;
	if (Input::IsKeyHeld(GLFW_KEY_S))
		moveVector.y -= 1.f;
	Vec2 moveDir{ 0,0 };
	if (Input::IsKeyHeld(GLFW_KEY_A))
		moveVector.x -= 1.f;
	if (Input::IsKeyHeld(GLFW_KEY_D))
		moveVector.x += 1.f;

	moveVector.Normalise();
	// scale movement with viewport size
	moveVector.x *= editorCamera.cam.viewportSize.x * (1.f / editorCamera.zoom);
	moveVector.y *= editorCamera.cam.viewportSize.y * (1.f / editorCamera.zoom);

	//move camera
	editorCamera.position += moveVector * dt;
#ifdef PLATFORM_ANDROID
	if (Input::IsPointerPressed(0))
	{
		currPos = prevPos = { Input::GetX(0), Input::GetY(0) };
	}
	else if (Input::IsPointerHeld(0))
	{
		currPos = Vec2{ Input::GetX(0), Input::GetY(0) };
		Vec2 posOffset = currPos - prevPos;
		editorCamera.position += posOffset;

		prevPos = currPos;
	}
#endif
}
void CameraManager::UpdateEditorCameraMouseInput()
{
	static Vec2 currPos{}, prevPos{};

	if (!isEditorCamera) // dont bother updating if not editor camera anyways
		return;
	if (Input::IsKeyPressed(GLFW_KEY_EQUAL))
	{
		editorCamera.zoom += 0.1f;
	}
	if (Input::IsKeyPressed(GLFW_KEY_MINUS))
	{
		editorCamera.zoom += editorCamera.zoom > 0.1f ? -0.1f : 0.f;
	}

	if (suppressEditorCameraMouseMovement)
		return; //movement is suppressed, return
#ifdef PLATFORM_WINDOWS
	if (Input::IsButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
	{
		currPos = prevPos = { Input::GetX(), Input::GetY() };
	}
	else if (Input::IsButtonHeld(GLFW_MOUSE_BUTTON_RIGHT))
	{
		currPos = Vec2{ Input::GetX(), Input::GetY() };
		Vec2 posOffset = { prevPos.x - currPos.x, currPos.y - prevPos.y }; // y offset is flipped because window-y coordinate direction is opposite of opengl
		editorCamera.position += (posOffset * 1.f / editorCamera.zoom);

		prevPos = currPos;
	}
#endif
}
void CameraManager::HandleMouseScroll(float scrollOffset)
{
	if (suppressEditorCameraMouseScroll)
		return;

	editorCamera.zoom += scrollOffset * 0.1f;
	editorCamera.zoom = editorCamera.zoom > 0.1f ? editorCamera.zoom : 0.1f;
}
void CameraManager::UpdateEditorCamera()
{
	if (isViewportDirty) // update viewport as aspect ratio changed
	{
		if (aspectRatio > editorCamera.cam.viewportSize.x / editorCamera.cam.viewportSize.y)
			editorCamera.cam.viewportSize = { editorCamera.cam.viewportSize.y * aspectRatio, editorCamera.cam.viewportSize.y };
		else
			editorCamera.cam.viewportSize = { editorCamera.cam.viewportSize.x, editorCamera.cam.viewportSize.x / aspectRatio};
	}

	// update camera transforms
	Mat3 rotate = Mat3::Rotation(ToRad(-editorCamera.rotation));
	Mat3 trans = Mat3::Translation(-editorCamera.position.x, -editorCamera.position.y);
	editorCamera.cam.proj = Mat3::Scale(2.f / (editorCamera.cam.viewportSize.x * 1.f / editorCamera.zoom), 2.f / (editorCamera.cam.viewportSize.y * 1.f / editorCamera.zoom));
	editorCamera.cam.view = trans * rotate;
	editorCamera.cam.vp = editorCamera.cam.proj * editorCamera.cam.view;
}
void CameraManager::ResetEditorCameraZoom()
{
	editorCamera.zoom = 1.f;
}
void CameraManager::SetEditorCameraViewportSize(Vec2 viewportSize)
{
	editorCamera.cam.viewportSize = viewportSize;
}
void CameraManager::SetEditorCameraPosition(Vec2 pos)
{
	editorCamera.position = pos;
}
#endif
bool CameraManager::IsViewportDirty()
{
	return isViewportDirty;
}
void CameraManager::IsViewportDirty(bool isDirty)
{
	isViewportDirty = isDirty;
}
float CameraManager::AspectRatio()
{
	return aspectRatio;
}

#ifdef EditorFlag
Vec2 CameraManager::EditorPosToViewportPos(Vec2 mousePos)
{
	CameraManager camManager = *CEO::Get<CameraManager>();
	if (camManager.isEditorCamera)
	{
		Vec2 halfCameraViewportSize = 0.5f * camManager.editorCamera.cam.viewportSize;

		// mousepos relative to editor viewport
		Vec2 relativePos = mousePos - camManager.editorViewportOrigin;
		// mousepos relative to camera viewport with origin at top left
		Vec2 camMousePos = relativePos / camManager.editorViewportSize.y * camManager.editorCamera.cam.viewportSize.y;
		// mousepos relative to camera viewport with origin at bottom left
		camMousePos.y = camManager.editorCamera.cam.viewportSize.y - camMousePos.y;
		// mousepos relative to camera viewport with origin at center
		camMousePos.x -= halfCameraViewportSize.x;
		camMousePos.y -= halfCameraViewportSize.y;

		//clamp position to camera viewport
		camMousePos.x = std::clamp<float>(camMousePos.x, -halfCameraViewportSize.x, halfCameraViewportSize.x);
		camMousePos.y = std::clamp<float>(camMousePos.y, -halfCameraViewportSize.y, halfCameraViewportSize.y);

		return camMousePos;
	}

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((camManager.mainCamera && registry.HasComponent<CameraComponent>(camManager.mainCamera)) || camManager.FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(camManager.mainCamera);
		if (cam)
		{
			Vec2 halfCameraViewportSize = 0.5f * cam->viewportSize;

			// mousepos relative to editor viewport
			Vec2 relativePos = mousePos - camManager.editorViewportOrigin;
			// mousepos relative to camera viewport with origin at top left
			Vec2 camMousePos = relativePos / camManager.editorViewportSize.y * cam->viewportSize.y;
			// mousepos relative to camera viewport with origin at bottom left
			camMousePos.y = cam->viewportSize.y - camMousePos.y;
			// mousepos relative to camera viewport with origin at center
			camMousePos.x -= halfCameraViewportSize.x;
			camMousePos.y -= halfCameraViewportSize.y;

			//clamp position to camera viewport
			camMousePos.x = std::clamp<float>(camMousePos.x, -halfCameraViewportSize.x, halfCameraViewportSize.x);
			camMousePos.y = std::clamp<float>(camMousePos.y, -halfCameraViewportSize.y, halfCameraViewportSize.y);

			return camMousePos;
		}
	}
	return Vec2{};
}

void CameraManager::OnEditorViewportMoved(Vec2 newPos)
{
	editorViewportOrigin = newPos;
}

void CameraManager::OnEditorViewportResized(Vec2 newSize)
{
	editorViewportSize = newSize;
}

void CameraManager::OnImGuiHidden(bool hidden) {
	static Vec2 oldOrigin{ editorViewportOrigin };
	static Vec2 oldSize{ editorViewportSize };

	if (hidden) {
		oldOrigin = editorViewportOrigin;
		oldSize = editorViewportSize;
		editorViewportOrigin = { 0.f, 0.f };
		auto& screen{ *CEO::Get<ScreenManager>() };
		editorViewportSize = { 
			static_cast<float>(screen.screenBuffer.Width()), 
			static_cast<float>(screen.screenBuffer.Height()) 
		};
	}
	else {
		editorViewportOrigin = oldOrigin;
		editorViewportSize = oldSize;
	}
}
#endif

Vec2 CameraManager::ScreenToViewPos(Vec2 screenPos)
{
#ifdef EditorFlag
	if (isEditorCamera)
	{
		return ScreenToViewPos(editorCamera.cam, screenPos);
	}
#endif

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam)
		{
			return ScreenToViewPos(*cam, screenPos);
		}
	}
	return Vec2{};
}

Vec2 CameraManager::ScreenToViewPos(CameraComponent const& camera, Vec2 screenPos)
{

	Vec2 halfCameraViewportSize = 0.5f * camera.viewportSize;

#ifdef EditorFlag
	// screenPos relative to editor viewport
	Vec2 relativePos = screenPos - editorViewportOrigin;
	// screenPos relative to camera viewport with origin at top left
	Vec2 camViewPos = relativePos / editorViewportSize.y * camera.viewportSize.y;
#endif
#ifndef EditorFlag
	Vec2 camViewPos = { screenPos.x /screenSize.x *camera.viewportSize.x,  screenPos.y / screenSize.y * camera.viewportSize.y };
#endif

// PC screen origin at top left, additional transformations needed
#ifdef PLATFORM_WINDOWS
	// mousepos relative to camera viewport with origin at bottom left
	camViewPos.y = camera.viewportSize.y - camViewPos.y;
	// mousepos relative to camera viewport with origin at center
	camViewPos.x -= halfCameraViewportSize.x;
	camViewPos.y -= halfCameraViewportSize.y;
#endif // PLATFORM_WINDOWS

	//clamp position to camera viewport
	camViewPos.x = std::clamp<float>(camViewPos.x, -halfCameraViewportSize.x, halfCameraViewportSize.x);
	camViewPos.y = std::clamp<float>(camViewPos.y, -halfCameraViewportSize.y, halfCameraViewportSize.y);

	return camViewPos;
}

Vec2 CameraManager::ScreenToWorldPos(CameraComponent const& camera, Vec2 screenPos)
{
	Vec2 viewPos = ScreenToViewPos(camera, screenPos);

	return camera.view.Inversed() * viewPos;
}

Vec2 CameraManager::ScreenToWorldPos(Vec2 screenPos)
{
#ifdef EditorFlag
	if (isEditorCamera)
		return ScreenToWorldPos(editorCamera.cam, screenPos);
#endif

	Registry& registry = *CEO::Get<Registry>();
	CameraComponent* cam;
	if ((mainCamera && registry.HasComponent<CameraComponent>(mainCamera)) || FindAndSetMainCamera())
	{
		cam = registry.GetComponent<CameraComponent>(mainCamera);
		if (cam)
		{
			return ScreenToWorldPos(*cam, screenPos);
		}
	}

	return Vec2{};
}

void CameraManager::Free()
{
	mainCamera = 0;
}

void UpdateCameras(Registry& registry)
{
	CameraManager* cameraManager = CEO::Instance().GetManager<CameraManager>();
	bool isViewportDirty = cameraManager->IsViewportDirty();
	// update camera entities
	auto entities = registry.GetEntitiesWithComponents<TransformComponent, CameraComponent>();
	Mat3 rotate{}, trans{};
	for (auto entity : entities)
	{
		TransformComponent& transform = *registry.GetComponent<TransformComponent>(entity);
		CameraComponent& camera = *registry.GetComponent<CameraComponent>(entity);

		if (isViewportDirty && !camera.fixedAspectRatio) // update viewport as aspect ratio changed
		{
			if (cameraManager->AspectRatio() > camera.viewportSize.x / camera.viewportSize.y)
				camera.viewportSize = { camera.viewportSize.y * cameraManager->AspectRatio(), camera.viewportSize.y };
			else
				camera.viewportSize = { camera.viewportSize.x , camera.viewportSize.x / cameraManager->AspectRatio() };

			LOGI("Changing camera viewport size: %f, %f", camera.viewportSize.x, camera.viewportSize.y);
		}

		// update camera transforms
		rotate = Mat3::Rotation(-transform.rotation);
		trans = Mat3::Translation(-transform.translate.x, -transform.translate.y);

		// mdl to ndc
		camera.proj = Mat3::Scale(2.f / camera.viewportSize.x, 2.f / camera.viewportSize.y);
		camera.view = trans * rotate;
		camera.vp = camera.proj * camera.view;
	}

#ifdef EditorFlag
	cameraManager->UpdateEditorCamera();
#endif
	cameraManager->IsViewportDirty(false);
}


Vec2 CameraManager::GetScreenSize()
{
	return screenSize;
}