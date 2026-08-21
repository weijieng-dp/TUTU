/*!
@file       RenderBuffer.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Interface for interacting OpenGL buffer data.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "RenderBuffer.h"

RenderBuffer::RenderBuffer(RenderBuffer&& buffer) noexcept
	: vboid(std::exchange(buffer.vboid, 0)),  bufferType(buffer.bufferType), 
	bufferUsage(buffer.bufferUsage){ }

RenderBuffer& RenderBuffer::operator=(RenderBuffer&& buffer) noexcept {
	if (this != &buffer) {
		if(vboid != 0) glDeleteBuffers(1, &vboid);
		vboid = std::exchange(buffer.vboid, 0);
		bufferType = buffer.bufferType;
		bufferUsage = buffer.bufferUsage;
	}
	return *this;
}

void RenderBuffer::Bind() const { glBindBuffer(bufferType, vboid); }
void RenderBuffer::Unbind(GLenum target) { 
	/*if(target != GL_ELEMENT_ARRAY_BUFFER) */glBindBuffer(target, 0); 
}

void RenderBuffer::CreateRenderBuffer() {
	if (vboid == 0) glGenBuffers(1, &vboid);
}

void RenderBuffer::DeleteRenderBuffer() {
	if (vboid != 0) {	// check if buffer had been created
#ifdef PLATFORM_ANDROID
		if (eglGetCurrentContext() != EGL_NO_CONTEXT) // check if egl context exists before deleting
#endif // PLATFORM_ANDROID
		glDeleteBuffers(1, &vboid);	// delete if so
	}
	vboid = 0;					// set id back to 0
}

void RenderBuffer::SetBuffer(const void* data, GLsizeiptr size) {
	if (size <= 0) {
		LOGI("Can't set RenderBuffer with size <= 0.");
		return;
	}

	CreateRenderBuffer();	// create buffer if not already created

	this->Bind();		// binds the buffer
	glBufferData(bufferType, size, data, bufferUsage);
	if(bufferType != GL_ELEMENT_ARRAY_BUFFER) // don't unbind if buffer is ebo (because unbinding before unbind vao will make it forget the binding
		RenderBuffer::Unbind(bufferType);	// unbind the buffer
}

void RenderBuffer::ConfigureBuffer(const void* data, GLsizeiptr size, GLsizeiptr count, GLintptr offset) {
	if (bufferUsage == GL_STATIC_DRAW || data == nullptr || vboid == 0) return;
	this->Bind();						// bind the buffer
	glBufferData(bufferType, size, nullptr, GL_DYNAMIC_DRAW);	// orphan the buffer
	glBufferSubData(bufferType, offset, count, data);	// update the data accordingly
	RenderBuffer::Unbind(bufferType);	// unbind the buffer
}