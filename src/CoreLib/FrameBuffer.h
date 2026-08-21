/*!
@file       FrameBuffer.h
@author     Ou Yukang (yukang.ou) 100%
@date       06/10/2025
@brief		Handler for a frame buffer and it's dependencies, used for off-
			screen rendering and post process effects


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#pragma once
#include "Platform.h"

class FrameBuffer {
public:
	enum AttachmentFlag :char {
		ATTACH_COLOR = 1 << 0,
		ATTACH_DEPTH = 1 << 1,
		ATTACH_STENCIL = 1 << 2,
		ATTACH_ENTITY_ID = 1 << 3,
		ATTACH_ADDITIONAL_COLOR = 1 << 4,
		ATTACH_LIGHT = 1 << 5,
	};

	FrameBuffer() = default;
	~FrameBuffer() = default;

	/*!
	* \brief
	*	initialize frame buffer with size and attachment bitflag of type (AttachmentFlag)
	* \param
	*	_width - width of fbo
	* \param
	*	_height - height of fbo
	* \param
	*	_attachmentFlags - bitmask flag for the attachments 
	*/
	bool Init(GLuint _width, GLuint _height, char _attachmentFlags = ATTACH_COLOR, bool hasAlphaChannel = false);

	/*!
	* \brief
	*	bind frame buffer to the opengl context
	*/
	void Bind();
	/*!
	* \brief
	*	draw color attachment into a quad
	*/
	void Draw(GLSLShader& shader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame"));
	/*!
	* \brief
	*	bind the default frame buffer to the opengl context
	*/
	void Unbind();

	/*!
	* \brief
	*	resize framebuffer and it's attachments
	* \param
	*	_width - new width of fbo
	* \param
	*	_height - new height of fbo
	*/
	void Resize(GLuint _width, GLuint _height);

	/*!
	* \brief
	*	free frame buffer resources
	*/
	void Free();

	/*!
	* \brief
	*	getter for the color attachment of the frame buffer
	*	can be used as a normal texture
	* \return
	*	name of texture
	*/
	GLuint ColorBufferTex() const;

	/*!
	* \brief
	*	getter for the secondary color attachment of the frame buffer
	*	can be used as a normal texture
	* \return
	*	name of texture
	*/
	GLuint SecondaryColorBufferTex() const;

	/*!
	* \brief
	*	getter for the light attachment of the frame buffer
	*	can be used as a normal texture
	* \return
	*	name of texture
	*/
	GLuint LightBufferTex() const;

	inline GLint Width() const noexcept { return width; }
	inline GLint Height() const noexcept { return height; }
private:
	/*!
	* \brief
	*	adds the specified fbo attachment type to the fbo
	* \param
	*	_width - new width of fbo
	* \param
	*	_height - new height of fbo
	*/
	void CreateAttachment(AttachmentFlag attachment, GLuint _width, GLuint _height);
	/*!
	* \brief
	*	removes the specified fbo attachment type to the fbo
	* \param
	*	attachment - attachment type to create
	* \param
	*	_width - new width of fbo
	* \param
	*	_height - new height of fbo
	*/
	void FreeAttachment(AttachmentFlag attachment);

	bool isInit = false;
	bool hasAlphaChannel = false;
	char attachFlags = 0;
	GLuint frameBuffer = 0, colorBufferTex = 0, depthBuffer = 0,stencilBuffer = 0,entityIDBuffer = 0, secondaryColorBufferTex = 0, lightIntensityTex = 0;
	GLint width{};
	GLint height{};
};