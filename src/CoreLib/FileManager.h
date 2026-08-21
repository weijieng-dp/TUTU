#pragma once
/**___________________________________________________________________________/
@file          FileManager.h
@author        j.junbo@digipen.edu
@date          9/29/2025

This holds a simple wrapper for the fstream objects to handle android compatability.
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include <fstream>
#include <sstream>
#include "Platform.h"

#ifdef PLATFORM_ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#endif

/*!
* \brief
*   Simple wrapper for fstream objects for android compatability.
* \brief
*	Since android gives char buffers, input and output through filemanager
*	uses stringstreams
*/
class FileManager {
private:
	std::string AndroidWriteFilepath{ "/data/data/com.example.jnicpp/files/" };	// com.example.jnicpp may have to change if we change android project file structure
	std::string PCWriteFilepath{};
public:
	FileManager() = default;



	/*!
	* \brief
	*    Android only function. Used to initialize the android assetmanager
	*
	* \param 
	*	[void*] the java asset manager
	*
	* \return null
	*/
	void Initialize(void* javaAssetManager);

	~FileManager();


	FileManager(const FileManager&) = delete;
	FileManager& operator=(const FileManager&) = delete;

	/*!
	* \brief
	*    Replacement for an ifstream object. Can be used in the exact same way
	*
	* \param
	*	[std::string&] the filepath
	* \param
	*	[std::ios_base::openmode = std::ios_base::in] Flags for the fstream initialization. 
	*
	* \return 
	*	[std::stringstream] A stringstream containing a char array that is the file data. 
	*	The stringstream will be initialized with the same flags as the fstream.
	*/
	std::stringstream ReadFile(const std::string& filename, bool fromWritePath = false, std::ios_base::openmode mode = std::ios_base::in);
	
	/*!
	* \brief
	*    Replacement for an ofstream object. Can be used in the exact same way
	*
	* \param
	*	[std::string&] the filepath
	* \param
	*	[std::stringstream&] A stringstream containing the data that is meant to be written into the file
	* \param
	*	[std::ios_base::openmode = std::ios_base::out] Flags for the fstream initialization.
	*
	* \return null
	*/
	void WriteFile(const std::string& filename, const std::stringstream &sstr, std::ios_base::openmode mode = std::ios_base::out);

#ifdef EditorFlag
	// Same as Write file but does not append the write filepath.
	void EditorWriteFile(const std::string& filename, const std::stringstream& sstr, std::ios_base::openmode mode = std::ios_base::out);
#endif

private:
#ifdef PLATFORM_ANDROID
	AAssetManager* assetManager;
	bool failLoad;
#endif

};
