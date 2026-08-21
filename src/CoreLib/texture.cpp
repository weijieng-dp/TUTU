/*!
@file       Texture.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Interface for texture object. Loading and unloading is handled
			by the ResourceManager.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "Texture.h"

TextureObj::TextureObj(TextureObj&& other) noexcept : 
	texId(std::exchange(other.texId, 0)), wrapMode(other.wrapMode), filterMode(other.filterMode),
	width(other.width), height(other.height), isLoaded(other.isLoaded), useBlend(other.useBlend), pixelData(std::move(other.pixelData)), channels(other.channels)
{
	other.channels = 0;
}

TextureObj& TextureObj::operator=(TextureObj&& other) noexcept {
	texId = std::exchange(other.texId, 0);
	wrapMode = other.wrapMode;
	filterMode = other.filterMode;
	width = other.width;
	height = other.height;
	isLoaded = other.isLoaded;
	pixelData = std::move(other.pixelData);
	channels = other.channels; 
	other.channels = 0;

	return *this;
}

void TextureObj::SetWrapMode(const WrapMode& wm) {
	SetWrapModeS(wm.s);
	SetWrapModeT(wm.t);
}
void TextureObj::SetWrapModeS(GLint s) { if (IsValidWrapMode(s)) wrapMode.s = s; }
void TextureObj::SetWrapModeT(GLint t) { if (IsValidWrapMode(t)) wrapMode.t = t; }

bool TextureObj::IsValidWrapMode(GLint wm) {
	return (wm == GL_REPEAT || wm == GL_MIRRORED_REPEAT || wm == GL_CLAMP_TO_EDGE
#ifdef  PLATFORM_WINDOWS
		|| wm == GL_CLAMP_TO_BORDER
#endif
		);
}
void TextureObj::SetFilterMode(const FilterMode& fm) {
	SetMinFilterMode(fm.min);
	SetMagFilterMode(fm.mag);
}
void TextureObj::SetMinFilterMode(GLint fm) { if (IsValidMinFilterMode(fm)) filterMode.min = fm; }
void TextureObj::SetMagFilterMode(GLint fm) { if (IsValidMagFilterMode(fm)) filterMode.mag = fm; }
void TextureObj::SetBlend(GLboolean blend) { useBlend = blend; }

bool TextureObj::IsValidMinFilterMode(GLint fm) {
	return (fm == GL_NEAREST || fm == GL_LINEAR || fm == GL_NEAREST_MIPMAP_NEAREST
		|| fm == GL_LINEAR_MIPMAP_NEAREST || fm == GL_NEAREST_MIPMAP_LINEAR
		|| fm == GL_LINEAR_MIPMAP_LINEAR);
}
bool TextureObj::IsValidMagFilterMode(GLint fm) { return (fm == GL_NEAREST || fm == GL_LINEAR); }

bool TextureObj::ApplyBlend(float alpha) const {
	if (useBlend || alpha < 1.f) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		return true;
	}
	return false;
}


void TextureObj::ApplyTextureParam(bool texBound) const {
	if (!texBound) this->BindTexture();
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode.s);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode.t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode.min);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode.mag);
	if (!texBound) UnbindTexture();
}