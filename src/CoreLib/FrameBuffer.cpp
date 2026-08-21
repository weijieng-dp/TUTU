/*!
@file       FrameBuffer.cpp
@author     Ou Yukang (yukang.ou) 100%
@date       06/10/2025
@brief		Handler for a frame buffer and it's dependencies, used for off-
			screen rendering and post process effects


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/
#include "pch.h"
#include "FrameBuffer.h"

bool FrameBuffer::Init(GLuint _width, GLuint _height, char _attachmentFlags, bool _hasAlphaChannel)
{
	if (isInit)
		return false;

	if (_attachmentFlags == 0) // no attachments specified
	{
		LOGE("Failed to initialize frame buffer: no attachments specified");
		return false;
	}
	if (_width == 0)
		_width = 100;
	if(_height == 0)
		_height = 100;

	hasAlphaChannel = _hasAlphaChannel;

	// create and activate the frame buffer
	glGenFramebuffers(1, &frameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	// add the attachments specified based on the bitmask attachFlags
	attachFlags = _attachmentFlags;

	if ((attachFlags & ATTACH_ENTITY_ID) && attachFlags & ATTACH_LIGHT)
	{
		LOGE("Failed to initialize frame buffer: Entity buffer conflicts with light buffer (same color attachment)");
		return false;
	}

	if (attachFlags & ATTACH_COLOR)
		CreateAttachment(ATTACH_COLOR,_width, _height);

	if (attachFlags & ATTACH_DEPTH)
		CreateAttachment(ATTACH_DEPTH, _width, _height);

	if (attachFlags & ATTACH_STENCIL)
		CreateAttachment(ATTACH_STENCIL, _width, _height);

	if (attachFlags & ATTACH_ENTITY_ID)
		CreateAttachment(ATTACH_ENTITY_ID, _width, _height);

	if (attachFlags & ATTACH_ADDITIONAL_COLOR)
		CreateAttachment(ATTACH_ADDITIONAL_COLOR, _width, _height);

	if (attachFlags & ATTACH_LIGHT)
		CreateAttachment(ATTACH_LIGHT, _width, _height);
	// ensure fbo generated correctly
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		LOGE("Failed to generate frame buffer: %d", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		return false;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	isInit = true;
	width = _width;
	height = _height;
	return true;
}
void FrameBuffer::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

	/* sets output of fragment shader, attachment 1 used for object picking
	*  layout (location=0) will write to Attachment 0
	*  layout (location=1) will write to Attachment 1
	*/
	GLenum bufs[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, bufs);
}
void FrameBuffer::Draw(GLSLShader& shader)
{
	shader.Use();
	glBindTexture(GL_TEXTURE_2D, colorBufferTex);
	shader.SetUniform("uTex2d", 0);
	CEO::Instance().Get<RenderUtils>()->RenderQuad();
	shader.UnUse();
}
void FrameBuffer::Unbind()
{
	// reset to default frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void FrameBuffer::Resize(GLuint _width, GLuint _height)
{
	width = _width;
	height = _height;

	if (frameBuffer) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		// resize color buffer
		if (attachFlags & ATTACH_COLOR)
		{
			glBindTexture(GL_TEXTURE_2D, colorBufferTex);
			glTexImage2D(GL_TEXTURE_2D, 0, hasAlphaChannel?GL_RGBA:GL_RGB, _width, _height, 0, hasAlphaChannel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		if (attachFlags & ATTACH_ADDITIONAL_COLOR)
		{
			glBindTexture(GL_TEXTURE_2D, secondaryColorBufferTex);
			glTexImage2D(GL_TEXTURE_2D, 0, hasAlphaChannel ? GL_RGBA : GL_RGB, _width, _height, 0, hasAlphaChannel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		if (attachFlags & ATTACH_LIGHT)
		{
			glBindTexture(GL_TEXTURE_2D, lightIntensityTex);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, _width, _height, 0, GL_RED, GL_FLOAT, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		// render buffers recommended to be recreated instead of resize according to docs
		if (attachFlags & ATTACH_DEPTH)
		{
			FreeAttachment(ATTACH_DEPTH);
			CreateAttachment(ATTACH_DEPTH, _width, _height);
		}

		if (attachFlags & ATTACH_STENCIL)
		{
			FreeAttachment(ATTACH_STENCIL);
			CreateAttachment(ATTACH_STENCIL, _width, _height);
		}

		if (attachFlags & ATTACH_ENTITY_ID)
		{
			glBindTexture(GL_TEXTURE_2D, entityIDBuffer);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, _width, _height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		Unbind();
	}
}
void FrameBuffer::Free()
{
	if (frameBuffer)
	{
		FreeAttachment(ATTACH_COLOR);
		FreeAttachment(ATTACH_DEPTH);
		FreeAttachment(ATTACH_STENCIL);
		FreeAttachment(ATTACH_ENTITY_ID);
		FreeAttachment(ATTACH_ADDITIONAL_COLOR);
		FreeAttachment(ATTACH_LIGHT);
		glDeleteFramebuffers(1, &frameBuffer);
	}
	isInit = false;
}

GLuint FrameBuffer::ColorBufferTex() const
{
	return colorBufferTex;
}

GLuint FrameBuffer::SecondaryColorBufferTex() const
{
	return secondaryColorBufferTex;
}

GLuint FrameBuffer::LightBufferTex() const
{
	return lightIntensityTex;
}

void FrameBuffer::CreateAttachment(AttachmentFlag attachment, GLuint _width, GLuint _height)
{
	switch (attachment)
	{
	case ATTACH_COLOR:
		// initialize color buffer texture
		glGenTextures(1, &colorBufferTex);
		glBindTexture(GL_TEXTURE_2D, colorBufferTex);
		glTexImage2D(GL_TEXTURE_2D, 0, hasAlphaChannel ? GL_RGBA : GL_RGB, _width, _height, 0, hasAlphaChannel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		// attach color buffer to frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBufferTex, 0);
		break;
	case ATTACH_DEPTH:
		// initialize depth buffer
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, _width, _height);
		/*glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH, _width, _height);*/
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// attach depth buffer to frame buffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
		break;
	case ATTACH_STENCIL:
		// initialize stencil buffer
		glGenRenderbuffers(1, &stencilBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, stencilBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL, _width, _height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// attach stencil buffer to frame buffer
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilBuffer);
		break;
	case ATTACH_ENTITY_ID:
		// initialize color buffer texture
		glGenTextures(1, &entityIDBuffer);
		glBindTexture(GL_TEXTURE_2D, entityIDBuffer);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, _width, _height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, _width, _height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glBindTexture(GL_TEXTURE_2D, 0);
		// attach color buffer to frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, entityIDBuffer, 0);
		break;
	case ATTACH_ADDITIONAL_COLOR:
		// initialize color buffer texture
		glGenTextures(1, &secondaryColorBufferTex);
		glBindTexture(GL_TEXTURE_2D, secondaryColorBufferTex);
		glTexImage2D(GL_TEXTURE_2D, 0, hasAlphaChannel ? GL_RGBA : GL_RGB, _width, _height, 0, hasAlphaChannel ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		// attach color buffer to frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, secondaryColorBufferTex, 0);
		break;
	case ATTACH_LIGHT:
		// initialize color buffer texture
		glGenTextures(1, &lightIntensityTex);
		glBindTexture(GL_TEXTURE_2D, lightIntensityTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, _width, _height, 0, GL_RED, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);

		// attach color buffer to frame buffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, lightIntensityTex, 0);
		break;
	default:
		LOGE("Failed to create frame buffer attachment: unknown attachments specified: %d", attachment);
		break;
	}
}

void FrameBuffer::FreeAttachment(AttachmentFlag attachment)
{
	switch (attachment)
	{
	case ATTACH_COLOR:
		glDeleteTextures(1, &colorBufferTex);
		colorBufferTex = 0;
		break;
	case ATTACH_DEPTH:
		glDeleteRenderbuffers(1, &depthBuffer);
		depthBuffer = 0;
		break;
	case ATTACH_STENCIL:
		glDeleteRenderbuffers(1, &stencilBuffer);
		stencilBuffer = 0;
		break;
	case ATTACH_ENTITY_ID:
		glDeleteTextures(1, &entityIDBuffer);
		entityIDBuffer = 0;
		break;
	case ATTACH_ADDITIONAL_COLOR:
		glDeleteTextures(1, &secondaryColorBufferTex);
		secondaryColorBufferTex = 0;
		break;
	case ATTACH_LIGHT:
		glDeleteTextures(1, &lightIntensityTex);
		lightIntensityTex = 0;
		break;
	default:
		LOGE("Failed to free frame buffer attachment: unknown attachments specified: %d", attachment);
		break;
	}
}
