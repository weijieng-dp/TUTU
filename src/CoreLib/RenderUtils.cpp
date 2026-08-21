/*!
@file       RenderUtils.cpp
@author     Tan Jun Jie (t.junjie) 70%
@co-author  Ou Yukang (yukang.ou) 30%
@date       25/09/2025
@brief		Handles primitives and font rendering.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "RenderUtils.h"
#undef PLATFORM_WINDOWS

#ifdef PLATFORM_WINDOWS
static void GLAPIENTRY
 MessageCallback(GLenum source,
     GLenum type,
     GLuint id,
     GLenum severity,
     GLsizei length,
     const GLchar* message,
     const void* userParam)
 {
     // ignore if just a notification
     if (type != GL_DEBUG_TYPE_ERROR)
         return;

     LOGE("GL ERROR:type = 0x%x, severity = 0x%x, message = %s\n",
         type, severity, message);
 }
#endif

#pragma region Init Functions
void RenderUtils::Init() {
	if (initialised) return;
#ifdef _DEBUG
	LOGI("Initializing RenderUtils");
#endif
	InitQuad();				// initialise quad primitive
	InitCircle();			// initialise circle primitive
	InitQuadWireframe();	// initialise quad wireframe primitive
	InitInstancing(entityInstance, instanceLimit);	// initialise instancing
	InitInstancing(particleInstance[&CEO::Instance().GetManager<ResourceManager>()->GetErrorTex()], 
		particleInstanceLimit);		// initialise instancing

	glEnable(GL_DEPTH_TEST);		// enable depth test for layering
	glDepthFunc(GL_GREATER);		// set depth func to use: pass if depth value is greater
#ifdef _DEBUG
	LOGI("OPENGL Version: %s" , glGetString(GL_VERSION));
#endif
#ifdef PLATFORM_WINDOWS
#ifdef _DEBUG
  	// opengl error callback
	glEnable(GL_DEBUG_OUTPUT);
  	glDebugMessageCallback(MessageCallback, 0);
#endif
#endif
	initialised = true;
}

void RenderUtils::InitQuad() {
	std::array<GLfloat, 24> vtx{
		//	x		y	u		v
		-0.5f,	0.5f,	0.f,	1.f,
		-0.5f,	-0.5f,	0.f,	0.f,
		0.5f,	0.5f,	1.f,	1.f,
		0.5f,	0.5f,	1.f,	1.f,
		0.5f,	-0.5f,	1.f,	0.f,
		-0.5f,	-0.5f,	0.f,	0.f
	};
	// pre-calulation
	GLsizeiptr bufferSize{ vtx.size() * sizeof(GLfloat) };
	GLsizei stride{ sizeof(GLfloat) * 4 };
	GLuint posRelOffSet{}, texRelOffSet{ sizeof(GLfloat) * 2 };

	quad.vbo.SetBuffer(vtx.data(), bufferSize);
	quad.vao.SetAttribute(quad.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_POSITION, 2, stride, 0, posRelOffSet);
	quad.vao.SetAttribute(quad.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TEXTURE, 2, stride, 0, texRelOffSet);
}
void RenderUtils::InitCircle() {
	std::vector<GLfloat> vtx;
	vtx.reserve(static_cast<size_t>((64 + 2) * 2));		// use 64 slices / segment

	const GLfloat angle{ 360.f / 64 },
		conversion{ PI / 180.f };		// for converting degree to radians, precalculate pi / 180

	vtx.emplace_back(0.f); vtx.emplace_back(0.f);
	for (int i{}; i < 64; ++i) {
		GLfloat radAngle{ (angle * i) * conversion };
		vtx.emplace_back(cosf(radAngle));
		vtx.emplace_back(sinf(radAngle));
	}
	vtx.emplace_back(cosf(0.f)); vtx.emplace_back(sinf(0.f));

	// pre-calulation
	GLsizeiptr bufferSize{ static_cast<GLsizeiptr>(vtx.size() * sizeof(GLfloat)) };
	GLsizei stride{ sizeof(GLfloat) * 2 };	

	circle.vbo.SetBuffer(vtx.data(), bufferSize);
	circle.vao.SetAttribute(circle.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_POSITION, 2, stride, 0, 0);

}
void RenderUtils::InitQuadWireframe() {
	std::array<GLshort, 4> idx{ 1, 4, 2, 0 };	// reuse quad vbo but only points to draw border

	// create the EBO
	quadWireframeEbo.SetBuffer(idx.data(), idx.size() * sizeof(GLshort));

	// create the wireframe vao
	quadWireframeVao.SetAttribute(quad.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_POSITION,
		2, sizeof(GLfloat) * 4, 0, 0);

	// bind the EBO to the vao
	quadWireframeVao.Bind();
	quadWireframeEbo.Bind();
	RenderAttributes::Unbind();
	RenderBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER);
}

void RenderUtils::InitInstancing(PrimitiveMesh& instanceMesh, size_t m_instanceLimit) {
	GLsizei stride{ sizeof(GLfloat) * 4 };
	GLuint posRelOffSet{}, texRelOffSet{ sizeof(GLfloat) * 2 };

	instanceMesh.vbo.BufferUsage() = GL_DYNAMIC_DRAW;		// set buffer as dynamic draw
	instanceMesh.vbo.SetBuffer(nullptr, m_instanceLimit * sizeof(InstanceData));
	instanceMesh.vao.SetAttribute(quad.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_POSITION,
		2, stride, 0, posRelOffSet);
	instanceMesh.vao.SetAttribute(quad.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TEXTURE,
		2, stride, 0, texRelOffSet);

	// set up the VAO for the transforms in instance vbo
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM1,
		3, sizeof(InstanceData), 0, offsetof(InstanceData, xform));
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM2,
		3, sizeof(InstanceData), 0, offsetof(InstanceData, xform) + sizeof(GLfloat) * 3);
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM3,
		3, sizeof(InstanceData), 0, offsetof(InstanceData, xform) + sizeof(GLfloat) * 6);

	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_SPRITE_ORIGIN,
		2, sizeof(InstanceData), 0, offsetof(InstanceData, sOrigin));
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_SPRITE_SIZE,
		2, sizeof(InstanceData), 0, offsetof(InstanceData, sSize));

	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_ID,
		1, sizeof(InstanceData), 0, offsetof(InstanceData, id), GL_UNSIGNED_INT);

	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_COLOR,
		4, sizeof(InstanceData), 0, offsetof(InstanceData, color));
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TILE,
		2, sizeof(InstanceData), 0, offsetof(InstanceData, sTile));
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_EMISSION,
		1, sizeof(InstanceData), 0, offsetof(InstanceData, emissiveState), GL_INT);
	instanceMesh.vao.Bind();		// bind the instance vao
	// Set Up the Attribute divisior for Instance Data
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_POSITION, 0);		// use diff pos per vertex
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TEXTURE, 0);		// use diff uv per vertex
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM1, 1);		/// change instance buffer per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM2, 1);		// change instance buffer per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM3, 1);		// change instance buffer per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_SPRITE_ORIGIN, 1);	// change instance buffer per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_SPRITE_SIZE, 1);	// change instance buffer per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_ID, 1);				// change instance buffer per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_COLOR, 1);			// change instance buffer per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TILE, 1);			// change instance buffer per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_EMISSION, 1);			// change instance buffer per instance
	RenderAttributes::Unbind();	// unbind the instance vao
}

#pragma endregion
void RenderUtils::Free() {
#ifdef _DEBUG
	LOGI("Cleaning RenderUtils:");
	LOGI("Cleaning circle: [VAO id: %d] | [id: %d]", circle.vao.Id(), circle.vbo.Id());
#endif
	circle.Free();

#ifdef _DEBUG
	LOGI("Cleaning quad: [VAO id: %d] | [VBO id: %d]", quad.vao.Id(), quad.vbo.Id());
#endif
	quad.Free();

#ifdef _DEBUG
	LOGI("Cleaning wireframe: [VAO id: %d] | [EBO id: %d]", quadWireframeVao.Id(), quadWireframeEbo.Id());
#endif
	quadWireframeVao.DeleteVAO();			// free up quad wireframe vao
	quadWireframeEbo.DeleteRenderBuffer();	// free up quad wireframe index buffer resources

#ifdef _DEBUG
	LOGI("Cleaning Entity Instance:  [VAO id: %d] | [VBO id: %d]", entityInstance.vao.Id(), entityInstance.vbo.Id());
#endif
	entityInstance.Free();

	for (auto& [tex, instance] : particleInstance) {
#ifdef _DEBUG
		LOGI("Cleaning Particle Instance: [VAO id: %d] | [VBO id: %d]", instance.vao.Id(), instance.vbo.Id());
#endif	
		instance.Free();
	}

	initialised = false;
}

#pragma region Render Function
void RenderUtils::RenderQuadWireframe() {
	BindQuadWireframeVAO();
	glDrawElements(GL_LINE_LOOP, 4, GL_UNSIGNED_SHORT, NULL); /* // draws the quad wireframe with line loop */
	RenderAttributes::Unbind();
}

void RenderUtils::RenderQuad() {
	BindQuadVAO();
	glDrawArrays(GL_TRIANGLES, 0, 6); /* draws the quad with triangles */
	RenderAttributes::Unbind();
}

void RenderUtils::RenderCircle() {
	BindCircleVAO();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 66); /* draw circle with triangle fan (66 slices) */
	RenderAttributes::Unbind();
}

void RenderUtils::RenderCircleWireframe() {
	BindCircleVAO();
	glDrawArrays(GL_LINE_LOOP, 1, 65);	/* draw the circle wireframe using GL_LINE_LOOP */
	RenderAttributes::Unbind();
}

void RenderUtils::RenderLine() {
	BindQuadVAO();
	glDrawArrays(GL_LINES, 0, 2);
	RenderAttributes::Unbind();
}

void RenderUtils::RenderTriangle() {
	BindQuadVAO();
	glDrawArrays(GL_TRIANGLES, 0, 3); /* draw the first 3 vertices as triangle */
	RenderAttributes::Unbind();
}
void RenderUtils::RenderText(std::string const& text, Mat3 mvp, Vec2 pos, float fontSize, Color color, FontObj const& font) {
	if (text.empty()) return;	//nothing to render

	// Init transforms
	Mat3 scale = Mat3::Scale(fontSize / font.resolution, fontSize / font.resolution);
	Mat3 pivot = Mat3::Translation(0.5f, -0.5f);
	Mat3 parentOffset = Mat3::Translation(pos.x, pos.y);
	Mat3 charOffset = Mat3::Identity();
	Mat3 charScale = Mat3::Identity(); // characters are not perfect squares, deformation needed
	Mat3 transform = Mat3::Identity();

	// Init shader properties
	auto shader{ CEO::Get<ResourceManager>()->GetShader("font") };// get font shader
	shader.Use();										// use font shader
	glEnable(GL_BLEND);									// enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// set blend function
	glActiveTexture(GL_TEXTURE0); // set texture unit that future gl calls modify
	shader.SetUniform("uTex2d", 0);			// set shader to use texture unit 0
	shader.SetUniform("uColour", color);	// pass in the colour to use

	float offsetX = 0;
	BindQuadVAO();		// bind quad vao (prevent repeated binding and unbinding)
	for (int i = 0; i < text.size(); ++i)
	{
		FontObj::Character const& character = font.map.at(text[i]);
		float xCharOffset = (offsetX + character.bearingX);
		float yCharOffset = ((-(static_cast<float>(static_cast<float>(character.sizeY) - static_cast<float>(character.bearingY))) + character.sizeY / 2.f));
		charOffset = Mat3::Translation(xCharOffset, yCharOffset);
		charScale = Mat3::Scale(static_cast<float>(character.sizeX),
			static_cast<float>(character.sizeY));

		glBindTexture(GL_TEXTURE_2D, character.texID);		// bind the texture for the current font character
		shader.SetUniform("mdl_to_ndc", mvp * parentOffset * scale * charOffset * charScale * pivot);
		RenderQuadRaw();									// render as a quad

		// offset to the starting position of nect character
		offsetX += character.advance;
	}
	RenderAttributes::Unbind();			// unbind quad vao
	glDisable(GL_BLEND);				// disable blending
	glBindTexture(GL_TEXTURE_2D, 0);
	GLSLShader::UnUse();				// unuse font shader
}

void RenderUtils::RenderTextRaw(std::string const& text, Mat3 mvp, Vec2 pos, float fontSize, GLSLShader& shader, FontObj const& font) {
	if (text.empty()) return;
	Mat3 scale{ Mat3::Scale(fontSize / font.resolution, fontSize / font.resolution) },
		pivot{ Mat3::Translation(0.5f, 0.5f) },
		parentOffset{ Mat3::Translation(pos.x, pos.y) },
		charOffset{ Mat3::Identity() },
		charScale{ Mat3::Identity() }, // characters are not perfect squares, deformation needed
		transform{ Mat3::Identity() };

	glEnable(GL_BLEND);									// enable blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	// set blend function
	float offsetX{};
	for (int i = 0; i < text.size(); ++i) {
		FontObj::Character const& character = font.map.at(text[i]);
		float xCharOffset = (offsetX + character.bearingX);
		float yCharOffset = static_cast<float>(-(character.sizeY - character.bearingY));
		charOffset = Mat3::Translation(xCharOffset, yCharOffset);
		charScale = Mat3::Scale(static_cast<float>(character.sizeX),
			static_cast<float>(character.sizeY));
		glBindTexture(GL_TEXTURE_2D, character.texID);		// bind the texture for the current font character
		shader.SetUniform("mdl_to_ndc", mvp * parentOffset * scale * charOffset * charScale * pivot);
		RenderQuadRaw();									// render as a quad

		// offset to the starting position of nect character
		offsetX += character.advance;
	}
	glDisable(GL_BLEND);				// disable blending
	glBindTexture(GL_TEXTURE_2D, 0);	// unbind texture

}
#pragma endregion

void RenderUtils::UpdateEntityInstanceBuffer(const std::vector<InstanceData>& xforms, size_t size) {
	entityInstance.vbo.ConfigureBuffer(xforms.data(), instanceLimit * sizeof(InstanceData), size * sizeof(InstanceData), 0);
}

void RenderUtils::UpdateParticleInstanceBuffer(const std::vector<InstanceData>& xforms, TextureObj* tex, size_t limit, size_t size) {
	if (limit == 0) InitInstancing(particleInstance[tex], particleInstanceLimit);	// initialise the vao, vbo if limit == 0
	particleInstance[tex].vbo.ConfigureBuffer(xforms.data(), limit * sizeof(InstanceData), size * sizeof(InstanceData), 0);
}