/**___________________________________________________________________________/
@file          FontManager.h
@author        j.junbo@digipen.edu
@date          9/29/2025

Simple private class to handle the loading and freeing of font assets
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#pragma once
#include <string>
#include "font.h"

class FontManager {
	friend class ResourceManager;

public:

	FontManager() = default;
	~FontManager() = default;
private:

	FontManager(const FontManager&) = delete;
	FontManager& operator=(const FontManager&) = delete;

	/*!
	* \brief
	*   Initializes libraries needed to load in fonts
	*/
	void Init();
	/*!
	* \brief
	*   Frees the libraries needed to load in fonts
	*/
	void Free();

	/*!
	* \brief
	*   Loads a font asset based on the filename
	*
	* \param [const std::string&] filename
	* \param [unsigned int] The resolution of the font to be initially loaded in as
	*
	* \return [FontObj] The font object
	*/
	FontObj LoadFont(const std::string& name, unsigned int resolution);

	/*!
	* \brief
	*   Frees the font object from memory
	*
	* \param [FontObj&] the font obj to free
	*/
	void FreeFont(FontObj& obj);

};