/*!
@file       DebugRender.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       25/09/2025
@brief		Handles all the drawing / rendering for debug (e.g.
			collision hitbox, velocity direction arrow, UI wireframes).


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*//*______________________________________________________________________*/
#include "DebugRender.h"
#include "Components.h"
#include <algorithm>
#include "GraphicsSystem.h"
#include <Camera.h>
#include "ParticleSystem.h"
#include "UpdateStackManager.h"

static bool init{ false }; // To make sure this only runs once

void DebugRender::Init() {
	if (!init) {
#ifdef _DEBUG
		LOGI("Initializing DebugRender");
#endif
		instanceCount.fill(0);			// set instance count to 0
		instanceLimits.fill(2500);		// set limit to an initial value of 2500
		debugColors.fill({ 1.f, 0.f, 0.f, 1.f });

		InitInstance(quadWireframeTransforms, quadWireframeInstance, DebugShapes::QUAD_WIREFRAME);
		InitInstance(circleWireframeTransforms, circleWireframeInstance, DebugShapes::CIRCLE_WIREFRAME);
		InitInstance(arrowTransforms, arrowInstance, DebugShapes::ARROW);
		init = true;
#ifdef _DEBUG
		LOGI("DebugRender initialized");
#endif
	}
}

#pragma region Normal Render
void DebugRender::DrawDebug(GLSLShader& shader) {
	DrawCamera();
	if (!debugDrawEnabled) return;		// if flag for debug draw is set to false, don't draw
	
	Mat3 viewProj{ CEO::Instance().GetManager<CameraManager>()->GetViewProjection() };
	Mat3 proj{ CEO::Instance().GetManager<CameraManager>()->GetProjection() };

	glLineWidth(lineWidth);						// set line width
	shader.Use();
	shader.SetUniform("uUseTex", 0);			// tell shader not to use texture
	
	shader.SetUniform("ndcDepth", 1.f);			// pass depth value of 0.f into shader (to draw debug on top)

	if (collisionDebugEnabled) DrawCollision(shader, viewProj);		// draw hitbox collision
	if (velocityDebugEnabled) {
		shader.SetUniform("uColour", debugColors[DebugTypes::VELOCITY_ARROW]);		// pass colour in
		DrawVelocity(shader, viewProj);			// draw velocity arrow
	}
	if (particleDebugEnabled) DrawParticleSpawnbox(shader, viewProj);
	if (uiDebugEnabled) {
		shader.SetUniform("uColour", debugColors[DebugTypes::UI_QUAD_WIREFRAME]);		// pass colour in
		DrawUI(shader, proj);						// draw UI wireframe
	}
	
	glLineWidth(1.f);					// reset line width
	shader.UnUse();
}

void DebugRender::DrawCollision(GLSLShader& shader, const Mat3& viewProj) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	shader.SetUniform("uColour", debugColors[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME]);		// pass colour in
	renderUtils.BindQuadWireframeVAO();		// pre-bind quad wireframe vao
	for (int i{}; i < instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME]; ++i) {	// loop through data in collision quad wireframe instance buffer
		Mat3 xform{ quadWireframeTransforms[i] };				// get the collision quad wireframe transform
		xform.m[8] = 1.f;									// reset depth value holder in transform matrix back to 1.f
		shader.SetUniform("mdl_to_ndc", viewProj * xform);	// pass transform into shader
		renderUtils.RenderQuadWireframeRaw();				// render quad wireframe
	}

	shader.SetUniform("uColour", debugColors[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME]);		// pass colour in
	renderUtils.BindCircleVAO();				// pre-bind circle vao
	for (int i{}; i < instanceCount[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME]; ++i) {	// loop through data in collision circle wireframe instance buffer
		Mat3 xform{ circleWireframeTransforms[i] };			// get the collision circle wireframe transform
		xform.m[8] = 1.f;									// reset depth value holder in transform matrix back to 1.f
		shader.SetUniform("mdl_to_ndc", viewProj * xform);	// pass transform into shader
		renderUtils.RenderCircleWireframeRaw();			// render circle wireframe
	}
	RenderAttributes::Unbind();					// unbind active vao
}

void DebugRender::DrawUI(GLSLShader& shader, const Mat3& proj) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	renderUtils.BindQuadWireframeVAO();				// pre-bind quad wireframe vao
	for (int i{}; i < instanceCount[DebugTypes::UI_QUAD_WIREFRAME]; ++i) {
		Mat3 xform{ quadWireframeTransforms[instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] + i] };					// get the UI wireframe transform
		xform.m[8] = 1.f;								// reset depth value holder in transform matrix back to 1.f
		shader.SetUniform("mdl_to_ndc", proj * xform);	// pass transform into shader
		renderUtils.RenderQuadWireframeRaw();			// render the UI wireframe
	}
	RenderAttributes::Unbind();							// unbind quad wireframe vao
}

void DebugRender::DrawParticleSpawnbox(GLSLShader& shader, const Mat3& viewProj) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	shader.SetUniform("uColour", debugColors[DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME]);		// pass colour in
	int particleQuadOffset{ instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] + instanceCount[DebugTypes::UI_QUAD_WIREFRAME] };
	renderUtils.BindQuadWireframeVAO();
	for (int i{}; i < instanceCount[DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME]; ++i) {	// loop through particle instance buffer
		Mat3 xform{ quadWireframeTransforms[particleQuadOffset + i] };				// get the particle quad wireframe transform
		xform.m[8] = 1.f;									// reset depth value holder in transform matrix back to 1.f
		shader.SetUniform("mdl_to_ndc", viewProj * xform);	// pass transform into shader
		renderUtils.RenderQuadWireframeRaw();				// render quad wireframe
	}

	shader.SetUniform("uColour", debugColors[DebugTypes::PARTICLE_SPAWNCIRCLE_WIREFRAME]);		// pass colour in
	renderUtils.BindCircleVAO();
	for (int i{}; i < instanceCount[DebugTypes::PARTICLE_SPAWNCIRCLE_WIREFRAME]; ++i) {	// loop through particle circle instance buffer
		Mat3 xform{ circleWireframeTransforms[instanceCount[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME] + i] };			// get the particle circle wireframe transform
		xform.m[8] = 1.f;									// reset depth value holder in transform matrix back to 1.f
		shader.SetUniform("mdl_to_ndc", viewProj * xform);	// pass transform into shader
		renderUtils.RenderCircleWireframeRaw();			// render circle wireframe
	}
	RenderAttributes::Unbind();								// unbind active vao
}

void DebugRender::DrawVelocity(GLSLShader& shader, const Mat3& viewProj) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	renderUtils.BindQuadVAO();		// pre-bind the quad vao
	for (int i{}; i < instanceCount[DebugTypes::VELOCITY_ARROW]; ++i) {	// loop through the data in velocity instance buffer
		Mat3 xform{ arrowTransforms[i] };				// get the velocity transform
		xform.m[8] = 1.f;									// reset depth value holder in transform matrix back to 1.f
		shader.SetUniform("mdl_to_ndc", viewProj * xform);	// pass transform into shader
		renderUtils.RenderTriangleRaw();					// draw triangle

	}
	RenderAttributes::Unbind();		// unbind the quad vao after drawing
}

void DebugRender::DrawCamera() {
	CameraManager& camManager = *CEO::Get<CameraManager>();
	if(!camManager.HasMainCamera())
		CEO::Instance().Get<RenderUtils>()->RenderText("No main camera found", camManager.GetProjection(), 
			Vec2{ -camManager.GetViewportSize().x / 2.f, camManager.GetViewportSize().y / 2.f - 32 }, 64, Color{ 1.f,1.f,1.f,1.f });
}
#pragma endregion

#pragma region Instanced Rendering
void DebugRender::DrawDebugInstanced(GLSLShader& shader) {
	DrawCamera();
	if (!debugDrawEnabled) return;		// if flag for debug draw is set to false, don't draw

	glLineWidth(lineWidth);				// set the line width

	shader.Use();
	shader.SetUniform("uUseTex", GL_FALSE);
	shader.SetUniform("uUseDebugColor", GL_TRUE);
	shader.SetUniform("uViewProj", CEO::Instance().GetManager<CameraManager>()->GetViewProjection());

	if(collisionDebugEnabled) DrawCollisionInstanced(shader);	// draw collision using instanced if collision debug is enabled
	if (velocityDebugEnabled) {
		shader.SetUniform("uDebugColor", debugColors[DebugTypes::VELOCITY_ARROW]);		// pass colour in
		DrawVelocityInstanced();	// draw velocity using instanced if velocity debug is enabled
	}
	if (particleDebugEnabled) DrawParticleSpawnboxInstanced(shader);	//draw particle spawn area using instanced if velocity debug is enabled
	if (uiDebugEnabled) {
		shader.SetUniform("uDebugColor", debugColors[DebugTypes::UI_QUAD_WIREFRAME]);		// pass colour in
		shader.SetUniform("uViewProj", CEO::Instance().GetManager<CameraManager>()->GetProjection());
		DrawUIInstanced();
	}
	
	glLineWidth(1.f);					// reset line width back to 0
	shader.UnUse();
}

void DebugRender::DrawCollisionInstanced(GLSLShader& shader) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	if (instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME]) {	// check if there are quad collision wireframes to draw
		shader.SetUniform("uDebugColor", debugColors[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME]);		// pass colour in
		quadWireframeInstance.vao.Bind();	// bind the instance vao for quad collision wireframe
		OffsetInstanceBuffer(quadWireframeInstance, 0);		// reset the offset for quadWireframe back to 0
		renderUtils.RenderInstancedIndexed(instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME], GL_LINE_LOOP, 4);
	}

	if (instanceCount[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME]) {	// check if there are circle collision wireframes to draw
		shader.SetUniform("uDebugColor", debugColors[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME]);		// pass colour in
		circleWireframeInstance.vao.Bind();	// bind the instance vao for circle collision wireframe
		OffsetInstanceBuffer(circleWireframeInstance, 0);	// reset the offset for circleWireframe back to 0
		renderUtils.RenderInstanced(instanceCount[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME], GL_LINE_LOOP, 1, 65);
	}
	RenderAttributes::Unbind();		// unbind any active vao
}

void DebugRender::DrawUIInstanced(){
	if (instanceCount[DebugTypes::UI_QUAD_WIREFRAME] == 0) return;	// if there are no UI wireframes to draw, end func early
	quadWireframeInstance.vao.Bind();			// bind the quadWireframe instance vao for ui
	// offset the quadWireframe buffer by the size of entity quad collision wireframe
	OffsetInstanceBuffer(quadWireframeInstance, instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] * sizeof(Mat3));
	CEO::Instance().Get<RenderUtils>()->RenderInstancedIndexed(instanceCount[DebugTypes::UI_QUAD_WIREFRAME], GL_LINE_LOOP, 4);
	RenderAttributes::Unbind();		// unbind the ui instance vao
}

void DebugRender::DrawParticleSpawnboxInstanced(GLSLShader& shader) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	if (instanceCount[DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME]) {
		shader.SetUniform("uDebugColor", debugColors[DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME]);		// pass colour in
		// calculate the offset till first element of particle spawnbox
		int particleQuadOffset{ instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] +
			instanceCount[DebugTypes::UI_QUAD_WIREFRAME] };
		quadWireframeInstance.vao.Bind();		// bind the quadWireframe instance vao for particle spawnbox
		OffsetInstanceBuffer(quadWireframeInstance, particleQuadOffset * sizeof(Mat3));
		renderUtils.RenderInstancedIndexed(instanceCount[DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME], GL_LINE_LOOP, 4);
	}
	if (instanceCount[DebugTypes::PARTICLE_SPAWNCIRCLE_WIREFRAME]) {
		shader.SetUniform("uDebugColor", debugColors[DebugTypes::PARTICLE_SPAWNCIRCLE_WIREFRAME]);		// pass colour in
		// calculate the offset till first element of particle spawncircle
		int particleCircleOffset{ instanceCount[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME] };
		circleWireframeInstance.vao.Bind();		// bind the circleWireframe instance vao for particle spawncircle
		OffsetInstanceBuffer(circleWireframeInstance, particleCircleOffset * sizeof(Mat3));
		renderUtils.RenderInstanced(instanceCount[DebugTypes::PARTICLE_SPAWNCIRCLE_WIREFRAME], GL_LINE_LOOP, 1, 65);
	}
	RenderAttributes::Unbind();		// unbind any active vao
}

void DebugRender::DrawVelocityInstanced() {
	if (instanceCount[DebugTypes::VELOCITY_ARROW] == 0) return;		// if there are no velocity arrows to draw, end func early
	arrowInstance.vao.Bind();	// bind the instance vao for velocity
	CEO::Instance().Get<RenderUtils>()->RenderInstanced(instanceCount[DebugTypes::VELOCITY_ARROW], GL_TRIANGLES, 0, 3);
	RenderAttributes::Unbind();		// unbind the velocity instance vao
}
#pragma endregion

#pragma region Update
void DebugRender::UpdateInstancedTransform(Registry& registry) {
	static Vec2 triCenter{ (-0.5f + -0.5f + 0.5f) / 3.f , (0.5f + -0.5f + 0.5f) / 3.f };
	static Mat3 preTransform{							// transform triangle vertices to point at +tive x-axis
		Mat3::Translation(-triCenter.x, -triCenter.y) *	// translate center of the triangle to origin
		Mat3::Rotation(ToRad(-135.f))					// rotate to point at positive x-axis
	};

	// get all entities that has transform component
	auto entities{ registry.GetEntitiesWithComponents<TransformComponent>() };

	LayerManager& layerManager{ *CEO::Instance().GetManager<LayerManager>() };
	uint64_t cameraMask{ CEO::Instance().GetManager<CameraManager>()->GetMainCamMask() };
	auto& usm{ *CEO::Get<UpdateStackManager>() };

	// increase instance buffer size if limit is reached
	if (entities.size() >= instanceLimits[QUAD_WIREFRAME]) ResizeInstanceBuffer(QUAD_WIREFRAME, entities.size());
	if (entities.size() >= instanceLimits[CIRCLE_WIREFRAME]) ResizeInstanceBuffer(CIRCLE_WIREFRAME, entities.size());
	if (entities.size() >= instanceLimits[ARROW]) ResizeInstanceBuffer(ARROW, entities.size());

	std::for_each(instanceCount.begin(), instanceCount.end(), [](int& count) { count = 0; });	// reset instance count to 0
	Mat3 collision_xform, velocity_xform, ui_xform, particle_xform;
	for (EntityRegistry::Entity entity : entities) {	// iterate through each entity
		if (!usm.IsActive(entity)) continue;
		SpriteRendererComponent* renderer{ registry.GetComponent<SpriteRendererComponent>(entity) };
		if (renderer != nullptr && !renderer->visible) continue; // skip not visible entities
		LayerComponent* layerComp{ registry.GetComponent<LayerComponent>(entity) };	// get layer component of entity
		// skip entities rendered on layers that is not enabled on camera
		if (layerComp != nullptr && !layerManager.IsVisibleOnCam(cameraMask, layerComp->layer)) continue;

		TransformComponent& transform{ *registry.GetComponent<TransformComponent>(entity) };	// get transform component
		CollisionComponent* collider{ registry.GetComponent<CollisionComponent>(entity) };		// get collision component
		PhysicsComponent* physics{ registry.GetComponent<PhysicsComponent>(entity) };			// get physics component

		Vec2 worldPos{ transform.transform.m[6], transform.transform.m[7] };		// extract translate
		Vec2 worldScale{	// extract scale
			std::sqrtf(transform.transform.m[0] * transform.transform.m[0] + transform.transform.m[1] * transform.transform.m[1]),
			std::sqrtf(transform.transform.m[3] * transform.transform.m[3] + transform.transform.m[4] * transform.transform.m[4])};

		if (collider != nullptr) {	// if it has collision component, render collision wireframe
			// Calculate the scale based on the half extents (for box) or radius (for circle)
			collision_xform = (collider->shape == Shape::Box || collider->shape == Shape::Capsule) ?
				Mat3::Scale(collider->baseHalfExtents.x * 2.f, collider->baseHalfExtents.y * 2.f) :
				Mat3::Scale(collider->radius, collider->radius);


			// calculate the translation based on the offset from center
			collision_xform = Mat3::Translation(collider->offset.x, collider->offset.y) * collision_xform;
			
			// add the visual scale
			collision_xform = Mat3::Scale(collider->colliderScale.x, collider->colliderScale.y) * collision_xform;

			// transform by each entity's scale and translation
			collision_xform =  Mat3::Translation(worldPos.x, worldPos.y) *			// entity's translation
								Mat3::Scale(worldScale.x, worldScale.y) *			// entity's scale
								collision_xform;
			collision_xform.m[8] = 1.f;

			// update the collisionTransforms with the calculated collision_xform
			switch (collider->shape) {
			case Shape::Box:
				quadWireframeTransforms[instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME]++] = collision_xform;
				break;
			case Shape::Circle:
				circleWireframeTransforms[instanceCount[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME]++] = collision_xform;
				break;
			case Shape::Capsule:
				quadWireframeTransforms[instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME]++] = collision_xform;
				break;
			}
		}
		if (physics != nullptr) {	// if it has physics component, render velocity arrow
			float length{ physics->velocity.LengthSquared() };	// get the squared length of the velocity vector
			if (length < 1e-10f) continue;						// if its too small, don't render
			Vec2 norm{ physics->velocity / std::sqrt(length) };	// find the normalised vector (for the direction)

			// calculate the offset based on entity's scale to draw velocity arrow (so that it is drawn around entity)
			Vec2 absScale{ fabs(worldScale.x), fabs(worldScale.y) };

			float xOffset{ fabs(norm.x) > 1e-6f ? (absScale.x * 0.5f) / fabs(norm.x) : 0.f };	// calculate x offset
			float yOffset{ fabs(norm.y) > 1e-6f ? (absScale.y * 0.5f) / fabs(norm.y) : 0.f };	// calculate y offset
			float offset{			// find how much to offset by
				xOffset > 0.f && yOffset > 0.f ? std::min(xOffset, yOffset)	// use min if both x & y offset are above 0
				: std::max(xOffset, yOffset)	// if either axis offset is 0, use the other value
			};

			Vec2 arrowSize{	// calculate the arrow size based on velocityArrowSize, entity's scale, and velocity
				// set arrow size		entity's scale				velocity (to make arrow pointer the higher the velocity)
				velocityArrowSize.x + absScale.x * 0.3f + length * 0.0003f ,
				velocityArrowSize.y + absScale.y * 0.3f
			};

			// calculate the Rotation Matrix to orientate the arrow to where the velocity vector
			velocity_xform = Mat3::Rotation(atan2(physics->velocity.y, physics->velocity.x));

			velocity_xform = 
				Mat3::Translation(worldPos.x + norm.x * offset, worldPos.y + norm.y * offset)		// Offset arrow from entity center to around the border of collision hitbox
				* velocity_xform						// the rotation to orientate arrow to velocity vector
				* Mat3::Scale(arrowSize.x, arrowSize.y)	// how much to scale the arrow
				* preTransform;							// the pre transform to prepare triangle vertices

			velocity_xform.m[8] = 1.f;

			// update the velocityTransforms with the calculated velocity_xform
			arrowTransforms[instanceCount[DebugTypes::VELOCITY_ARROW]++] = velocity_xform;
		}
	}

	// get all entities with UITransform component
	auto uiEntities = registry.GetEntitiesWithComponent<UITransformComponent>();
	// increase instance buffer size if limit is reached
	if (uiEntities.size() + instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] >= instanceLimits[QUAD_WIREFRAME])
		ResizeInstanceBuffer(DebugShapes::QUAD_WIREFRAME, 
			instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] + uiEntities.size());

	for (EntityRegistry::Entity entity : uiEntities) {	// iterate through each entity
		if (!usm.IsActive(entity)) continue;

		auto* layerComp{ registry.GetComponent<LayerComponent>(entity) };	// get layer component of entity
		if (layerComp != nullptr && !layerManager.IsVisibleOnCam(cameraMask, layerComp->layer)) 
			continue;	// skip entities rendered on layers that is not enabled on camera
		auto* transform{ registry.GetComponent<UITransformComponent>(entity) };	// get the transform component for this entity
		ui_xform = transform->transform;
		ui_xform.m[8] = 1.f;
		quadWireframeTransforms[instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] + 
			instanceCount[DebugTypes::UI_QUAD_WIREFRAME]++] = ui_xform;
	}

	int particleQuadOffset{ instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] +
		instanceCount[DebugTypes::UI_QUAD_WIREFRAME] },
		particleCircleOffset{ instanceCount[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME] };

	auto particleEntities{ registry.GetEntitiesWithComponent<ParticleEmitterComponent>() };
	if (particleEntities.size() + particleQuadOffset >= instanceLimits[DebugShapes::QUAD_WIREFRAME])
		ResizeInstanceBuffer(DebugShapes::QUAD_WIREFRAME, particleEntities.size() + particleQuadOffset);

	if (particleEntities.size() + particleCircleOffset >= instanceLimits[DebugShapes::CIRCLE_WIREFRAME])
		ResizeInstanceBuffer(DebugShapes::CIRCLE_WIREFRAME, particleEntities.size() + particleCircleOffset);

	for (auto entity : particleEntities){
		if (!usm.IsActive(entity)) continue;
		auto* layerComp{ registry.GetComponent<LayerComponent>(entity) };	// get layer component of entity
		if (layerComp != nullptr && !layerManager.IsVisibleOnCam(cameraMask, layerComp->layer))
			continue;	// skip entities rendered on layers that is not enabled on camera
		const auto& emitter{ *registry.GetComponent<ParticleEmitterComponent>(entity) };
		auto* transform{ registry.GetComponent<TransformComponent>(entity) };

		Vec2 particleScale{ emitter.startScale * ParticleSystem::PARTICLE_SCALE_MULTIPLIER };

		if (!emitter.useLocalPosition || !transform) {
			particle_xform = Mat3::Translation(emitter.emitterPosition.x, emitter.emitterPosition.y);
		}
		else {
			particle_xform = Mat3::Translation(transform->transform.m[6] + emitter.emitterPosition.x, 
					transform->transform.m[7] + emitter.emitterPosition.y);
		}

		switch (emitter.particleSpawnAreaShape) {
		case Shape::Box:
			particle_xform = particle_xform *
				Mat3::Scale(emitter.halfExtents.x * 2.f + particleScale.x, emitter.halfExtents.y * 2.f + particleScale.y);
			particle_xform.m[8] = 1.f;
			quadWireframeTransforms[particleQuadOffset + instanceCount[DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME]++] = particle_xform;
			break;
		case Shape::Circle:
			particle_xform = particle_xform * Mat3::Scale(emitter.radius + particleScale.x, emitter.radius + particleScale.y);
			particle_xform.m[8] = 1.f;
			circleWireframeTransforms[particleCircleOffset + instanceCount[DebugTypes::PARTICLE_SPAWNCIRCLE_WIREFRAME]++] = particle_xform;
			break;
		case Shape::Capsule: //note that capsules will still be rendered as boxes since we don't have a capsule wireframe, but we will still calculate the transform based on the capsule shape
			particle_xform = particle_xform *
				Mat3::Scale(emitter.halfExtents.x * 2.f + particleScale.x, emitter.halfExtents.y * 2.f + particleScale.y);
			particle_xform.m[8] = 1.f;
			quadWireframeTransforms[particleQuadOffset + instanceCount[DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME]++] = particle_xform;
			break;
		}
	}
	UpdateInstanceBuffer();	// update the instance buffer
}
#pragma endregion

void DebugRender::Free() {
	if (!init) return;

#ifdef _DEBUG
		LOGI("Cleaning DebugRender");

		LOGI("Cleaning quadWireframeInstance: [VAO id: %d] | [VBO id: %d]", quadWireframeInstance.vao.Id(), quadWireframeInstance.vbo.Id());
#endif
		quadWireframeInstance.Free();

#ifdef _DEBUG
		LOGI("Cleaning circleWireframeInstance: [VAO id: %d] | [VBO id: %d]", circleWireframeInstance.vao.Id(), circleWireframeInstance.vbo.Id());
#endif
		circleWireframeInstance.Free();

#ifdef _DEBUG
		LOGI("Cleaning arrowInstance: [VAO id: %d] | [VBO id: %d]", arrowInstance.vao.Id(), arrowInstance.vbo.Id());
#endif
		arrowInstance.Free();

		init = false;
}

void DebugRender::InitInstance(std::vector<Mat3>& transform, PrimitiveMesh& instanceMesh, DebugShapes types) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	transform.resize(instanceLimits[types]);
	instanceMesh.vbo.BufferUsage() = GL_DYNAMIC_DRAW;

	instanceMesh.vbo.SetBuffer(nullptr, instanceLimits[types] * sizeof(Mat3));

	// ============= Initialise instancee vao and vbo =============
	RenderBuffer& buffer{ types == DebugShapes::CIRCLE_WIREFRAME ? renderUtils.CircleVBO() : renderUtils.QuadVBO() };
	GLsizei stride{ static_cast<GLsizei>(types == DebugShapes::CIRCLE_WIREFRAME ? sizeof(GLfloat) * 2 : sizeof(GLfloat) * 4 )};

	instanceMesh.vao.SetAttribute(buffer, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_POSITION, 2, stride, 0, 0);

	// set up the VAO for the transforms in instance vbo
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM1,
		3, sizeof(Mat3), 0, 0);
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM2,
		3, sizeof(Mat3), 0, sizeof(GLfloat) * 3);
	instanceMesh.vao.SetAttribute(instanceMesh.vbo, RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM3,
		3, sizeof(Mat3), 0, sizeof(GLfloat) * 6);

	instanceMesh.vao.Bind();			// bind instance vao
	if (types == DebugShapes::QUAD_WIREFRAME)  renderUtils.BindQuadWireframeEBO();	// bind quad wireframe's EBO to collision instance's vao
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_POSITION, 0);	// use diff pos per vertex
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM1, 1);	// use the same transform per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM2, 1);	// use the same transform per instance
	glVertexAttribDivisor(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM3, 1);	// use the same transform per instance
	RenderAttributes::Unbind();			// unbind instanace vao
	if (types == DebugShapes::QUAD_WIREFRAME) RenderBuffer::Unbind(GL_ELEMENT_ARRAY_BUFFER); // unbind quad wireframe's EBO	
}

void DebugRender::UpdateInstanceBuffer() {
	// calculate the total size for each primitive by summing the counts in instanceCount
	GLsizei quadSize{ instanceCount[DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME] +
		instanceCount[DebugTypes::UI_QUAD_WIREFRAME] +
		instanceCount[DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME]
	}, circleSize{ instanceCount[DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME] + 
		instanceCount[DebugTypes::PARTICLE_SPAWNCIRCLE_WIREFRAME] 
	}, arrowSize{ instanceCount[DebugTypes::VELOCITY_ARROW] };
	// multiply size by sizeof the data
	quadSize *= sizeof(Mat3);
	circleSize *= sizeof(Mat3);
	arrowSize *= sizeof(Mat3);

	// update the instance buffer
	quadWireframeInstance.vbo.ConfigureBuffer(quadWireframeTransforms.data(),
		instanceLimits[DebugShapes::QUAD_WIREFRAME] * sizeof(Mat3), quadSize, 0);

	circleWireframeInstance.vbo.ConfigureBuffer(circleWireframeTransforms.data(),
		instanceLimits[DebugShapes::CIRCLE_WIREFRAME] * sizeof(Mat3), circleSize, 0);

	arrowInstance.vbo.ConfigureBuffer(arrowTransforms.data(),
		instanceLimits[DebugShapes::ARROW] * sizeof(Mat3), arrowSize, 0);
}

void DebugRender::ResizeInstanceBuffer(DebugShapes buffer, size_t size) {
	if (buffer >= DebugShapes::END_OF_DEBUG_SHAPES) return;
	if (instanceLimits[buffer] * 2 <= size) // if 2 * limit is still smaller than size
		instanceLimits[buffer] = static_cast<int>(size) * 2;	// set instance limit as 2 * size instead
	else instanceLimits[buffer] *= 2;		// otherwise, use 2 * instance Limit as new instance Limit
	switch (buffer) {			// depending on what primitive buffer, update the respective instance buffer
		case QUAD_WIREFRAME:
			quadWireframeTransforms.resize(instanceLimits[buffer]);
			quadWireframeInstance.vbo.SetBuffer(quadWireframeTransforms.data(), instanceLimits[buffer] * sizeof(Mat3));
			break;
		case CIRCLE_WIREFRAME:
			circleWireframeTransforms.resize(instanceLimits[buffer]);
			circleWireframeInstance.vbo.SetBuffer(circleWireframeTransforms.data(), instanceLimits[buffer] * sizeof(Mat3));
			break;
		case ARROW:
			arrowTransforms.resize(instanceLimits[buffer]);
			arrowInstance.vbo.SetBuffer(arrowTransforms.data(), instanceLimits[buffer] * sizeof(Mat3));
			break;
		default: break;
	}
}

void DebugRender::OffsetInstanceBuffer(PrimitiveMesh& instance, GLuint offset) {
	instance.vbo.Bind();
	// adjust the VAO based on the offset
	instance.vao.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM1,
		3, sizeof(Mat3), offset, 0);
	instance.vao.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM2,
		3, sizeof(Mat3), offset, sizeof(GLfloat) * 3);
	instance.vao.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM3,
		3, sizeof(Mat3), offset, sizeof(GLfloat) * 6);
	instance.vbo.Unbind(instance.vbo.BufferType());
}