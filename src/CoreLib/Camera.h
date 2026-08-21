/*!
@file       Camera.h
@author     Ou Yukang (yukang.ou) 100%
@date       06/10/2025
@brief		Camera Manager for both editor and in game


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "Platform.h"
#include "Registry.h"
#include "MathLib.h"
#include "Components.h"

class CameraManager {
private:
	struct EditorCamera
	{
		Vec2 position = {};
		float rotation = 0;
		float zoom = 1.f;
		CameraComponent cam;
	};

	Vec2 screenSize{};
	float aspectRatio =  16.f/9.f; // aspect ratio used by camera viewport
	bool isViewportDirty = true;
public:
	CameraManager() = default;
	~CameraManager() = default;

	/*!
	* \brief
	*	initialize camera manager
	* \return
	*	whether init was successful
	*/
	bool Init(int screenWidth, int screenHeight);
	/*!
	* \brief
	*	check if there is a main camera
	* \return
	*	if a main camera exists
	*/
	bool HasMainCamera();
	/*!
	* \brief
	*	set the main camera to render onto screen
	* \param
	*	registry - ecs registry instance
	* \param
	*	newMainCam - camera to be set as main camera
	*/
	void SetMainCamera(Registry::Entity newMainCam);
	/*!
	* \brief
	*	find the main camera among entities
	* \param
	*	newMainCam - camera to be set as main camera
	* \return
	*	whether a main camera is found
	*/
	bool FindAndSetMainCamera();
	/*!
	* \brief
	*	get position of current camera
	* \return
	*	view position
	*/
	Vec2 GetPosition();
	/*!
	* \brief
	*	get view matrix of current camera
	* \return
	*	view matrix
	*/
	Mat3 GetView();
	/*!
	* \brief
	*	get projection matrix of current game camera
	* \return
	*	projection matrix
	*/
	Mat3 GetGameProjection();
	/*!
	* \brief
	*	get projection matrix of current game/editor camera
	* \return
	*	projection matrix
	*/
	Mat3 GetProjection();
	/*!
	* \brief
	*	Get projection * view matrix of current camera
	* \return
	*	projection * view matrix
	*/
	Mat3 GetViewProjection();
	/*!
	* \brief
	*	get viewport size of current game camera
	* \return
	*	viewport size
	*/
	Vec2 GetGameViewportSize();
	/*!
	* \brief
	*	get viewport size of current game or editor camera
	* \return
	*	viewport size
	*/
	Vec2 GetViewportSize();
	/*!
	* \brief
	*	update aspect ratio used for viewport size of all cameras
	* \param
	*	ar - new aspect ratio
	*/
	void ChangeAspectRatio(float ar);
	/*!
	* \brief
	*	update screen size
	* \param
	*	newSize - new screen size
	*/
	void ChangeScreenSize(Vec2 newSize);
	/*!
	* \brief
	*	free any runtime resources used by camera manager
	*/
	void Free();

	/*!
	* \brief
	*	Set the culling mask of the main camera if it exists.
	* \param[in] index - The layer index to set
	* \param[in] show - Whether to show or hide the layer. true means show, false means hide
	* \return
	*	Whether the set was successful
	*/
	bool SetMainCamMask(GLuint index, bool show);

	/*!
	* \brief
	*	Get the culling mask of the main camera if it exists.
	* \return
	*	The culling mask of the main camera if it exists. If
	*	it doesn't exist or it is currently in editor camera,
	*	return a full mask (all layers enabled).
	*/
	uint64_t GetMainCamMask();
#ifdef EditorFlag
	/*!
	* \brief
	*	getter for isEditorCam
	* \return
	*	if current cam is editor cam
	*/
	bool IsEditorCam();
	/*!
	* \brief
	*	setter for isEditorCam
	* \return
	*	if current cam is editor cam
	*/
	void IsEditorCam(bool isEditor);
	/*!
	* \brief
	*	switch between game camera and editor camera
	*/
	void ToggleEditorCam();
	/*!
	* \brief
	*	update editor camera based on keyboard input
	*	to be called in InputActions.h
	*/
	void UpdateEditorCameraKeyboardInput(float dt);
	/*!
	* \brief
	*	update editor camera based on mouse input
	*	to be called in InputActions.h
	*/
	void UpdateEditorCameraMouseInput();
	/*!
	* \brief
	*	update editor camera based on mouse scroll
	*	to be called in InputActions.h
	*/
	void HandleMouseScroll(float scrollOffset);
	/*!
	* \brief
	*	recompute editor camera matrices based on properties
	*/
	void UpdateEditorCamera();
	/*!
	* \brief
	*	reset zoom of editor camera
	*/
	void ResetEditorCameraZoom();

	/*!
	* \brief
	*	sets editor camera's viewport size
	*/
	void SetEditorCameraViewportSize(Vec2 viewportSize);

	/*!
	* \brief
	*	sets editor camera position to position
	*/
	void SetEditorCameraPosition(Vec2 pos);

	static Vec2 EditorPosToViewportPos(Vec2 mousePos);
#endif
	/*!
	* \brief
	*	getter for isViewportDirty
	*/
	bool IsViewportDirty();
	/*!
	* \brief
	*	setter for isViewportDirty
	* \param
	*	isDirty - new value to set
	*/
	void IsViewportDirty(bool isDirty);
	/*!
	* \brief
	*	getter for aspectRatio
	*/
	float AspectRatio();

	/*!
	* \brief
	*	transforms screen coordinates into camera view position
	*/
	Vec2 ScreenToViewPos(CameraComponent const& camera, Vec2 screenPos);

	/*!
	* \brief
	*	transforms screen coordinates into camera view position
	*/
	Vec2 ScreenToViewPos(Vec2 screenPos);

	/*!
	* \brief
	*	transforms screen coordinates into world
	*/
	Vec2 ScreenToWorldPos(CameraComponent const& camera, Vec2 screenPos);

	/*!
	* \brief
	*	transforms screen coordinates into world coordinates
	*/
	Vec2 ScreenToWorldPos(Vec2 screenPos);


	Vec2 GetScreenSize();

private:
	Registry::Entity mainCamera = 0; // entity with camera component that is the main camera
#ifdef EditorFlag
	bool isEditorCamera = true;
	bool imguiHidden = false;
	bool isFullscreen = false;
	EditorCamera editorCamera;
	Vec2 editorViewportSize;
	Vec2 editorViewportOrigin;

	void OnEditorViewportMoved(Vec2 newPos);
	void OnEditorViewportResized(Vec2 newSize);
	void OnImGuiHidden(bool hidden);
public:
	bool suppressEditorCameraMouseScroll = false;
	bool suppressEditorCameraMouseMovement = false;
	bool suppressEditorCameraKeyboardMovement = false;
#endif
};

/*!
* \brief
*	recompute all camera component's matrices based on their transform
*/
void UpdateCameras(Registry& registry);

