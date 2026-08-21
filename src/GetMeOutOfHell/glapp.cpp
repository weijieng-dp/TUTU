/*!
@file       glapp.cpp
@author     pghali@digipen.edu
@co-author  parminder.singh@digipen.edu
@co-author  Ng Wei Jie (weijie.ng) (89%)
@co-author  Tan Jun Jie (t.junjie) (10%)
@co-author  Zhang Mingyang (mingyang.zhang) (1%)
@date       05/05/2025

This file implements functionality useful and necessary to build OpenGL
applications including use of external APIs such as GLFW to create a
window and start up an OpenGL context and to extract function pointers
to OpenGL implementations.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
#include "Platform.h"
#include "glapp.h"
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "ComponentLoader.h"
#include "Json.h"

#include "ResourceManager.h"
#include "DebugRender.h"
#include "GraphicsSystem.h"
#include "Animation.h"

#include <cstdlib>
#include "editor.h"

#include "InputManager.h"
#include "Camera.h"
#include "AudioManager.h"

#include "PhysicsSystem.h"
#include "UIManager.h"
#include "ScreenManager.h"
#include "SceneManager.h"
#include "UpdateStackManager.h"

#include "LayerManager.h"

#include "EventsDispatcher.h"
#include "HierarchyManager.h"

#include "ParticleSystem.h"
#include "MapManager.h"
#include "GameObjects.h"
#include "AchievementManager.h"
#include "videoManager.h"
#include "ItemManager.h"
#include <UserSettingsManager.h>

// Before asking GLFW to create an OpenGL context, we specify the minimum constraints
// in that context:
#define GL_MINOR 0
#define GL_MAJOR 3

/*                                                   objects with file scope
----------------------------------------------------------------------------- */
GLint GLApp::width{};
GLint GLApp::height{};
GLint GLApp::configWidth{};
GLint GLApp::configHeight{};
std::string GLApp::ost;

GLfloat GLApp::ar;
double GLApp::fps;
double GLApp::delta_time;
std::string GLApp::title;
#ifdef  PLATFORM_WINDOWS
GLFWwindow* GLApp::ptr_window;
bool GLApp::isFullscreen{ false };
#else
#endif

bool GLApp::ImguiFlag = true;
bool GLApp::recompiling = false;
bool GLApp::displayFPS = false;

GLboolean GLApp::keystateW{ GL_FALSE };
GLboolean GLApp::keystateA{ GL_FALSE };
GLboolean GLApp::keystateS{ GL_FALSE };
GLboolean GLApp::keystateD{ GL_FALSE };

#ifdef PLATFORM_WINDOWS
std::map<std::string, GLFWcursor*> cursors;
GLFWcursor* currCursor = nullptr;
unsigned char* pixels;
GLFWimage image;


void joystick_callback(int jid, int event)
{
	if (event == GLFW_CONNECTED)
	{
		if(jid == 0)
		CEO::Get<EventsDispatcher>()->Dispatch<Events::JoystickConnected>(Events::JoystickConnected{ 0});

		LOGI("heheheheha %d",jid);
	}
	else if (event == GLFW_DISCONNECTED)
	{
		// The joystick was disconnected
		if(jid ==0)
		CEO::Get<EventsDispatcher>()->Dispatch<Events::JoystickDisconnected>(Events::JoystickDisconnected{ 0 });

		LOGI("GRRRRRR %d",jid);

	}
}
#endif
/*  _________________________________________________________________________ */
/*! init

@param GLint width
@param GLint height
Dimensions of window requested by program

@param std::string title_str
String printed to window's title bar

@return bool
true if OpenGL context and GLEW were successfully initialized.
false otherwise.

Uses GLFW to create OpenGL context. GLFW's initialization follows from here:
http://www.glfw.org/docs/latest/quick.html
a window of size width x height pixels
and its associated OpenGL ES context that matches a profile that is
compatible with OpenGL ES 3.0, has 32-bit RGBA,
double-buffered color buffer, 24-bit depth buffer and 8-bit stencil buffer
with each buffer of size width x height pixels
*/
bool GLApp::init(Registry& registry, ComponentRegistry& compRegistry) {
	LOGI("Entered init in GLAPP INIT");


#ifdef PLATFORM_WINDOWS

	json j("Assets/config.json");

	auto obj = j.GetObjVec("Window Configuration");


	GLApp::width = *(obj[0]->GetValue("Width")->GetInt());
	GLApp::height = *(obj[0]->GetValue("Height")->GetInt());
	GLApp::title = *(obj[0]->GetValue("Title")->GetString());
	GLApp::ost = *(obj[0]->GetValue("BGM")->GetString());
	GLApp::ar = static_cast<GLfloat>(width) / height;
	GLApp::configWidth = width;
	GLApp::configHeight = height;

	GLApp::isFullscreen = { *(obj[0]->GetValue("Fullscreen")->GetBool()) };
#else
	json j("config.json");

	auto obj = j.GetObjVec("Window Configuration");
#endif // PLATFORM_WINDOWS

#ifdef  PLATFORM_WINDOWS
	if (!glfwInit()) {
		std::cout << "GLFW init has failed - abort program!!!" << std::endl;
		return false;
	}

	glfwSetJoystickCallback(joystick_callback);
	// In case a GLFW function fails, an error is reported to callback function
	glfwSetErrorCallback(GLApp::error_cb);

	// Try OpenGL ES first, fallback to desktop OpenGL if it fails
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, GL_MAJOR);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, GL_MINOR);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

	// applications will be double-buffered ...
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
	// default behavior: colorbuffer is 32-bit RGBA, depthbuffer is 24-bits
	// don't change size of window
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);     // window is not resizeable
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

	glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

	// we'll use the entire window as viewport ...
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	// size of viewport: width x height
	if (GLApp::isFullscreen) {    // check whether to start in fullscreen mode
		GLApp::ptr_window = glfwCreateWindow(mode->width, mode->height, title.c_str(), glfwGetPrimaryMonitor(), NULL);
	}
	else {    // start in windowed mode
		GLApp::ptr_window = glfwCreateWindow(configWidth, configHeight, title.c_str(), NULL, NULL);
		int posX{ (mode->width - configWidth) / 2 },      // calculate the x position to center the application
			posY{ (mode->height - configHeight) / 2 };    // calculate the y position to center the application
		glfwSetWindowPos(ptr_window, posX, posY);
	}

  if (!GLApp::ptr_window) {
    std::cerr << "OpenGL ES context creation failed, trying desktop OpenGL...\n";
    
    // Configure GLFW for OpenGL ES
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		if (GLApp::isFullscreen) {
			GLApp::ptr_window = glfwCreateWindow(mode->width, mode->height, title.c_str(), glfwGetPrimaryMonitor(), NULL);
		}
		else {
			GLApp::ptr_window = glfwCreateWindow(configWidth, configHeight, title.c_str(), NULL, NULL);
			int posX{ (mode->width - configWidth) / 2 },
				posY{ (mode->height - configHeight) / 2 };
			glfwSetWindowPos(ptr_window, posX, posY);
		}


		if (!GLApp::ptr_window) {
			std::cerr << "GLFW unable to create any OpenGL context - abort program\n";
			glfwTerminate();
			return false;
		}
		std::cout << "Successfully created desktop OpenGL context\n";
	}
	else {
		std::cout << "Successfully created OpenGL ES context\n";
	}
	glfwMakeContextCurrent(GLApp::ptr_window);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		printf("Failed to initialize GLEW");
		return false;
	}

	// make the previously created OpenGL context current ...
	// set callback for events associated with window size changes; keyboard;
   // mouse buttons, cursor position, and scroller
	glfwSetFramebufferSizeCallback(GLApp::ptr_window, GLApp::fbsize_cb);
	glfwSetKeyCallback(GLApp::ptr_window, GLApp::key_cb);
	glfwSetMouseButtonCallback(GLApp::ptr_window, GLApp::mousebutton_cb);
	glfwSetCursorPosCallback(GLApp::ptr_window, GLApp::mousepos_cb);
	glfwSetScrollCallback(GLApp::ptr_window, GLApp::mousescroll_cb);
	if (*(obj[0]->GetValue("Lose Focus when Alt Tabbed")->GetBool())) glfwSetWindowFocusCallback(ptr_window, focus_cb);

	// this is the default setting ...
	glfwSetInputMode(GLApp::ptr_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// initialize OpenGL (and extension) function loading library
	GLenum err = glewInit();
	if (GLEW_OK != err) {
		std::cerr << "Unable to initialize GLEW - error: "
			<< glewGetErrorString(err) << " abort program" << std::endl;
		return false;
	}

#ifdef _DEBUG
	std::cout << "Using glew version: " << glewGetString(GLEW_VERSION) << std::endl;
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;
#endif
	const GLubyte* glVersion = glGetString(GL_VERSION);
	std::cout << "OpenGL Version String: " << glVersion << std::endl;

	int major, minor;
	std::string versionStr = reinterpret_cast<const char*>(glVersion);
	std::cout << "Parsing version string: " << versionStr << std::endl;

	// Check if this is OpenGL ES or desktop OpenGL
	size_t esPos = versionStr.find("OpenGL ES ");
	if (esPos != std::string::npos) {
		// Parse OpenGL ES version string (e.g., "OpenGL ES 3.2 NVIDIA 566.26")
		size_t versionStart = esPos + 10; // Length of "OpenGL ES "
		size_t spacePos = versionStr.find(' ', versionStart);
		std::string versionPart = versionStr.substr(versionStart, spacePos - versionStart);

		if (sscanf(versionPart.c_str(), "%d.%d", &major, &minor) == 2) {
			std::cout << "Parsed OpenGL ES version: " << major << "." << minor << std::endl;

			if (major > GL_MAJOR || (major == GL_MAJOR && minor >= GL_MINOR)) {
				std::cout << "OpenGL ES " << major << "." << minor << " is supported (requested 3.0+)." << std::endl;
			}
			else {
				std::cerr << "OpenGL ES 3.0+ is not supported. Available: " << major << "." << minor << std::endl;
				return false;
			}
		}
		else {
			std::cerr << "Failed to parse OpenGL ES version from: " << versionStr << std::endl;
			return false;
		}
	}
	else {
		// Parse desktop OpenGL version string (e.g., "4.1.0 NVIDIA 566.26")
		if (sscanf(versionStr.c_str(), "%d.%d", &major, &minor) == 2) {
			std::cout << "Parsed desktop OpenGL version: " << major << "." << minor << std::endl;

			if (major >= 4 && minor >= 1) {
				std::cout << "Desktop OpenGL " << major << "." << minor << " is supported (requested 4.1+)." << std::endl;
			}
			else {
				std::cerr << "Desktop OpenGL 4.1+ is not supported. Available: " << major << "." << minor << std::endl;
				return false;
			}
		}
		else {
			std::cerr << "Failed to parse OpenGL version from: " << versionStr << std::endl;
			return false;
		}
	}
	glfwSwapInterval(1);

	glm::ivec2 vp_width_height(0, 0);
	glfwGetFramebufferSize(ptr_window, &vp_width_height.x, &vp_width_height.y);
	GLApp::fbsize_cb(ptr_window, vp_width_height.x, vp_width_height.y);
#else
	Input::SetInstance(new InputAndroid());
#endif


#ifdef PLATFORM_WINDOWS
	json config{ "Assets/config.json" };
	CEO::Instance().GetManager<FileManager>()->Initialize(nullptr);	// Windows only.
#endif
#ifdef PLATFORM_ANDROID
  json config{"config.json"};
    Input::GetControllers()[0].ID = 1;
    Input::GetControllers()[1].ID = 2;
    Input::GetControllers()[0].InitJoystick("ui/game/android_joystick_select.png", "ui/game/android_joystick_outline.png");
    Input::GetControllers()[1].InitJoystick("ui/game/android_attack_select.png", "ui/game/android_joystick_outline.png");

#endif

	ComponentInitialise(registry, compRegistry);

	CEO::Instance().AddManager<GraphicsSystem>();
	CEO::Instance().AddManager<DebugRender>();

	CEO::Instance().GetManager<ResourceManager>()->Init();
	CEO::Instance().GetManager<CameraManager>()->Init(width, height);
	CEO::Instance().GetManager<UserSettingsManager>()->Init("UserSettings.json");
	CollisionSystem::Instance().Init();

	CEO::Instance().GetManager<AchievementManager>()->Init("GameData/AchievementData.json");
	CEO::Instance().GetManager<LayerManager>()->Init(config);
	CEO::Instance().GetManager<GraphicsSystem>()->Init();
	CEO::Instance().GetManager<GraphicsSystem>()->UseTexAsBackground(false);

	CEO::Instance().GetManager<ParticleSystem>()->Init();
	CEO::Instance().GetManager<MapManager>()->Init("GameData/MapTileData.json", "GameData/WaveData.json");
	CEO::Instance().GetManager<ItemManager>()->Init();

	ScreenManager::Init(width, height);
	UIManager::Init(registry, compRegistry);

#ifdef PLATFORM_WINDOWS
	// initialize custom cursor types based on config
	InitCursors(config);

	// set default to custom "pointer" cursor
	if (cursors.find("pointer") != cursors.end())
	{
		currCursor = cursors["pointer"];
		glfwSetCursor(ptr_window, currCursor);
	}
	CEO::Get<EventsDispatcher>()->Subscribe<Events::ChangeCursor>([](Events::ChangeCursor eventObj) {
		if (cursors.find("pointer") != cursors.end())
		{
			currCursor = cursors[eventObj.cursorName];
			glfwSetCursor(ptr_window, currCursor);
		}});
#endif


#ifndef EditorFlag
#ifdef PLATFORM_WINDOWS
		//SceneManager::DeserializeSceneRJson(*(CEO::Instance().GetManager<Registry>()), *(obj[0]->GetValue("EntryScene")->GetString()));
		SceneManager::QueueSceneAction(*(obj[0]->GetValue("EntryScene")->GetString()), SceneManager::CHANGE);
		// CEO::Instance().GetManager<ResourceManager>()->LoadSceneEntities(*(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), *(obj[0]->GetValue("EntryScene")->GetString()));
#else
		SceneManager::QueueSceneAction(*(obj[0]->GetValue("EntryScene")->GetString()), SceneManager::CHANGE);
#endif
#endif

		auto* events = CEO::Instance().GetManager<EventsDispatcher>();

		events->Subscribe<Events::CloseWindow>(
			[](const Events::CloseWindow&) {
#ifndef EditorFlag
#ifdef PLATFORM_WINDOWS
				glfwSetWindowShouldClose(ptr_window, GLFW_TRUE);
#endif
#endif // !EditorFlag
			}
		);

#ifdef PLATFORM_WINDOWS
		events->Subscribe<Events::ChangeWindowMode>([](const Events::ChangeWindowMode&) { changeWindowMode(); });
#endif
		events->DumpListeners();
		return true;
}

void GLApp::ChangeFrameBufferSize(int _width, int _height)
{
#ifdef _DEBUG
	LOGI("Changing screen size: %d, %d", _width, _height);
#endif
	GLApp::width = _width;
	GLApp::height = _height;
	if (width > 0 && height > 0)
	{
		GLApp::ar = static_cast<float>(_width) / _height;
		ScreenManager::ResizeBuffers(_width, _height);
		CEO::Instance().GetManager<CameraManager>()->ChangeScreenSize(Vec2{ static_cast<float>(_width), static_cast<float>(_height) });
		CEO::Instance().GetManager<CameraManager>()->ChangeAspectRatio(ar);
	}

#ifdef PLATFORM_ANDROID
	Input::GetControllers()[0].ScreenSize(Vec2(_width, _height));

	Input::GetControllers()[1].ScreenSize(Vec2(_width, _height));
#endif
}

void GLApp::ChangeFocus(bool isFocused)
{
	CEO::Instance().GetManager<ResourceManager>()->PauseAllAudio(!isFocused);
	// We need to pause the game in some other form. Something like the main menu screen and map editor won't need pausing but gameplay would
	if (!isFocused)
		CEO::Instance().GetManager<UpdateStackManager>()->IncrementStack();
	else
		CEO::Instance().GetManager<UpdateStackManager>()->DecrementStack();

#ifdef PLATFORM_WINDOWS
#ifdef EditorFlag
	if (!isFocused)
		if (Editor::Instance().GetState() == Editor::EditorState::Play) {
			Editor::Instance().SetState(Editor::EditorState::Pause);
		}
#endif

#endif
}

void GLApp::Update(Registry& registry) {
	// write window title with current fps ...
	auto entities = registry.GetEntitiesWithComponents<SpriteRendererComponent, AnimatorComponent>();
	CEO::Get<ResourceManager>()->UpdateBGMQueue(static_cast<float>(delta_time));

#ifndef PLATFORM_WINDOWS

	if (Input::GetControllers()[0].active)
	{
		Input::GetControllers()[0].UpdateJoystick();
	}
	if (Input::GetControllers()[1].active)
	{
		Input::GetControllers()[1].UpdateJoystick();
	}

#else

#endif

	CEO::Instance().GetManager<EventsDispatcher>()->DispatchQueue();
	UIManager::UpdateUIState();
	//TestMovementUpdate(registry);       // physics movement based on WASD
	UpdateAnimatedSprites(registry, static_cast<float>(delta_time));

	CEO::Instance().GetManager<HierarchyManager>()->UpdateActiveHierarchy();
	UIManager::UpdateUITransform(registry);
	UpdateTransform(registry);
	UpdateCameras(registry);
	CEO::Instance().GetManager<ParticleSystem>()->Update(registry, static_cast<float>(delta_time));

	CEO::Instance().GetManager<GraphicsSystem>()->Update(registry);



	// Advance video decode once per frame using engine delta time.
	VideoManager::Tick(static_cast<double>(delta_time));

	// Apply video textures to render components after decode/upload updates.
	VideoManager::SyncVideoComponents(registry);
}

void GLApp::EditorUpdate(Registry& registry)
{
	CEO::Instance().GetManager<EventsDispatcher>()->DispatchQueue();
	CEO::Instance().GetManager<HierarchyManager>()->UpdateActiveHierarchy();
	UIManager::UpdateUITransform(registry);
	UpdateTransform(registry);
	UpdateCameras(registry);
	CEO::Instance().GetManager<GraphicsSystem>()->Update(registry);
}

void GLApp::FixedUpdate(Registry& registry, float fixedDt)
{
	(void)fixedDt;
	CollisionSystem::Instance().Update(registry);   // update the collision system
}

#define UNUSED_PARAM(P) (void)P
void GLApp::PostRenderUpdate(Registry& registry)
{
	UNUSED_PARAM(registry);
	CEO::Instance().GetManager<SceneManager>()->PostRenderUpdate();
	CEO::Instance().GetManager<GameObjectTracker>()->DestroyGameObjectQueue();

}

#define UNUSED_PARAM(P) (void)P
void GLApp::EditorPostRenderUpdate(Registry& registry)
{
	UNUSED_PARAM(registry);
	CEO::Instance().GetManager<SceneManager>()->PostRenderUpdate();
}

/*  _________________________________________________________________________ */
/*! draw

@param none

@return none

For now, there is nothing to do except set the back buffer fill color
as RGB(0, 1, 0).
*/
void GLApp::draw(Registry& registry) {
	CEO::Get<GraphicsSystem>()->UpdateMinimap(registry);

	glViewport(0, 0, width, height);
	glClearDepthf(0.f);
	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ScreenManager::screenBuffer.Bind();
	CEO::Get<GraphicsSystem>()->Draw(registry);

	if (displayFPS)
	{
		glDisable(GL_DEPTH_TEST);
		std::stringstream so;
		so << "FPS: " << std::fixed << std::setprecision(2) << GLApp::fps;
		CameraManager& camManager = *CEO::Get<CameraManager>();
		CEO::Instance().Get<RenderUtils>()->RenderText(so.str(), camManager.GetProjection(), Vec2{ 0,0 }, 64, Color{ 1.f,1.f,1.f,1.f });
		glEnable(GL_DEPTH_TEST);
	}

	ScreenManager::screenBuffer.Unbind();

	glClearColor(0.f, 0.f, 0.f, 0.f);
	glClearDepthf(0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

#ifdef EditorFlag
	if (!GLApp::ImguiFlag) { ScreenManager::screenBuffer.Draw(); }
#endif

#ifdef PLATFORM_ANDROID
	ScreenManager::screenBuffer.Draw();
	glDisable(GL_DEPTH_TEST);
	if (Input::GetControllers()[0].active)
		Input::GetControllers()[0].DrawJoystick(CEO::Instance().GetManager<ResourceManager>()->GetShader("sprite"), Vec2(width, height));
	if (Input::GetControllers()[1].active)
		Input::GetControllers()[1].DrawJoystick(CEO::Instance().GetManager<ResourceManager>()->GetShader("sprite"), Vec2(width, height));

	glEnable(GL_DEPTH_TEST);
#else
#ifndef EditorFlag
	ScreenManager::screenBuffer.Draw();
#endif // EditorFlag
#endif
	// 

	//CEO::Get<GraphicsSystem>()->DrawMinimap(registry);


}

void GLApp::cleanup() {
	UIManager::Free();
	ScreenManager::Free();
	Input::Shutdown();
	CollisionSystem::Instance().Free();
	CEO::Instance().GetManager<GraphicsSystem>()->Free();
	CEO::Instance().GetManager<ResourceManager>()->Free();
	CEO::Instance().GetManager<CameraManager>()->Free();

#ifdef  PLATFORM_WINDOWS
	glfwTerminate();
#endif
}

#ifdef  PLATFORM_WINDOWS


/*  _________________________________________________________________________*/
/*! key_cb

@param GLFWwindow*
Handle to window that is receiving event

@param int
the keyboard key that was pressed or released

@parm int
Platform-specific scancode of the key

@parm int
GLFW_PRESS, GLFW_REPEAT or GLFW_RELEASE
action will be GLFW_KEY_UNKNOWN if GLFW lacks a key token for it,
for example E-mail and Play keys.

@parm int
bit-field describing which modifier keys (shift, alt, control)
were held down

@return none

This function is called when keyboard buttons are pressed.
When the ESC key is pressed, the close flag of the window is set.
*/
void GLApp::key_cb(GLFWwindow* pwin, int key, int scancode, int action, int mod) {
	(void)pwin; (void)key; (void)scancode; (void)action; (void)mod;
	if (GLFW_PRESS == action) {

#ifdef _DEBUG
		//std::cout << "Key pressed" << std::endl;
#endif
	}
	else if (GLFW_REPEAT == action) {
#ifdef _DEBUG
		//std::cout << "Key repeatedly pressed" << std::endl;
#endif
	}
	else if (GLFW_RELEASE == action) {
#ifdef _DEBUG
		//std::cout << "Key released" << std::endl;
#endif
	}

	//if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action) {
	//  glfwSetWindowShouldClose(pwin, GLFW_TRUE);
	//}
}

/*  _________________________________________________________________________*/
/*! mousebutton_cb

@param GLFWwindow*
Handle to window that is receiving event

@param int
the mouse button that was pressed or released
GLFW_MOUSE_BUTTON_LEFT and GLFW_MOUSE_BUTTON_RIGHT specifying left and right
mouse buttons are most useful

@parm int
action is either GLFW_PRESS or GLFW_RELEASE

@parm int
bit-field describing which modifier keys (shift, alt, control)
were held down

@return none

This function is called when mouse buttons are pressed.
*/
void GLApp::mousebutton_cb(GLFWwindow* pwin, int button, int action, int mod) {
	(void)pwin; (void)button; (void)action; (void)mod;
	switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
#ifdef _DEBUG
		//std::cout << "Left mouse button ";
#endif
		break;
	case GLFW_MOUSE_BUTTON_RIGHT:
#ifdef _DEBUG
		// std::cout << "Right mouse button ";
#endif
		break;
	}
	switch (action) {
	case GLFW_PRESS:
#ifdef _DEBUG
		//std::cout << "pressed!!!" << std::endl;
#endif
		break;
	case GLFW_RELEASE:
#ifdef _DEBUG
		// std::cout << "released!!!" << std::endl;
#endif
		break;
	}
}

/*  _________________________________________________________________________*/
/*! mousepos_cb

@param GLFWwindow*
Handle to window that is receiving event

@param double
new cursor x-coordinate, relative to the left edge of the client area

@param double
new cursor y-coordinate, relative to the top edge of the client area

@return none

This functions receives the cursor position, measured in screen coordinates but
relative to the top-left corner of the window client area.
*/
void GLApp::mousepos_cb(GLFWwindow* pwin, double xpos, double ypos) {
	(void)pwin; (void)xpos; (void)ypos;
#ifdef _DEBUG
	//std::cout << "Mouse cursor position: (" << xpos << ", " << ypos << ")" << std::endl;
#endif
#ifdef EditorFlag
	if (!ImguiFlag)
		UIManager::gameMousePos = { static_cast<float>(xpos), static_cast<float>(ScreenManager::screenBuffer.Height() - ypos) };
#else
	UIManager::gameMousePos = { static_cast<float>(xpos), static_cast<float>(ScreenManager::screenBuffer.Height() - ypos) };
#endif
}

/*  _________________________________________________________________________*/
/*! mousescroll_cb

@param GLFWwindow*
Handle to window that is receiving event

@param double
Scroll offset along X-axis

@param double
Scroll offset along Y-axis

@return none

This function is called when the user scrolls, whether with a mouse wheel or
touchpad gesture. Although the function receives 2D scroll offsets, a simple
mouse scroll wheel, being vertical, provides offsets only along the Y-axis.
*/
void GLApp::mousescroll_cb(GLFWwindow* pwin, double xoffset, double yoffset) {
	(void)pwin; (void)xoffset; (double)yoffset;
#ifdef _DEBUG
	//std::cout << "Mouse scroll wheel offset: ("
	//  << xoffset << ", " << yoffset << ")" << std::endl;
#endif
	CEO::Get<EventsDispatcher>()->Dispatch<Events::MouseScroll>(Events::MouseScroll{ static_cast<float>(yoffset) });
}
#endif

/*  _________________________________________________________________________ */
/*! error_cb

@param int
GLFW error code

@parm char const*
Human-readable description of the code

@return none

The error callback receives a human-readable description of the error and
(when possible) its cause.
*/
void GLApp::error_cb(int error, char const* description) {
	(void)error;
	std::cerr << "GLFW error: " << description << std::endl;
}
#ifdef  PLATFORM_WINDOWS
/*  _________________________________________________________________________ */
/*! fbsize_cb

@param GLFWwindow*
Handle to window that is being resized

@parm int
Width in pixels of new window size

@parm int
Height in pixels of new window size

@return none

This function is called when the window is resized - it receives the new size
of the window in pixels.
*/
void GLApp::fbsize_cb(GLFWwindow* ptr_win, int _width, int _height) {
	(void)ptr_win;
	// use the entire framebuffer as drawing region
	glViewport(0, 0, _width, _height);

	ChangeFrameBufferSize(_width, _height);
	// later, if working in 3D, we'll have to set the projection matrix here ...
}

void GLApp::changeWindowMode() {
	isFullscreen = !isFullscreen;
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	if (isFullscreen) {   // change windowmode to fullscreen
		glfwSetWindowMonitor(ptr_window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
	}
	else {
		int posX{ (mode->width - configWidth) / 2 };
		int posY{ (mode->height - configHeight) / 2 };
		glfwSetWindowMonitor(ptr_window, NULL, posX, posY, configWidth, configHeight, NULL);
	}
}

void GLApp::focus_cb(GLFWwindow* window, int focused) {
	if (!focused)
		glfwIconifyWindow(window);

	ChangeFocus(focused);
}


#endif

#ifdef PLATFORM_WINDOWS
void GLApp::update_time(double fps_calc_interval) {
	// get elapsed time (in seconds) between previous and current frames
	static double prev_time = glfwGetTime();
	double curr_time = glfwGetTime();
	GLApp::delta_time = curr_time - prev_time;
	prev_time = curr_time;

	// fps calculations
	static double count = 0.0; // number of game loop iterations
	static double start_time = glfwGetTime();
	// get elapsed time since very beginning (in seconds) ...
	double elapsed_time = curr_time - start_time;

	++count;

	// update fps at least every 10 seconds ...
	fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
	fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
	if (elapsed_time > fps_calc_interval) {
		GLApp::fps = count / elapsed_time;
		start_time = curr_time;
		count = 0.0;
	}
}
#else

inline double GetTimeSeconds() {
	using clock = std::chrono::steady_clock;
	static auto start = clock::now();
	auto now = clock::now();
	return std::chrono::duration<double>(now - start).count();
}

void GLApp::update_time(double fps_calc_interval) {
	// get elapsed time (in seconds) between previous and current frames
	static double prev_time = GetTimeSeconds();
	double curr_time = GetTimeSeconds();
	GLApp::delta_time = curr_time - prev_time;
	prev_time = curr_time;

	// fps calculations
	static double count = 0.0; // number of game loop iterations
	static double start_time = GetTimeSeconds();
	// get elapsed time since very beginning (in seconds) ...
	double elapsed_time = curr_time - start_time;

	++count;

	// update fps at least every 10 seconds ...
	fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
	fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
	if (elapsed_time > fps_calc_interval) {
		GLApp::fps = count / elapsed_time;
		start_time = curr_time;
		count = 0.0;
	}
}
#endif

void GLApp::UpdateTransform(Registry& registry)
{
	Mat3 scale{}, rotate{}, trans{};

	for (auto entity : CEO::Instance().GetManager<HierarchyManager>()->GetHierarchyList())
	{
		TransformComponent& transform = *registry.GetComponent<TransformComponent>(entity);
		HierarchyComponnent& HC = *registry.GetComponent<HierarchyComponnent>(entity);

		// mdl transform
		if (!&transform) continue;
		if (HC.parent == 0)
		{
			scale = Mat3::Scale(transform.scale.x, transform.scale.y);
			rotate = Mat3::Rotation(transform.rotation);
			trans = Mat3::Translation(transform.translate.x, transform.translate.y);

			transform.transform = trans * rotate * scale;
		}
		else
		{
			TransformComponent& ParentTransform = *registry.GetComponent<TransformComponent>(HC.parent);

			scale = Mat3::Scale(transform.scale.x, transform.scale.y);
			rotate = Mat3::Rotation(transform.rotation);
			trans = Mat3::Translation(transform.translate.x, transform.translate.y);

			transform.transform = ParentTransform.transform * trans * rotate * scale;
		}
	}
}

void GLApp::TestMovementUpdate(Registry& registry) {
	// get entities that are active (playable / moveable characters will have active = true)
	auto entities = registry.GetEntitiesWithComponents<AnimatorComponent, ActiveComponent, PhysicsComponent>();
	GLfloat dt{ static_cast<float>(GLApp::delta_time) };
	for (auto entity : entities) {
		PhysicsComponent* physics{ registry.GetComponent<PhysicsComponent>(entity) };
		//AnimatorComponent* animator{ registry.GetComponent<AnimatorComponent>(entity) };
		auto active = registry.GetComponent<ActiveComponent>(entity)->isActiveSelf && registry.GetComponent<ActiveComponent>(entity)->isActiveInHierarchy;

		if (!active) continue;
		//velocity-based movement because its more normal
		if (keystateW && !keystateS) {
			physics->velocity.y = 300.f;
#ifdef EditorFlag
			if (!CEO::Instance().GetManager<CameraManager>()->IsEditorCam())
#endif
				CEO::Instance().GetManager<GraphicsSystem>()->backgroundOffset.y += dt;
		}
		else if (keystateS && !keystateW) {
			physics->velocity.y = -300.f;
#ifdef EditorFlag
			if (!CEO::Instance().GetManager<CameraManager>()->IsEditorCam())
#endif
				CEO::Instance().GetManager<GraphicsSystem>()->backgroundOffset.y -= dt;
		}
		else {
			physics->velocity.y = 0.f;
		}

		if (keystateA && !keystateD) {
			physics->velocity.x = -300.f;
#ifdef EditorFlag
			if (!CEO::Instance().GetManager<CameraManager>()->IsEditorCam())
#endif
				CEO::Instance().GetManager<GraphicsSystem>()->backgroundOffset.x -= dt;
		}
		else if (keystateD && !keystateA) {
			physics->velocity.x = 300.f;
#ifdef EditorFlag
			if (!CEO::Instance().GetManager<CameraManager>()->IsEditorCam())
#endif
				CEO::Instance().GetManager<GraphicsSystem>()->backgroundOffset.x += dt;
		}
		else {
			physics->velocity.x = 0.f;
		}

		//if (physics->velocity.x != 0 || physics->velocity.y != 0) {
		//    if (animator->currAnim->animName == "usa_idle.anim" && animator->currAnim->animName != "usa_run.anim") {
		//        animator->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation("usa_run.anim");
		//        registry.GetComponent<SpriteRendererComponent>(entity)->start = { 0.f, 0.f };
		//        animator->currFrame = 0; animator->timeElapsed = 0.f;
		//    }
		//}
		//else {
		//    if (animator->currAnim->animName == "usa_run.anim" && animator->currAnim->animName != "usa_idle.anim") {
		//        animator->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation("usa_idle.anim");
		//        registry.GetComponent<SpriteRendererComponent>(entity)->start = { 0.f, 0.f };
		//        animator->currFrame = 0; animator->timeElapsed = 0.f;
		//    }
		//}

		//uncomment block below if u wish to suffer and move using forces instead

		//const float thrust = 100.f;      // force magnitude per input

		//Vec2 appliedForce{ 0.f, 0.f };
		//if (keystateW && !keystateS) appliedForce.y += thrust;
		//if (keystateS && !keystateW) appliedForce.y -= thrust;
		//if (keystateA && !keystateD) appliedForce.x -= thrust;
		//if (keystateD && !keystateA) appliedForce.x += thrust;

		//if (appliedForce.x != 0.f || appliedForce.y != 0.f){
		//    PhysicsSystem::Instance().ApplyForce(entity, appliedForce, registry);
		//}
	}
}
#ifdef PLATFORM_WINDOWS

void GLApp::InitCursors(json data)
{
	ResourceManager& resourceManager = *CEO::Get<ResourceManager>();

	// grab all cursors from config
	for (json::object* cursorObj : data.GetObjVec("Cursors"))
	{
		// get member variables from config
		std::string cursorName = *cursorObj->GetValue("name")->GetString();
		std::string texPath = *cursorObj->GetValue("texPath")->GetString();
		int hotX = *cursorObj->GetValue("hotX")->GetInt();
		int hotY = *cursorObj->GetValue("hotY")->GetInt();

		TextureObj const& tex = resourceManager.GetTexture(texPath);

		if (!tex.HasPixelData())
		{
			LOGE("No pixel data in provided custom cursor texture");
			return;
		}

		// construct image
		image.width = tex.Width();
		image.height = tex.Height();
		unsigned char const* pixData = tex.PixelData();
		int colorChannels = tex.Channels();
		unsigned char* flippedPixData = new unsigned char[image.width * image.height * colorChannels];

		// flip image data, glfw reads textures differently
		for (int y = 0; y < image.height; ++y)
		{
			for (int x = 0; x < image.width; ++x)
			{
				for (int channeli = 0; channeli < colorChannels; ++channeli)
				{
					flippedPixData[((image.height - y - 1) * image.width + x) * colorChannels + channeli] = pixData[(y * image.width + x) * colorChannels + channeli];
				}
			}
		}
		image.pixels = flippedPixData;

		// create a cursor with image we constructed earlier
		cursors[cursorName] = glfwCreateCursor(&image, hotX, hotY);

		// glfwCreateCursor creates a copy of our flipped data, safe to delete
		delete[] flippedPixData;
	}
}

//void GLApp::SetCustomCursorTex(TextureObj const& tex, int xHot, int yHot)
//{
//#ifdef PLATFORM_WINDOWS
//    if (!tex.HasPixelData())
//    {
//        LOGE("No pixel data in provided custom cursor texture");
//        return;
//    }
//
//    image.width = tex.Width();
//    image.height = tex.Height();
//    unsigned char const* pixData = tex.PixelData();
//    int colorChannels = tex.Channels();
//    unsigned char* flippedPixData = new unsigned char[image.width * image.height * colorChannels];
//
//
//    // flip image data
//    for (int y = 0; y < image.height; ++y)
//    {
//        for (int x = 0; x < image.width; ++x)
//        {
//            for (int channeli = 0; channeli < colorChannels; ++channeli)
//            {
//               flippedPixData[((image.height - y - 1) * image.width + x) * colorChannels + channeli] = pixData[(y * image.width + x) * colorChannels+ channeli];
//            }
//        }
//    }
//    image.pixels = flippedPixData;
//    cursorObj = glfwCreateCursor(&image, xHot, yHot);
//
//    // glfwCreateCursor creates a copy of our flipped data, safe to delete
//    delete[] flippedPixData;
//
//    glfwSetCursor(ptr_window, cursorObj);
//#endif
//}

void GLApp::SetCustomCursor()
{
	glfwSetCursor(ptr_window, currCursor);
}

bool InputGLFW::IsKeyHeldImpl(int KeyCode)
{

	auto state = glfwGetKey(GLApp::ptr_window, KeyCode);
	return state == GLFW_PRESS || state == GLFW_REPEAT;

}

bool InputGLFW::IsButtonHeldImpl(int ButtonCode)
{
	auto state = glfwGetMouseButton(GLApp::ptr_window, ButtonCode);
	return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool InputGLFW::IsKeyPressedImpl(int KeyCode)
{
	return KeyState[KeyCode].PressedState;

}

bool InputGLFW::IsButtonPressedImpl(int ButtonCode)
{
	return ButtonState[ButtonCode].PressedState;

}

bool InputGLFW::IsKeyReleaseImpl(int KeyCode)
{
	return KeyState[KeyCode].ReleasedState;
}

bool InputGLFW::IsButtonReleaseImpl(int ButtonCode)
{
	return ButtonState[ButtonCode].ReleasedState;
}

float InputGLFW::GetMouseXImpl()
{
	double PosX, PosY;
	glfwGetCursorPos(GLApp::ptr_window, &PosX, &PosY);

	return static_cast<float>(PosX);
}

float InputGLFW::GetMouseYImpl()
{
	double PosX, PosY;
	glfwGetCursorPos(GLApp::ptr_window, &PosX, &PosY);

	return static_cast<float>(PosY);
}
void InputGLFW::UpdateImpl()
{
	for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key) {
		int state = glfwGetKey(GLApp::ptr_window, key);
		bool isPressed = (state == GLFW_PRESS || state == GLFW_REPEAT);

		// Store current state
		KeyState[key].CurrentState = isPressed;

		// Detect edges
		KeyState[key].PressedState = (isPressed && !KeyState[key].PreviousState);
		KeyState[key].ReleasedState = (!isPressed && KeyState[key].PreviousState);

		// Update previous
		KeyState[key].PreviousState = isPressed;
	}
	for (int Button = GLFW_MOUSE_BUTTON_1; Button <= GLFW_MOUSE_BUTTON_8; ++Button) {
		int state = glfwGetMouseButton(GLApp::ptr_window, Button);
		bool isPressed = (state == GLFW_PRESS || state == GLFW_REPEAT);

		// Store current state
		ButtonState[Button].CurrentState = isPressed;

		// Detect edges
		ButtonState[Button].PressedState = (isPressed && !ButtonState[Button].PreviousState);
		ButtonState[Button].ReleasedState = (!isPressed && ButtonState[Button].PreviousState);

		// Update previous
		ButtonState[Button].PreviousState = isPressed;
	}

	

	for (int i = 0; i < static_cast<int>(m_gamepads.size()); ++i)
	{
		auto& pad = m_gamepads[i];


		for (int b = 0; b < GamepadState::ButtonCount; ++b)
			pad.previousButtons[b] = pad.buttons[b];

		int glfwID = GLFW_JOYSTICK_1 + i;

		if (!glfwJoystickPresent(glfwID) || !glfwJoystickIsGamepad(glfwID))
		{
			pad.connected = false;

			for (int b = 0; b < GamepadState::ButtonCount; ++b)
				pad.buttons[b] = false;

			for (int a = 0; a < GamepadState::AxisCount; ++a)
				pad.axes[a] = 0.0f;

			continue;
		}
		
		GLFWgamepadstate state{};
		if (!glfwGetGamepadState(glfwID, &state))
		{
			pad.connected = false;
			continue;
		}

		pad.connected = true;

		for (int b = 0; b < GamepadState::ButtonCount; ++b)
			pad.buttons[b] = (state.buttons[b] == GLFW_PRESS);

		pad.axes[static_cast<int>(GamepadAxis::LEFT_X)] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
		pad.axes[static_cast<int>(GamepadAxis::LEFT_Y)] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y] * -1;
		pad.axes[static_cast<int>(GamepadAxis::RIGHT_X)] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
		pad.axes[static_cast<int>(GamepadAxis::RIGHT_Y)] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] * -1;
		pad.axes[static_cast<int>(GamepadAxis::LEFT_TRIGGER)] = state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER];
		pad.axes[static_cast<int>(GamepadAxis::RIGHT_TRIGGER)] = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER];
	}
}

void InputGLFW::InitImpl()
{
	CEO::Get<EventsDispatcher>()->Subscribe<Events::JoystickConnected>([&](Events::JoystickConnected) {
		m_gamepads[0].connected = true;
		});

	CEO::Get<EventsDispatcher>()->Subscribe<Events::JoystickDisconnected>([&](Events::JoystickDisconnected) {
		m_gamepads[0].connected = false;
		});
}

Input::GamepadState const& InputGLFW::GetGamepadImpl(int index)
{
	static GamepadState dummy{};
	if (index < 0 || index >= static_cast<int>(m_gamepads.size()))
		return dummy;

	return m_gamepads[index];

}


float InputGLFW::GetMouseGameXImpl()
{
#ifdef EditorFlag
	if (CEO::Instance().GetManager<UIManager>()->gameMousePos.x - ScreenManager::screenBuffer.Width() / 2 < -ScreenManager::screenBuffer.Width() / 2)
		return 0;
	return CEO::Instance().GetManager<UIManager>()->gameMousePos.x - ScreenManager::screenBuffer.Width() / 2;

#else
	return GetMouseXImpl() - GLApp::width / 2;
#endif
}

float InputGLFW::GetMouseScreenXImpl()
{
#ifdef EditorFlag
	if (CEO::Instance().GetManager<UIManager>()->gameMousePos.x - ScreenManager::screenBuffer.Width() / 2 < -ScreenManager::screenBuffer.Width() / 2)
		return 0;
	// Registry& registry = *CEO::Instance().GetManager < Registry>();

	return (CEO::Instance().GetManager<UIManager>()->gameMousePos.x - ScreenManager::screenBuffer.Width() / 2) * CEO::Instance().GetManager<CameraManager>()->GetViewportSize().x / CEO::Instance().GetManager <ScreenManager>()->screenBuffer.Width();

#else

	return GetMouseXImpl() - GLApp::width / 2 * CEO::Instance().GetManager<CameraManager>()->GetViewportSize().x / CEO::Instance().GetManager <ScreenManager>()->screenBuffer.Width();
#endif
}

float InputGLFW::GetMouseGameYImpl()
{
#ifdef EditorFlag
	if (CEO::Instance().GetManager<UIManager>()->gameMousePos.y - ScreenManager::screenBuffer.Height() / 2 < -ScreenManager::screenBuffer.Height() / 2)
		return 0;
	return CEO::Instance().GetManager<UIManager>()->gameMousePos.y - ScreenManager::screenBuffer.Height() / 2;
#else
	return -(GetMouseYImpl() - GLApp::height / 2);

#endif
}


float InputGLFW::GetMouseWorldXImpl(Registry&) {
#ifdef EditorFlag
	if (CEO::Instance().GetManager<UIManager>()->gameMousePos.x - ScreenManager::screenBuffer.Width() / 2 < -ScreenManager::screenBuffer.Width() / 2)
		return 0;
	Mat3 inv{ CEO::Instance().GetManager<CameraManager>()->GetViewProjection().Inversed() };
	return (inv.m[0] * GetMouseGameXImpl() + inv.m[3] * GetMouseGameXImpl() + inv.m[6]);
#else
	Mat3 inv{ CEO::Instance().GetManager<CameraManager>()->GetViewProjection().Inversed() };
	return (inv.m[0] * GetMouseGameXImpl() + inv.m[3] * GetMouseGameXImpl() + inv.m[6]);
#endif
}

float InputGLFW::GetMouseWorldYImpl(Registry&) {
#ifdef EditorFlag
	if (CEO::Instance().GetManager<UIManager>()->gameMousePos.x - ScreenManager::screenBuffer.Height() / 2 < -ScreenManager::screenBuffer.Height() / 2)
		return 0;
	Mat3 inv{ CEO::Instance().GetManager<CameraManager>()->GetViewProjection().Inversed() };
	return (inv.m[0] * GetMouseGameYImpl() + inv.m[3] * GetMouseGameYImpl() + inv.m[6]);
#else
	Mat3 inv{ CEO::Instance().GetManager<CameraManager>()->GetViewProjection().Inversed() };
	return (inv.m[0] * GetMouseGameYImpl() + inv.m[3] * GetMouseGameYImpl() + inv.m[6]);
#endif
}
#else

bool InputAndroid::IsPointerHeldImpl(int PointerID)
{
	auto it = State.find(PointerID);
	if (it != State.end())
		return it->second.held;
	return false;
}

bool InputAndroid::IsPointerPressedImpl(int PointerID)
{
	auto it = State.find(PointerID);
	if (it != State.end()) {
		if (it->second.pressed) {
			return true;
		}
	}
	return false;
}

bool InputAndroid::IsPointerReleaseImpl(int PointerID)
{
	auto it = State.find(PointerID);
	if (it != State.end()) {
		if (it->second.released) {
			return true;
		}
	}
	return false;
}

float InputAndroid::GetPointerXImpl(int id) {
	auto it = State.find(id);
	if (it != State.end())
		return (it->second.x - GLApp::width / 2);
	return 0.0f;
}

float InputAndroid::GetPointerScreenXImpl(int id) {
	auto it = State.find(id);
	if (it != State.end())
	{
		Registry& registry = *CEO::Instance().GetManager < Registry>();
		return (it->second.x - GLApp::width / 2) * CEO::Instance().GetManager<CameraManager>()->GetViewportSize().x / CEO::Instance().GetManager <ScreenManager>()->screenBuffer.Width();;
	}
	return 0.0f;
}

float InputAndroid::GetPointerYImpl(int id)
{
	auto it = State.find(id);
	if (it != State.end())
		return -(it->second.y - GLApp::height / 2);
	return 0.0f;
}

void InputAndroid::UpdateTouchImpl(int pointerId, int action, float x, float y) {
	auto& act = SavedState[pointerId];

	switch (action) {
	case 0:
	case 5: // ACTION_POINTER_DOWN
		act.x = x;
		act.y = y;
		act.pressed = true;
		act.held = true;
		act.released = false;
		break;

	case 1: // ACTION_UP
	case 6: // ACTION_POINTER_UP
		act.x = x;
		act.y = y;
		act.released = true;
		act.held = false;
		break;

	case 2: // ACTION_MOVE
		act.x = x;
		act.y = y;
		act.pressed = false;
		// just update pos
		break;
	}
}
void InputAndroid::UpdateTouchImplStart()
{
	State = SavedState;
	for (auto& [pointerid, action] : SavedState)
	{

		action.pressed = false;

		action.released = false;
	}
}
void InputAndroid::UpdateTouchImplEnd() {

	for (auto& [pointerid, action] : State)
	{
		if (action.pressed)
		{
			action.pressed = false;
		}
		if (action.released)
		{
			action.released = false;
		}
	}
}

std::map<int, Input::Action> InputAndroid::GetAllPointerImpl()
{
	return State;
}



#endif
