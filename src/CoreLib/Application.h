/*!
@file       Application.h
@author  	yukang.ou@digipen.edu(yukang)
@date       07/10/2025
@brief      Handles the life cycle of the application

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
______________________________________________________________________*/

#pragma once

class Application {
public:
	/*!
	* \brief
	*   Private default constructor to enforce singleton pattern.
	*/
	Application() = default;
	
	/*!
	* \brief
	*   Private destructor to enforce singleton pattern.
	*/
	~Application() = default;

	/*!
	* \brief
	*   Initializes the engine's Core Managers
	*/
	void InitCoreManagers();

	/*!
	* \brief
	*   Signal app to exit
	*/
	static void SignalExit();

	/*!
	* \brief
	*   Check if app has signalled to exit
	*/
	static bool IsExitSignalled();

	/*!
	* \brief
	*   Update loop after draw calls
	*/
	void PostRenderUpdate();
private:
	/*!
	* \brief
	*   Deleted copy constructor to prevent copying of singleton instance.
	*/
	Application(const Application&) = delete;
	
	/*!
	* \brief
	*   Deleted assignment operator to prevent copying of singleton instance.
	*/
	Application& operator=(const Application&) = delete;

	/*!
	* \brief
	*   Cleans up resources and shuts down the Application.
	*/
	void Free();

	bool exitSignal = false;
};