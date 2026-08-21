/*!
@file       RenderAttributes.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Interface for setting attributes for VAO.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "RenderAttributes.h"

RenderAttributes::RenderAttributes(RenderAttributes&& other) noexcept : 
	vaoid(std::exchange(other.vaoid, 0)) { }

RenderAttributes& RenderAttributes::operator=(RenderAttributes&& other) noexcept {
	if (this != &other) {	// check that move assignment operator is not called on the same object
		if (vaoid != 0)		// check if this RenderAttributes object has already created an vao
			this->DeleteVAO();	// if so, delete it
		
		vaoid = std::exchange(other.vaoid, 0);	// steals the vaoid of the other RenderAttributes
	}
	return *this;
}

void RenderAttributes::CreateVAO() {
	if (vaoid == 0) glGenVertexArrays(1, &vaoid);
}

void RenderAttributes::DeleteVAO() {
	if (vaoid != 0) {	// check if vao is initialised / created
#ifdef PLATFORM_ANDROID
		if (eglGetCurrentContext() != EGL_NO_CONTEXT) // check if egl context exists before deleting
#endif // PLATFORM_ANDROID
		glDeleteVertexArrays(1, &vaoid);	// delete vao
	}
	vaoid = 0;			// reset vao id back to 0
}

void RenderAttributes::SetAttribute(const RenderBuffer& buffer, ATTRIBUTE_INDEX attribIndex,
	GLint size, GLsizei stride, GLsizeiptr bufferOffset, GLuint relativeOffset, GLenum type,
	GLboolean normalised) {

	CreateVAO();	// create vao if not already created

	this->Bind();	// bind this vao
	buffer.Bind();	// bind the given buffer

	glEnableVertexAttribArray(attribIndex);
	switch (type) {
		// Intentional flow down
		case GL_BYTE: case GL_UNSIGNED_BYTE: case GL_SHORT: 
		case GL_UNSIGNED_SHORT: case GL_INT: case GL_UNSIGNED_INT:
			// for integral values
			glVertexAttribIPointer(attribIndex, size, type, stride, (void*)(bufferOffset + relativeOffset));
			break;
		// Intentional flow down
		case GL_HALF_FLOAT: case GL_FLOAT: case GL_FIXED: 
		case GL_INT_2_10_10_10_REV: case GL_UNSIGNED_INT_2_10_10_10_REV:
			// for floating-point values
			glVertexAttribPointer(attribIndex, size, type, normalised, stride, (void*)(bufferOffset + relativeOffset));
			break;
		default:
			LOGE("Invalid data type passed into Set Attribute.");
			break;
	}
	RenderAttributes::Unbind();	// unbind any active vao
	RenderBuffer::Unbind(buffer.BufferType());	// unbind the buffer
}