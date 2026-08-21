/* !
@file    	glapp.h
@author  	pghali@digipen.edu
@co-author  parminder.singh@digipen.edu
@co-author  Ng Wei Jie (weijie.ng) (90%)
@co-author  Tan Jun Jie (t.junjie) (10%)
@date    	05/05/2025

This file contains the declaration of class GLApp that encapsulates the
functionality required to create an OpenGL context using GLFW; use GLEW
to load OpenGL extensions; initialize OpenGL state; and finally initialize
the OpenGL application by calling initalization functions associated with
objects participating in the application.

It also provides:
- [InputGLFW] for Windows platforms using GLFW (keyboard and mouse input).
- [InputAndroid] for Android platforms using touch pointers.

Each implementation overrides the virtual input methods in [Input] to query and
update platform-specific input states.



Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/

/*                                                                      guard
----------------------------------------------------------------------------- */
#ifndef GLAPP_H
#define GLAPP_H

/*                                                                   includes
----------------------------------------------------------------------------- */
#ifdef PLATFORM_WINDOWS
#include <GL/glew.h> // for access to OpenGL API declarations

#include <GLFW/glfw3.h>
#else
#include <GLES3/gl3.h> 
#endif
#include <string>
#include "InputManager.h"
#include "unordered_map"
#include "glslshader.h"
#include "chrono"
#include "Registry.h"

#include <CollisionSystem.h>
#include "Animation.h"
#include "FrameBuffer.h"
/*  _________________________________________________________________________ */
struct GLApp {
	/*!
	* \brief
	*	
	*
	* \param[in,out] registry
	*	- 
	* \param[in,out] compRegistry
	*	-
	*/
	static bool init(Registry& registry, ComponentRegistry& compRegistry);
	/*!
	* \brief
	*	Writes appropriate information to window title bar and updates
	*   the respective system.
	* 
	* \param[in,out] registry
	*	- The ECS registry
	*/
	static void Update(Registry& registry);
	static void EditorUpdate(Registry& registry);
	static void FixedUpdate(Registry& registry, float fixedDt);
	static void PostRenderUpdate(Registry& registry);
    static void EditorPostRenderUpdate(Registry& registry);
	static void draw(Registry& registry);
	/*!
	* \brief
	*	Return all allocated resources.
	*/
	static void cleanup();

	// callbacks ...
	static void error_cb(int error, char const* description);
#ifdef PLATFORM_WINDOWS
    static void changeWindowMode();
	static void fbsize_cb(GLFWwindow* ptr_win, int width, int height);
	// I/O callbacks ...
	static void key_cb(GLFWwindow* pwin, int key, int scancode, int action, int mod);
	static void mousebutton_cb(GLFWwindow* pwin, int button, int action, int mod);
	static void mousescroll_cb(GLFWwindow* pwin, double xoffset, double yoffset);
	static void mousepos_cb(GLFWwindow* pwin, double xpos, double ypos);
    static void focus_cb(GLFWwindow* window, int focused);

    
    static void InitCursors(json data);
    //static void SetCustomCursorTex(TextureObj const& tex, int xHot, int yHot);
    static void SetCustomCursor();
#endif
	/*!
	* \brief
	*	This function must be first called once during initialization and 
	*	once per game loop. It uses GLFW's time functions to compute:
	*	1. interval in seconds between each frame
	*	2. frames per second every "fps_calc_interval" seconds
	*
	* \param[in,out] fps_calc_interval
	*	- The interval (in seconds) at which fps is to be calculated,
	*	defaulted as 1.0.
	*/
	static void update_time(double fpsCalcInt = 1.0);

    static void UpdateTransform(Registry& registry);
	/*!
	* \brief
	*	A simple test to demonstrate physics-based movement on an entity.
	*	Press WASD to apply forces in the up/down/left/right directions.
	*
	* \param[in,out] registry
	*	- The ECS registry
	*/
	static void TestMovementUpdate(Registry& registry);


	static GLint width, height;                 // the current (running) width and height
    static std::string ost;
    static GLint configWidth, configHeight;     // the set width and height loaded from config
	static GLfloat ar;                          // aspect ratio
	static double fps;
	static double delta_time; // time taken to complete most recent game loop
	static std::string title;
#ifdef PLATFORM_WINDOWS
	static GLFWwindow* ptr_window;
    static bool isFullscreen;
#endif
	static GLboolean keystateW;				// to test wasd
	static GLboolean keystateA;				// to test wasd
	static GLboolean keystateS;				// to test wasd
	static GLboolean keystateD;				// to test wasd
	static bool ImguiFlag;

    static bool recompiling;
    static bool displayFPS;

    static void ChangeFrameBufferSize(int width, int height);
    static void ChangeFocus(bool isFocused);

	//static Animation testAnimation, testAnimation1;
};
#ifdef PLATFORM_WINDOWS

/*!
* \brief
*    Internal struct representing keyboard/mouse button state.
*
* \details
*    Tracks whether a key/button is currently pressed, released,
*    and stores previous frame state for edge detection.
*/
struct KeyBoardState
{
    bool PressedState;   //!< True if pressed this frame
    bool CurrentState;   //!< True if currently held
    bool ReleasedState;  //!< True if released this frame
    bool PreviousState;  //!< State in previous frame
};

/*!
* \brief
*    Windows-specific input implementation using GLFW.
*
* \details
*    Provides keyboard and mouse support via GLFW input APIs.
*    Extends the platform-abstracted [Input] base class.
*/
class InputGLFW : public Input
{
    /*!
    * \brief
    *    Check if a keyboard key is currently held (GLFW).
    *
    * \param
    *    [int] KeyCode - Platform-specific key code.
    *
    * \return
    *    [bool] True if the key is held down or repeated.
    */
    bool IsKeyHeldImpl(int KeyCode) override;

    /*!
    * \brief
    *    Check if a mouse button is currently held (GLFW).
    *
    * \param
    *    [int] ButtonCode - Platform-specific button code.
    *
    * \return
    *    [bool] True if the button is held down or repeated.
    */
    bool IsButtonHeldImpl(int ButtonCode) override;

    /*!
    * \brief
    *    Check if a keyboard key was pressed this frame (GLFW).
    *
    * \param
    *    [int] KeyCode - Platform-specific key code.
    *
    * \return
    *    [bool] True only on the first frame the key is pressed.
    */
    bool IsKeyPressedImpl(int KeyCode) override;

    /*!
    * \brief
    *    Check if a mouse button was pressed this frame (GLFW).
    *
    * \param
    *    [int] ButtonCode - Platform-specific button code.
    *
    * \return
    *    [bool] True only on the first frame the button is pressed.
    */
    bool IsButtonPressedImpl(int ButtonCode) override;

    /*!
    * \brief
    *    Check if a keyboard key was released this frame (GLFW).
    *
    * \param
    *    [int] KeyCode - Platform-specific key code.
    *
    * \return
    *    [bool] True only on the first frame the key is released.
    */
    bool IsKeyReleaseImpl(int KeyCode) override;

    /*!
    * \brief
    *    Check if a mouse button was released this frame (GLFW).
    *
    * \param
    *    [int] ButtonCode - Platform-specific button code.
    *
    * \return
    *    [bool] True only on the first frame the button is released.
    */
    bool IsButtonReleaseImpl(int ButtonCode) override;

    /*!
    * \brief
    *    Get the current mouse X position (GLFW).
    *
    * \return
    *    [float] Mouse X coordinate in screen space.
    */
    float GetMouseXImpl() override;


    float GetMouseGameXImpl() override;

    float GetMouseGameYImpl() override;

    float GetMouseScreenXImpl() override;

    /*!
    * \brief
    *    Get the current mouse Y position (GLFW).
    *
    * \return
    *    [float] Mouse Y coordinate in screen space.
    */
    float GetMouseYImpl() override;

    float GetMouseWorldXImpl(Registry& registry) override;
    float GetMouseWorldYImpl(Registry& registry) override;

    /*!
    * \brief
    *    Update internal input states (GLFW).
    *
    * \details
    *    Refreshes the state maps for keys and buttons and buttons each frame.
    */
    void UpdateImpl() override;
    /*!
* \brief
*   Initializes GLFW-specific input event subscriptions for joystick
*   connection and disconnection notifications. These subscriptions update
*   the stored gamepad connection state when the corresponding events are
*   dispatched by the engine.
*
* \return
*   None.
*/
    void InitImpl() override;


    /*!
    * \brief
    *   Retrieves the stored state of a gamepad at the specified index. If the
    *   requested index is outside the valid range, a dummy empty gamepad state
    *   is returned instead.
    *
    * \param
    *   index - The index of the gamepad to retrieve.
    *
    * \return
    *   [Input::GamepadState const&] A reference to the requested gamepad state,
    *   or a dummy state if the index is invalid.
    */
    GamepadState const& GetGamepadImpl(int index) override;

private:
    std::unordered_map<int, KeyBoardState> KeyState;    //!< Map of key codes to state
    std::unordered_map<int, KeyBoardState> ButtonState; //!< Map of button codes to state
    static constexpr int MaxGamepads = 1;
    std::array<GamepadState, MaxGamepads> m_gamepads{};
};

#else // ======================= ANDROID ===========================

    /*!
    * \brief
    *    Android-specific input implementation.
    *
    * \details
    *    Provides pointer/touch support for mobile devices.
    *    Extends the platform-abstracted [Input] base class.
    */
    class InputAndroid : public Input
    {
        /*!
        * \brief
        *    Check if a pointer is currently held (Android).
        *
        * \param
        *    [int] PointerID - Identifier for the pointer/touch.
        *
        * \return
        *    [bool] True if held, false otherwise.
        */
        bool IsPointerHeldImpl(int PointerID) override;

        /*!
        * \brief
        *    Check if a pointer was pressed this frame (Android).
        *
        * \param
        *    [int] PointerID - Identifier for the pointer/touch.
        *
        * \return
        *    [bool] True only on the first frame it is pressed.
        */
        bool IsPointerPressedImpl(int PointerID) override;

        /*!
        * \brief
        *    Check if a pointer was released this frame (Android).
        *
        * \param
        *    [int] PointerID - Identifier for the pointer/touch.
        *
        * \return
        *    [bool] True only on the first frame it is released.
        */
        bool IsPointerReleaseImpl(int PointerID) override;

        /*!
        * \brief
        *    Get the X position of a pointer (Android).
        *
        * \param
        *    [int] id - Identifier for the pointer/touch.
        *
        * \return
        *    [float] Pointer X coordinate relative to screen center.
        */
        float GetPointerXImpl(int id) override;

        float GetPointerScreenXImpl(int id) override;

        /*!
        * \brief
        *    Get the Y position of a pointer (Android).
        *
        * \param
        *    [int] id - Identifier for the pointer/touch.
        *
        * \return
        *    [float] Pointer Y coordinate relative to screen center (inverted).
        */
        float GetPointerYImpl(int id) override;

        /*!
        * \brief
        *    Update the state of a pointer (Android).
        *
        * \param
        *    [int] pointerId - Identifier for the pointer.
        * \param
        *    [int] action - Platform-specific touch action code.
        * \param
        *    [float] x - Pointer X position.
        * \param
        *    [float] y - Pointer Y position.
        */
        void UpdateTouchImpl(int pointerId, int action, float x, float y) override;
        void UpdateTouchImplStart() override;
        void UpdateTouchImplEnd() override;

        /*!
        * \brief
        *    Get the state of all active pointers (Android).
        *
        * \return
        *    [std::map<int, Input::Action>] Map of pointer IDs to their Action states.
        */
        std::map<int, Input::Action> GetAllPointerImpl() override;

    private:
        std::map<int, Input::Action> State; //!< Map of active pointers to their state
        std::map<int, Input::Action> SavedState; //!< Map of active pointers to their state
    };

#endif // PLATFORM_WINDOWS
#endif /* GLAPP_H */
