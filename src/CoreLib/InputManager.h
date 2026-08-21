/**___________________________________________________________________________/
@file          InputManager.h
@author        Ng Wei Jie (weijie.ng) (100%)
@date          9/29/2025

This file defines the platform-abstracted [Input] system for handling user input
across Windows and non-Windows platforms (e.g., mobile/touch).

It provides:
- Static global accessors for key, button, and pointer states.
- Platform-specific input querying (keyboard/mouse on Windows, touch pointers on mobile).
- A `Controller` class for mobile joystick-style touch input, including drawing
  and updating virtual joystick controls.

  Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include <map>
#include "Platform.h"
#include "RenderUtils.h"
#include "glslshader.h"


// =====================================================
// =                     Input                        =
// =====================================================
/*!
* \brief
*    Abstract base class for handling platform-specific input states.
*
* \details
*    Provides static global functions for querying keyboard, mouse,
*    or touch input depending on platform. Derived classes must implement
*    platform-specific behavior.
*/
class  Input
{

public:
	struct GamepadState
	{
		static constexpr int ButtonCount = 15;
		static constexpr int AxisCount = 6;

		bool connected = false;
		bool buttons[ButtonCount]{};
		bool previousButtons[ButtonCount]{};
		float axes[AxisCount]{};
	};

	enum class GamepadButton
	{
		A = 0,
		B,
		X,
		Y,
		LB,
		RB,
		BACK,
		START,
		GUIDE,
		LEFT_THUMB,
		RIGHT_THUMB,
		DPAD_UP,
		DPAD_RIGHT,
		DPAD_DOWN,
		DPAD_LEFT,
		COUNT
	};

	enum class GamepadAxis
	{
		LEFT_X = 0,
		LEFT_Y,
		RIGHT_X,
		RIGHT_Y,
		LEFT_TRIGGER,
		RIGHT_TRIGGER,
		COUNT
	};
#ifndef PLATFORM_WINDOWS
private:
    class Controller
    {
    public:
        /*!
        * \brief
        *    Construct a controller with a default joystick radius.
        */
        Controller() : radius(200) , ID(0) {}


		void InitJoystick(std::string const&,std::string const& );
        /*!
        * \brief
        *    Draw the on-screen joystick.
        *
        * \param
        *    [GLSLShader&] shader - Shader used for rendering.
        * \param
        *    [Vec2 const&] GivenScreenSize - Screen dimensions for positioning.
        */
        void DrawJoystick(GLSLShader& shader, Vec2 const& GivenScreenSize);

        /*!
        * \brief
        *    Update the joystick state based on active touch input.
        */
        void UpdateJoystick();

        /*!
        * \brief
        *    Get the normalized joystick vector representing direction and magnitude.
        *
        * \return
        *    [Vec2] Joystick input value.
        */
        Vec2 GetJoystickValue();

        /*!
        * \brief
        *    Setter for screen size
        */
        void ScreenSize(Vec2 newSize);

        int ID;
		bool active = false;

    private:
        Vec2 currentPos;   //!< Current pointer position
        Vec2 originalPos;  //!< Initial joystick origin
        Vec2 screenSize;   //!< Screen size for scaling
        int index;         //!< Active pointer index
        float radius;      //!< Joystick active radius
        TextureObj* joystickTexture;
        TextureObj* joystickBackgroundTexture;

    };

public:
    std::array<Controller,2> Controllers;
#else
public:
#endif


	// -------- Gamepad API (platform-agnostic) --------
#ifdef PLATFORM_WINDOWS
/*!
* \brief
*   Retrieves the current state of the gamepad at the specified index by
*   forwarding the request to the platform-specific implementation.
*
* \param
*   index - The index of the gamepad to retrieve. Defaults to 0.
*
* \return
*   [GamepadState const&] A constant reference to the requested gamepad state.
*/
static GamepadState const& GetGamepad(int index = 0)
	{
		return Instance->GetGamepadImpl(index);
	}

/*!
* \brief
*   Checks whether the gamepad at the specified index is currently connected.
*
* \param
*   index - The index of the gamepad to check. Defaults to 0.
*
* \return
*   [bool] True if the gamepad is connected; otherwise false.
*/
	static bool IsGamepadConnected(int index = 0)
	{
		return GetGamepad(index).connected;
	}

	/*!
* \brief
*   Checks whether a specific gamepad button is currently being held down on
*   the gamepad at the specified index.
*
* \param
*   button - The gamepad button to query.
* \param
*   index - The index of the gamepad to check. Defaults to 0.
*
* \return
*   [bool] True if the button is currently held and the gamepad is connected;
*   otherwise false.
*/
	static bool IsGamepadButtonHeld(GamepadButton button, int index = 0)
	{
		auto const& pad = GetGamepad(index);
		return pad.connected && pad.buttons[static_cast<int>(button)];
	}

	/*!
* \brief
*   Checks whether a specific gamepad button was pressed during the current
*   frame. A button is considered pressed if it is currently down but was not
*   down in the previous frame.
*
* \param
*   button - The gamepad button to query.
* \param
*   index - The index of the gamepad to check. Defaults to 0.
*
* \return
*   [bool] True if the button was pressed this frame and the gamepad is
*   connected; otherwise false.
*/
	static bool IsGamepadButtonPressed(GamepadButton button, int index = 0)
	{
		auto const& pad = GetGamepad(index);
		int idx = static_cast<int>(button);
		return pad.connected && pad.buttons[idx] && !pad.previousButtons[idx];
	}

	/*!
* \brief
*   Checks whether a specific gamepad button was released during the current
*   frame. A button is considered released if it is currently up but was down
*   in the previous frame.
*
* \param
*   button - The gamepad button to query.
* \param
*   index - The index of the gamepad to check. Defaults to 0.
*
* \return
*   [bool] True if the button was released this frame and the gamepad is
*   connected; otherwise false.
*/
	static bool IsGamepadButtonReleased(GamepadButton button, int index = 0)
	{
		auto const& pad = GetGamepad(index);
		int idx = static_cast<int>(button);
		return pad.connected && !pad.buttons[idx] && pad.previousButtons[idx];
	}

	/*!
* \brief
*   Retrieves the current value of a specified gamepad axis from the gamepad
*   at the given index. If the gamepad is not connected, the function returns
*   0.0f.
*
* \param
*   axis - The gamepad axis to query.
* \param
*   index - The index of the gamepad to check. Defaults to 0.
*
* \return
*   [float] The current axis value if the gamepad is connected; otherwise 0.0f.
*/
	static float GetGamepadAxis(GamepadAxis axis, int index = 0)
	{
		auto const& pad = GetGamepad(index);
		return pad.connected ? pad.axes[static_cast<int>(axis)] : 0.0f;
	}

	/*!
* \brief
*   Retrieves the current left stick input for the specified gamepad and
*   applies deadzone processing to the X and Y axis values before returning
*   the final stick vector.
*
* \param
*   index - The index of the gamepad to query. Defaults to 0.
*
* \return
*   [Vec2] The processed left stick input vector after deadzone adjustment.
*/
	static Vec2 GetLeftStick(int index = 0)
	{
		return Instance->ApplyStickDeadzone(
			GetGamepadAxis(GamepadAxis::LEFT_X, index),
			GetGamepadAxis(GamepadAxis::LEFT_Y, index)
		);
	}

	/*!
	* \brief
	*   Retrieves the current right stick input for the specified gamepad and
	*   applies deadzone processing to the X and Y axis values before returning
	*   the final stick vector.
	*
	* \param
	*   index - The index of the gamepad to query. Defaults to 0.
	*
	* \return
	*   [Vec2] The processed right stick input vector after deadzone adjustment.
	*/
	static Vec2 GetRightStick(int index = 0)
	{
		return Instance->ApplyStickDeadzone(
			GetGamepadAxis(GamepadAxis::RIGHT_X, index),
			GetGamepadAxis(GamepadAxis::RIGHT_Y, index)
		);
	}
#endif
	/*!
	 * \brief
	 *    Set the global Input instance.
	 *
	 * \param
	 *    [Input*] input - Pointer to the new Input instance.
	 *
	 * \details
	 *    If an existing instance already exists, it will be deleted and replaced.
	 */
	static void SetInstance(Input* input) {
		if (Instance) delete Instance;
		Instance = input;
	}

	static Input* GetInstance()
	{
		return Instance;
	}
	/*!
	* \brief
	*    Shut down the Input system and clean up the instance.
	*/
	static void Shutdown() {
		delete Instance;
		Instance = nullptr;
	}
#ifdef PLATFORM_WINDOWS
	// -------- WINDOWS: Keyboard & Mouse --------

	/*!
	* \brief
	*    Check if a keyboard key is currently held down.
	*
	* \param
	*    [int] KeyCode - Platform-specific key code.
	*
	* \return
	*    [bool] True if the key is held, false otherwise.
	*/
	static bool IsKeyHeld(int KeyCode) { return Instance->IsKeyHeldImpl(KeyCode); }

	/*!
	* \brief
	*    Check if a mouse button is currently held down.
	*
	* \param
	*    [int] ButtonCode - Platform-specific button code.
	*
	* \return
	*    [bool] True if the button is held, false otherwise.
	*/

	static bool IsButtonHeld(int ButtonCode) { return Instance->IsButtonHeldImpl(ButtonCode); }


	/*!
	* \brief
	*    Check if a keyboard key was pressed this frame.
	*
	* \param
	*    [int] KeyCode - Platform-specific key code.
	*
	* \return
	*    [bool] True if the key was pressed, false otherwise.
	*/
	static bool IsKeyPressed(int KeyCode) { return Instance->IsKeyPressedImpl(KeyCode); }



	/*!
	* \brief
	*    Check if a mouse button was pressed this frame.
	*
	* \param
	*    [int] ButtonCode - Platform-specific button code.
	*
	* \return
	*    [bool] True if the button was pressed, false otherwise.
	*/
	static bool IsButtonPressed(int ButtonCode) { return Instance->IsButtonPressedImpl(ButtonCode);}


	/*!
	* \brief
	*    Check if a keyboard key was released this frame.
	*
	* \param
	*    [int] KeyCode - Platform-specific key code.
	*
	* \return
	*    [bool] True if the key was released, false otherwise.
	*/
	static bool IsKeyRelease(int KeyCode) { return Instance->IsKeyReleaseImpl(KeyCode); }

	/*!
	* \brief
	*    Check if a mouse button was released this frame.
	*
	* \param
	*    [int] ButtonCode - Platform-specific button code.
	*
	* \return
	*    [bool] True if the button was released, false otherwise.
	*/
	static bool IsButtonRelease(int ButtonCode) { return Instance->IsButtonReleaseImpl(ButtonCode); }

	/*!
	* \brief
	*    Get the current X position of the mouse.
	*
	* \return
	*    [float] Mouse X coordinate.
	*/
    static float GetX() {
		return Instance->GetMouseXImpl();
	}

	static float GetGameX() {
		return Instance->GetMouseGameXImpl();
	}

	static float GetScreenX() {
		return Instance->GetMouseScreenXImpl();
	}

	static float GetWorldX(Registry& registry) {
		return Instance->GetMouseWorldXImpl(registry);
	}
	/*!
	* \brief
	*    Get the current Y position of the mouse.
	*
	* \return
	*    [float] Mouse Y coordinate.
	*/
	static float GetY() {
		return Instance->GetMouseYImpl();
	}

	static float GetGameY() {
		return Instance->GetMouseGameYImpl();
	}

	static float GetWorldY(Registry &registry) {
		return Instance->GetMouseWorldYImpl(registry);
	}

	static void Init()
	{
		return Instance->InitImpl();
	}


	/*!
	* \brief
	*    Update the input state (per-frame).
	*/
	static void Update()
	{
		return Instance->UpdateImpl();
	}
#else

// -------- MOBILE/TOUCH: Pointer Input --------

/*!
* \brief
*    Structure representing the state of a touch action.
*/

	struct Action {
		float x, y;
		bool pressed = false;
		bool held = false;
		bool released = false;
	};

	/*!
	* \brief
	*    Check if a pointer was released this frame.
	*
	* \param
	*    [int] PointerID - Identifier for the pointer/touch.
	*
	* \return
	*    [bool] True if released, false otherwise.
	*/
	static bool IsPointerRelease(int PointerID) { return Instance->IsPointerReleaseImpl(PointerID); }

	/*!
	* \brief
	*    Check if a pointer was pressed this frame.
	*
	* \param
	*    [int] PointerID - Identifier for the pointer/touch.
	*
	* \return
	*    [bool] True if pressed, false otherwise.
	*/
	static bool IsPointerPressed(int PointerID) { return Instance->IsPointerPressedImpl(PointerID); }

	/*!
	* \brief
	*    Check if a pointer is currently being held.
	*
	* \param
	*    [int] PointerID - Identifier for the pointer/touch.
	*
	* \return
	*    [bool] True if held, false otherwise.
	*/
	static bool IsPointerHeld(int PointerID) { return Instance->IsPointerHeldImpl(PointerID); };


	/*!
	* \brief
	*    Update the touch state of a pointer.
	*
	* \param
	*    [int] pointerId - Identifier for the pointer.
	* \param
	*    [int] action - Platform-specific action (press, release, move).
	* \param
	*    [float] x - X coordinate of the pointer.
	* \param
	*    [float] y - Y coordinate of the pointer.
	*/
	static void UpdateTouch(int pointerId, int action, float x, float y) { return Instance->UpdateTouchImpl(pointerId, action, x, y); }


	/*!
* \brief
*    Update the touch state of a pointer.
*
* \param
*    [int] pointerId - Identifier for the pointer.
* \param
*    [int] action - Platform-specific action (press, release, move).
* \param
*    [float] x - X coordinate of the pointer.
* \param
*    [float] y - Y coordinate of the pointer.
*/
	static void UpdateTouchEnd() { return Instance->UpdateTouchImplEnd(); }

	/*!
* \brief
*    Update the touch state of a pointer.
*
* \param
*    [int] pointerId - Identifier for the pointer.
* \param
*    [int] action - Platform-specific action (press, release, move).
* \param
*    [float] x - X coordinate of the pointer.
* \param
*    [float] y - Y coordinate of the pointer.
*/
	static void UpdateTouchStart() { return Instance->UpdateTouchImplStart(); }

	/*!
	* \brief
	*    Get the state of all active pointers.
	*
	* \return
	*    [std::map<int, Action>] Map of pointer IDs to their Action states.
	*/
	static std::map<int, Action> GetAllPointer() { return Instance->GetAllPointerImpl(); }


	/*!
	* \brief
	*    Get the X coordinate of a pointer.
	*
	* \param
	*    [int] id - Identifier for the pointer/touch.
	*
	* \return
	*    [float] X coordinate.
	*/
	static float GetX(int id) {
		return Instance->GetPointerXImpl(id);
	}

	static float GetScreenX(int id) {
		return Instance->GetPointerScreenXImpl(id);
	}

	/*!
	* \brief
	*    Get the Y coordinate of a pointer.
	*
	* \param
	*    [int] id - Identifier for the pointer/touch.
	*
	* \return
	*    [float] Y coordinate.
	*/
	static float GetY(int id) {

		return Instance->GetPointerYImpl(id);
	}

    static std::array<Controller,2>& GetControllers()
    {
        return Instance->Controllers;
    }

#endif // PLATFORM_WINDOWS

protected:

#ifdef PLATFORM_WINDOWS
	// ---- Windows Virtuals ----
	virtual bool IsKeyHeldImpl(int KeyCode) = 0;
	virtual bool IsButtonHeldImpl(int ButtonCode) = 0;
	virtual bool IsKeyPressedImpl(int KeyCode) = 0;
	virtual bool IsButtonPressedImpl(int ButtonCode) = 0;
	virtual bool IsKeyReleaseImpl(int KeyCode) = 0;
	virtual bool IsButtonReleaseImpl(int ButtonCode) = 0;
	virtual float GetMouseXImpl() = 0;
	virtual float GetMouseYImpl() = 0;
	virtual float GetMouseGameXImpl() = 0;
	virtual float GetMouseScreenXImpl() = 0;
	virtual float GetMouseGameYImpl() = 0;
	virtual float GetMouseWorldXImpl(Registry& registry) = 0;
	virtual float GetMouseWorldYImpl(Registry& registry) = 0;
	virtual void InitImpl() = 0;
	virtual void UpdateImpl() = 0;
#else
	// ---- Mobile Virtuals ----
	virtual float GetPointerXImpl(int id) = 0;
	virtual float GetPointerScreenXImpl(int id) = 0;
	virtual float GetPointerYImpl(int id) = 0;
	virtual void UpdateTouchImpl(int pointerId, int action, float x, float y) = 0;
	virtual void UpdateTouchImplEnd() = 0;
	virtual void UpdateTouchImplStart() = 0;
	virtual bool IsPointerReleaseImpl(int PointerID) = 0;
	virtual bool IsPointerPressedImpl(int PointerID) = 0;
	virtual bool IsPointerHeldImpl(int PointerID) = 0;
	virtual std::map<int, Action> GetAllPointerImpl() = 0;




    /*!
* \brief
*    Virtual joystick controller for touch-based input.
*
* \details
*    Provides on-screen joystick rendering and position tracking for mobile devices.
*/


#endif

#ifdef PLATFORM_WINDOWS
	virtual GamepadState const& GetGamepadImpl(int index) = 0;

	Vec2 ApplyStickDeadzone(float x, float y) const;
#endif
	virtual ~Input() = default;

	private:
		
		static Input* Instance; //!< Singleton instance pointer
};

#ifndef PLATFORM_WINDOWS
// =====================================================
// =                   Controller                    =
// =====================================================

#endif