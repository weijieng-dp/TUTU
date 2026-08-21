/*!
@file    	main.cpp
@author  	pghali@digipen.edu
@co-author  parminder.singh@digipen.edu
@date    	17/04/2024

This file uses functionality defined in type GLApp to initialize an OpenGL
context and implement a game loop.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.


/*____________________________________________________________________________*/

#include "Platform.h"

#ifdef PLATFORM_WINDOWS
#define _CRTDBG_MAP_ALLOC

#ifdef EditorFlag
#define ENABLE_PROFILER__
#endif

/*                                                                   includes
----------------------------------------------------------------------------- */
// Extension loader library's header must be included before GLFW's header!!!
#include "pch.h"
#include "glapp.h"
#include "editor.h"
#include "ScriptingAPI.h"
#include <crtdbg.h>
#include "demo.h"
#include "InputActions.h"
#include "PhysicsSystem.h"
#include "CollisionSystem.h"
#include "Pathfind.h"
#include "SceneManager.h"
#include "UpdateStackManager.h"
#include <Application.h>

/*                                                   type declarations
----------------------------------------------------------------------------- */

/*                                                      function declarations
----------------------------------------------------------------------------- */

static void Draw();
static void Loop(float dt);
static void FixedUpdate(float dt);

static void init(); static void preloopmisc();
static void free(); static void postloopmisc();

static const float fixedDeltaTime = 1.0f / 60.0f;
static float accumulator = 0.0f;
/*                                                   objects with file scope
----------------------------------------------------------------------------- */
namespace {
	
	std::unordered_map<std::string, rtr::TypeInfo> typeMap;
	bool firstframe = true;
}


/*                                                      function definitions
----------------------------------------------------------------------------- */


/*!
* \brief
*	The main fucntion with the main loop.
*	All functions relating to initialization of systems
*	such as openGL is to be placed within init();
*
*	The same is to be done for functions relating to freeing
*	of resources after the program terminates. (to be placed
*	in free())
*
*	The misc() functions are for functions that are more
*	"miscellaneous" in nature, neither an initialization
*	process nor a freeing process, i.e. printing out the
*	profiler information.
*
* \param null
*
* \return
*	[int] status
*/

//change this to main() if doesnt work

#ifndef EditorFlag

int WINAPI wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
	auto coutbuf = std::cout.rdbuf();
	std::stringstream sstr;
	std::cout.rdbuf(sstr.rdbuf());

#else
int main() {

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

#endif


	//ScanDirectoryForClasses("../../../src/Scripts");

	init();
#ifdef EditorFlag
	RecompileScript("../../../src/Scripts", *(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());
	CEO::Get<UpdateStackManager>()->UpdateStorages();
#endif
	preloopmisc();

	
	//try {
		// window's close flag is set by clicking close widget or Alt+F4
	while (!glfwWindowShouldClose(GLApp::ptr_window)) {
		
		PROFILE_FUNCTION__("STARTFRAME")
		static auto currentTime = std::chrono::high_resolution_clock::now();

		auto newTime = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> frameDuration = newTime - currentTime;
		currentTime = newTime;

		float frameTime = frameDuration.count();
		accumulator += (frameTime <= 0.25f? frameTime : 0.25f);

		// process events if any associated with input devices
		glfwPollEvents(); // Note: because of how poll works, our game basically freezes when the window is moved.
		Input::Update();

		InputAction::Action(); // When you want something to happen when you make an input, put it in here if possible
#ifdef EditorFlag


		if (Input::IsKeyPressed(GLFW_KEY_F1) && (Editor::Instance().GetState() == Editor::EditorState::Edit))
		{

				SceneManager::SerializeSceneRJson(*(CEO::Instance().GetManager<Registry>()), "temp");
				RecompileScript("../../../src/Scripts", *(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());
				CEO::Get<UpdateStackManager>()->UpdateStorages();
				SceneManager::DeserializeSceneRJson(*(CEO::Instance().GetManager<Registry>()), "temp");
				for (const auto& entry : std::filesystem::directory_iterator("Assets\\Scripts\\")) {
					// Check if the entry is a regular file
					if (std::filesystem::is_regular_file(entry.path())) {
						Editor::Instance().UpdateAssetsFolders(entry.path().relative_path().string(), "Scripts");
					}
				}

				firstframe = true; 
			
		}
		else if (Editor::Instance().RemoveScriptFile && (Editor::Instance().GetState() == Editor::EditorState::Edit))
		{
			RecompileScript("../../../src/Scripts", *(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());
			CEO::Get<Logger>()->GetLogs().clear();
			RecompileScript("../../../src/Scripts", *(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());
			Editor::Instance().RemoveScriptFile = false;
			CEO::Get<UpdateStackManager>()->UpdateStorages();
		}
		else if (Editor::Instance().AddedScriptFile && (Editor::Instance().GetState() == Editor::EditorState::Edit))
		{
			RecompileScript("../../../src/Scripts", *(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());
			CEO::Get<Logger>()->GetLogs().clear();
			RecompileScript("../../../src/Scripts", *(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());
			Editor::Instance().AddedScriptFile = false;
			CEO::Get<UpdateStackManager>()->UpdateStorages();
		}
#endif
		Loop(frameTime);
		PROFILE_FUNCTION__("ENDFRAME")
	}
	postloopmisc();
	free();

	DeleteScriptInstance(*(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()));

	
	CEO::Instance().Free();
#ifndef EditorFlag
	std::cout.rdbuf(coutbuf);
#endif
	return 1;
}

/*!
* \brief
*	Draw function that handles the drawing loop
*
* \param
*	note: these will be removed later. -junbo
* \param
*	[Registry&] reference to registry
* \param
*	[ComponentRegistry&] reference to componentregistry
*
* \return null
*/
static void Draw() {
	// render scene
	if (!glfwGetWindowAttrib(GLApp::ptr_window, GLFW_ICONIFIED)) {	// check if window is minimised
		// only draw if window isn't minimised

		PROFILE_FUNCTION__("Draw")
		GLApp::draw(*(CEO::Instance().GetManager<Registry>()));
		PROFILE_FUNCTION__()


#ifdef EditorFlag
			PROFILE_FUNCTION__("Draw ImGUI")
			if (GLApp::ImguiFlag)
			{
				Editor::Instance().DrawEditor(*(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), static_cast<float>(GLApp::width), static_cast<float>(GLApp::height));
			}
		PROFILE_FUNCTION__()
		if (!GLApp::ImguiFlag)
#endif //  EditorFlag
				GLApp::SetCustomCursor();
		// swap buffers: front <-> back
		// GLApp::ptr_window is handle to window that defines the OpenGL context
		glfwSwapBuffers(GLApp::ptr_window);
	}
}

/*!
* \brief
*	Update function that handles the update loop
*
* \param
*	[float] dt - delta time
*/
static void Loop(float dt) {
	Registry& reg = *CEO::Instance().GetManager<Registry>();// Reference to registry
	ComponentRegistry& compReg = *CEO::Instance().GetManager<ComponentRegistry>();
	GLApp::update_time(1.0);
	CEO::Instance().GetManager<ResourceManager>()->Update();

#ifdef EditorFlag

	//CEO::Instance().GetManager<Savestate>()->TickControl(accumulator);


	if (Editor::Instance().GetState() != Editor::EditorState::Play)
	{
		GLApp::EditorUpdate(reg);

		Draw();

		GLApp::EditorPostRenderUpdate(reg);
		return;
	}
	/******************************************
	* ONLY UPDATE HERE ONWARDS IF IN PLAY STATE
	******************************************/
#endif

	PROFILE_FUNCTION__("CollisionMap")
	CEO::Instance().GetManager<Pathfind>()->DrawCollisionMap(reg);
	PROFILE_FUNCTION__()

	// call on start for scripts if necessary
	CreateScriptInstance(reg, compReg);

	PROFILE_FUNCTION__("Fixed update")
	if (accumulator >= fixedDeltaTime) {
		int counter = 0;
			do {
				if (counter == 60)
				{
					accumulator = 0;
					break;
				}
				FixedUpdate(fixedDeltaTime);

				PROFILE_FUNCTION__("Glapp Fixed Update")
					GLApp::FixedUpdate(reg, fixedDeltaTime);
				PROFILE_FUNCTION__()
				accumulator -= fixedDeltaTime;
				counter++;
		} while (accumulator > fixedDeltaTime);
	}
	PROFILE_FUNCTION__()

	PROFILE_FUNCTION__("StateMachine")
	auto v = reg.GetEntitiesWithComponent<StateComponent>();
	UpdateStackManager* usm = CEO::Get<UpdateStackManager>();
	for (EntityRegistry::Entity e : v) {
		
		if (usm->ShouldNotUpdate(e)) continue;
		if (!usm->IsActive(e)) continue;

		auto& stateVec = reg.GetComponent<StateComponent>(e)->stateMachines;
		std::for_each(stateVec.begin(), stateVec.end(), [&reg, e, dt](std::shared_ptr<IStateMachine>& sm) {
			sm->Update(reg, e, dt);
			}
		);
	}
	PROFILE_FUNCTION__()

	PROFILE_FUNCTION__("Script Update")
	UpdateScriptInstance(reg, compReg, static_cast<float>(GLApp::delta_time), firstframe);
	PROFILE_FUNCTION__()

	PROFILE_FUNCTION__("GLApp Update")
	GLApp::Update(reg);
	PROFILE_FUNCTION__()

	//PROFILE_FUNCTION__("SavestatesUpdate")
	//CEO::Instance().GetManager<Savestate>()->Update();
	//PROFILE_FUNCTION__()

	Draw();

	GLApp::PostRenderUpdate(reg);
}

/*!
* \brief
*	Update function that handles the update loop
*
* \param
*	[float] dt - delta time
*/
static void FixedUpdate(float fixedDt) {

	Registry& r = *(CEO::Instance().GetManager<Registry>());

	// main loop computes fps and other time related stuff once for all apps ...
	//CEO::Instance().GetManager<ResourceManager>()->Update();

	PROFILE_FUNCTION__("Script Fixed Update")
	FixedUpdateScriptInstance(r , *(CEO::Instance().GetManager<ComponentRegistry>()), fixedDt, firstframe);
	PROFILE_FUNCTION__()

	PROFILE_FUNCTION__("Physics")
	// Only step when not paused OR single-step is triggered
	if (PhysicsSystem::Instance().ShouldStep()) {
		PhysicsSystem::Instance().Update(*(CEO::Instance().GetManager<Registry>()), fixedDt);
		//CollisionSystem::Instance().Update(*(CEO::Instance().GetManager<Registry>()));
	}
	PROFILE_FUNCTION__()
}


/*!
* \brief
*    init function for pre loop code
*
* \param null
*
* \return null
*/
static void init() {

#ifdef EditorFlag

	system("cmake -E copy_directory \"../../../Assets\" \"Assets\"");
#if _DEBUG
	system("cmake -E copy_directory \"Assets\" \"../Debug/Assets\"");

#else
	system("cmake -E copy_directory \"Assets\" \"../Release/Assets\"");


#endif

#endif // 
	CEO::Instance().SetInstance(new CEO);
	// initialize
	CEO::Instance().AddManager<Application>();
	CEO::Instance().GetManager<Application>()->InitCoreManagers();

	rtr::SetSharedMap(&typeMap);
	Input::SetInstance(new InputGLFW());
	CEO::Get<UpdateStackManager>()->UpdateStorages();
#ifdef EditorFlag
	//CompileScript("../../../src/Scripts", *(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());
	LoadScript(*(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());

	//CEO::Instance().GetManager<Savestate>()->InitRegistry(CEO::Instance().GetManager<Registry>());

#else
	LoadScript(*(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()), &typeMap, Input::GetInstance(), CEO::Instance().GetPtr());

#endif

	// start with a 16:9 aspect ratio
	if (!GLApp::init(*(CEO::Instance().GetManager<Registry>()), *(CEO::Instance().GetManager<ComponentRegistry>()))) {
		std::cout << "Unable to create OpenGL context" << std::endl;
		std::exit(EXIT_FAILURE);
	}
	Input::Init();

#ifdef EditorFlag
	Editor::Instance().InitEditor(static_cast<float>(GLApp::width), static_cast<float>(GLApp::height), *(CEO::Instance().GetManager<ComponentRegistry>()));
	//CEO::Instance().GetManager<Savestate>()->AddFunction(&GLApp::Update, *(CEO::Instance().GetManager<Registry>()));
	//CEO::Instance().GetManager<Savestate>()->AddFunction(&GLApp::UpdateTransform, *(CEO::Instance().GetManager<Registry>()));
#else
#endif
}

/*!
* \brief
*    misc functions before the loop
*
* \param null
*
* \return null
*/
static void preloopmisc() {
	// check function definition to see how to use the json serializer/deserializer
#ifdef EditorFlag

	//DEMO::json_demo();
	//DEMO::error_demo();
#endif
	//CEO::Instance().GetManager<ResourceManager>()->GetAudio(GLApp::ost, "BGM", true, true).Play(0.1f);
}

/*!
* \brief
*    misc functions after the loop
*
* \param null
*
* \return null
*/
static void postloopmisc() {

	// profiler printing stuff
#ifdef EditorFlag

	auto& profile = Profiler::Instance().Profile();

	for (auto const& p : profile) {
		std::cout << "In: " << p.first << '\n';
		std::cout << "1 min running average: " << p.second.minuteAverage << "ms" << '\n';
		std::cout << "99% longest time: " << p.second._99p << "ms" << '\n';
		std::cout << "95% longest time: " << p.second._95p << "ms" << '\n' << '\n';
	}
#endif
}

/*!
* \brief
*	free functions after the loop
*
* \param null
*
* \return null
*/
static void free() {

#ifdef  EditorFlag
	Editor::Instance().EditorCleanup();
#endif
	GLApp::cleanup();
}
#endif

