/**___________________________________________________________________________/
@file          TextureManager.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple private class to handle the loading and unloading of openGL Textures
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include <string>
#include "texture.h"

class TextureManager {
	friend class ResourceManager;
	friend class VideoManager;
public:

	TextureManager() = default;
	~TextureManager() = default;

private:
	TextureManager(const TextureManager&) = delete;
	TextureManager& operator=(const TextureManager&) = delete;

	

	/*!
	* \brief
	*	Initializes the libraries required to load in texture assets
	*/
	void Init();
	/*!
	* \brief
	*	Frees the libraries required to load in texture assets
	*/
	void Free();
	
	/*!
	* \brief
	*   Loads a texture asset based on the filename
	*
	* \param [const std::string&] filename
	*
	* \return [TextureObj] The texture object
	*/
	TextureObj LoadTex(const std::string& name);

	/*!
	* \brief
	*   Loads a texture asset based on the filename
	*
	* \param [const std::string&] filename
	*
	* \return [TextureObj] The texture object
	*/
	TextureObj LoadTexFromMemory(const unsigned char* data, int size);

	/*!
	* \brief
	*   Frees the texture object from memory
	*
	* \param [TextureObj&] the texture obj to free
	*/
	void FreeTex(TextureObj& obj);

};