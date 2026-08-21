#pragma once
/**___________________________________________________________________________/
@file          font.h
@author        j.junbo@digipen.edu
@co-author     yukang.ou@digipen.edu (50%)
@date          31/03/2024

This header contains a basic struct to contain font data. Yukang made the
original character struct, Junbo made the wrapper and the map.
/*____________________________________________________________________________*/
#include <map>

/*!
* \brief
*	A simple struct to store the data of a font texture
*/
struct FontObj {
	// Container for a char texture
	// taken from https://learnopengl.com/In-Practice/Text-Rendering
	struct Character{
		unsigned int texID{};			// ID handle of the glyph texture
		int   sizeX{}, sizeY{};			// Size of glyph
		int   bearingX{}, bearingY{};   // Offset from baseline to left/top of glyph
		int advance{};					// Offset to advance to next glyph
	};
	std::string path{};							// filepath
	std::map<unsigned char, Character> map{};	// Map of all the textures for a char
	unsigned int resolution{};
	bool loaded{ false };
};