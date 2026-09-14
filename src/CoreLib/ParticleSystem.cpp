/*!
@file       ParticleSystem.cpp
@author     Tan Jun Jie (t.junjie) 100%
@date       07/01/2026 (DD/MM/YYYY)
@brief		Handles the creation, updating, and rendering of particles.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "ParticleSystem.h"

Particle::Particle(const Color& startCol, const Color& endCol, const Vec2& pos,
	const Vec2& startScale, const Vec2& endScale, const Vec2& vel, uint32_t id, float rot, float lifespan, bool _isEmissive) :
	transform(), color(startCol), colorDelta{ 0.f, 0.f, 0.f, 0.f }, position(pos),
	scale(startScale), scaleDelta(0.f, 0.f), velocity(vel), emitterId(id),
	rotation(rot), lifetime(lifespan), timeElapsed(0.f), isEmissive(_isEmissive) {

	float divisor{ 1.f / lifespan };
	// Pre-calculate the delta for colour and scale	
	scaleDelta = (endScale - startScale) * divisor;
	colorDelta = {
		(endCol.r - startCol.r) * divisor,
		(endCol.g - startCol.g) * divisor,
		(endCol.b - startCol.b) * divisor,
		(endCol.a - startCol.a) * divisor
	};
}

ParticleSystem& ParticleSystem::Instance() {
	static ParticleSystem instance;
	return instance;
}

void ParticleSystem::Init() {
	if (initialized) return;
	TextureObj* tex{ &CEO::Instance().GetManager<ResourceManager>()->GetErrorTex() };

	particleLimit[tex] = CEO::Instance().Get<RenderUtils>()->GetParticleInstanceLimit();
	particles[tex].first.resize(particleLimit[tex]);	// pre-reserve memory for particles
	particles[tex].second = 0;
	particleInstance[tex].resize(particleLimit[tex]);
	

	// Clear all particles when scene changes
	CEO::Instance().GetManager<EventsDispatcher>()->Subscribe<Events::SceneChanged>([](auto const&)
		{
			CEO::Instance().GetManager<ParticleSystem>()->ClearAllParticles();
		}
	);

	initialized = true;
}

void Particle::Update(float dt) {
	timeElapsed += dt;
	position += velocity * dt;	// translate particle position with velocity

	color = {	// calculate new interpolated colour ( i need an arithmetic operator overload for color :(
		color.r + colorDelta.r * dt,
		color.g + colorDelta.g * dt,
		color.b + colorDelta.b * dt,
		color.a + colorDelta.a * dt
	};

	scale += scaleDelta * dt;	// calculate new interpolated scale

	transform =
		Mat3::Translation(position.x, position.y) *
		Mat3::Rotation(rotation) *
		Mat3::Scale(scale.x, scale.y);
}

void ParticleSystem::Update(Registry& registry, float dt) {
	UpdateParticles(registry, dt);		// update particles on cpu side
	UpdateInstance(registry);			// update particles on gpu side
}

void ParticleSystem::UpdateParticles(Registry& registry, float dt) {
	uint64_t cameraMask{ CEO::Instance().GetManager<CameraManager>()->GetMainCamMask() };
	LayerManager& layerManager{ *CEO::Instance().GetManager<LayerManager>() };

	// Loop through particle emitters
	for (auto entity : registry.GetEntitiesWithComponents<ParticleEmitterComponent>()) {
		auto& emitter{ *registry.GetComponent<ParticleEmitterComponent>(entity) };	// get the particle emitter
		LayerComponent* layerComp{ registry.GetComponent<LayerComponent>(entity) };
		TransformComponent* transform{ registry.GetComponent<TransformComponent>(entity) };
		ActiveComponent* active{ registry.GetComponent<ActiveComponent>(entity) };

		// skip if emitter is disabled or if current layer is not visible on camera
		if (!emitter.enabled || !active->isActiveSelf || !active->isActiveInHierarchy || layerComp != nullptr && !layerManager.IsVisibleOnCam(cameraMask, layerComp->layer))
			continue;

		emitter.timeElapsed += dt;

		if (emitter.timeElapsed >= emitter.duration) {
			if (emitter.looping) emitter.timeElapsed = 0.f;
			else { // if emitter reached duration and looping is not enabled, disable emitter
				emitter.enabled = false;
				emitter.timeElapsed = emitter.spawnAccumulator = 0.f;
				continue;
			}
		}

		// skip if the current amount of particles alive reaches the max allowed particles
		if (emitter.aliveParticles >= emitter.maxParticles) continue;
		if (emitter.emissionRateRange.x > emitter.emissionRateRange.y) emitter.emissionRateRange.x = emitter.emissionRateRange.y;
		// calculate how many particles to spawn in current frame
		float particlesPerFrame{ RandomFloat(emitter.emissionRateRange.x, emitter.emissionRateRange.y) * dt };
		uint32_t newParticles{ static_cast<uint32_t>(particlesPerFrame) };
		emitter.spawnAccumulator += particlesPerFrame - newParticles;
		while (emitter.spawnAccumulator >= 1.f) {
			newParticles += 1;
			emitter.spawnAccumulator -= 1.f;
		}

		// Get the number of particles to spawn (limited by range [particles alive, max of allowed particles]
		uint32_t spawnCount{ std::min(newParticles, emitter.maxParticles - emitter.aliveParticles) };
		if (spawnCount == 0) continue;	// if its 0, skip to next loop

		auto& particleVec{ particles[emitter.texture].first };		// for ease of access to vector of particles
		auto& particleAlive{ particles[emitter.texture].second };	// for ease of access to vector that tracks the number of particles alive
		
		if (spawnCount + particleAlive > particleLimit[emitter.texture])	// resize buffer if limit is reached / exceeded
			ResizeParticleBuffer(emitter.texture, spawnCount + particleAlive);

		// capping min value of range to max value
		if (emitter.minSpeed > emitter.maxSpeed) emitter.minSpeed = emitter.maxSpeed; 
		if (emitter.rotationRange.x > emitter.rotationRange.y) emitter.rotationRange.x = emitter.rotationRange.y;

		float dirRad{ ToRad(emitter.direction) };			// convert direction to radians
		float spreadRad{ ToRad(emitter.spread) };			// covert the spread to radians
		Vec2 randVel{},										// to store the random velocity
			emitterWorldPos{ (!emitter.useLocalPosition || !transform) ? emitter.emitterPosition :
				emitter.emitterPosition + Vec2(transform->transform.m[6], transform->transform.m[7]) };	// world position of emitter

		// create the particles and increment the num of particles alive
		uint32_t spawned{};	// to track how many particles are being spawned
		for (; spawned < spawnCount; ++spawned) {	// iterate through count and create that many particles
			Particle& particle{ particleVec[particleAlive++] };

			// calculate the random values
			Vec2 randPos{ emitterWorldPos };		// set initial value for random position to start at emitter world position
			float randRot{ RandomFloat(emitter.rotationRange.x, emitter.rotationRange.y) },			// calculate a randomise rotation for the particle
				randLifetime{ RandomFloat(emitter.particleLifetimeRange.x, emitter.particleLifetimeRange.y) },		// calculate a randomised life time for particle
				randSpeed{ RandomFloat(emitter.minSpeed, emitter.maxSpeed) };		// calculate a randomised speed

			if (spreadRad == 0.f) randVel = Vec2(cosf(dirRad), sinf(dirRad));	// if spread is 0, it will just be a direct beam at direction
			else {	// else rotate direction vector by a random value in spread
				float angle{ dirRad + RandomFloat(-spreadRad * 0.5f, spreadRad * 0.5f) };	// calculate the angle to rotate by in range [-spreadRad/2, +spreadRad/2]
				randVel = Vec2(cosf(angle), sinf(angle));		// rotate by a random angle along direction and spread
			}
			randVel *= randSpeed;	// scale velocity by randomised speed

			// if spawn area is Box, position will be randomised by the half extents
			if (emitter.particleSpawnAreaShape == Shape::Box) randPos += RandomVec2(-emitter.halfExtents, emitter.halfExtents);
			else if (emitter.particleSpawnAreaShape == Shape::Circle) {	// if spawn area is Circle, randomise position by radius
				Vec2 p{};
				do {
					p.x = RandomFloat(-1.f, 1.f);
					p.y = RandomFloat(-1.f, 1.f);
				} while (p.x * p.x + p.y * p.y > 1.f);
				randPos += p * emitter.radius;
			}
			else if (emitter.particleSpawnAreaShape == Shape::Capsule) {
				randPos += RandomVec2(-emitter.halfExtents, emitter.halfExtents);
			}
			// Create the particle
			particle = {
				emitter.startColor,								// the starting colour (for the particle to be spawned as)
				emitter.endColor,								// the end colour (for the particle to end on)
				randPos,										// the position to create the particle at
				emitter.startScale * PARTICLE_SCALE_MULTIPLIER, // starting scale of particle
				emitter.endScale * PARTICLE_SCALE_MULTIPLIER,	// ending scale of particle
				randVel,										// the velocity of the particle
				entity,											// the ID (entity ID) of the emitter the particle belongs to
				randRot,										// the rotation value of the particle
				randLifetime,									// the lifetime of the particle
				emitter.isEmmissive
			};
		}

		emitter.aliveParticles += spawned;	// increment number particles alive for this emitter by spawned amount
	}

	for (auto& [tex, particleBatch] : particles) {		// loop through each particle instance batch
		auto& particleVec{ particleBatch.first };		// container of particles for this batch
		auto& particlesAlive{ particleBatch.second };	// the number of particles alive for this battch

		for (size_t i{}; i < particlesAlive;) {			// Loop through the particles and update them
			Particle& p{ particleVec[i] };

			if (p.timeElapsed + dt >= p.lifetime) {	// if particle reaches or exceeds its lifetime
				// remove the particle
				--registry.GetComponent<ParticleEmitterComponent>(p.emitterId)->aliveParticles;
				particleVec[i] = particleVec[(particlesAlive--) - 1];
				continue;
			}
			
			p.velocity += registry.GetComponent<ParticleEmitterComponent>(p.emitterId)->acceleration * dt;
			p.Update(dt);	// update the particle
			++i;
		}
	}
}

void ParticleSystem::Draw(GLSLShader& shdr) const {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	shdr.Use();
	shdr.SetUniform("uTex2d", 0);
	shdr.SetUniform("uViewProj", CEO::Instance().GetManager<CameraManager>()->GetViewProjection());
	TextureObj* nullTex{ &CEO::Instance().GetManager<ResourceManager>()->GetErrorTex() };	// this texture is used to signify the particle is not using a texture
	for (auto& [tex, particleBatch] : particles) {	// loop through each particle instance batch
		auto& particlesAlive{ particleBatch.second };	
		if (particlesAlive == 0) continue;	// if there are no particles alive for this batch, skip

		if (tex != nullTex) {			// check if this batch uses texture
			// set param for texture rendering
			shdr.SetUniform("uUseTex", GL_TRUE);
			tex->BindTexture();
			tex->ApplyBlend();
		}
		else shdr.SetUniform("uUseTex", GL_FALSE);

		renderUtils.BindParticleInstanceVAO(tex);		// bind the particle instance vao for this batch
		renderUtils.RenderInstanced(static_cast<GLsizei>(particlesAlive));	// render this particle instance batch

		if (tex->IsUseBlend()) glDisable(GL_BLEND);		// disable blend if enabled
	}
	RenderAttributes::Unbind();				// unbind all vao
	shdr.UnUse();
}

void ParticleSystem::ClearAllParticles() { 
	Registry& registry{ *CEO::Instance().GetManager<Registry>() };
	for (auto entity : registry.GetEntitiesWithComponent<ParticleEmitterComponent>()) {
		auto& emitter{ *registry.GetComponent<ParticleEmitterComponent>(entity) };
		emitter.aliveParticles = 0;
		emitter.timeElapsed = emitter.spawnAccumulator = 0.f;
	}

	for (auto& [tex, particleBatch] : particles) {	// loop through each particle instance batch
		particleBatch.second = 0;	// reset the tracker for number of particles alive
	}
}

void ParticleSystem::ClearParticles(TextureObj* tex) {
	if (particles.find(tex) == particles.end()) return;

	particles[tex].second = 0;
}

void ParticleSystem::EnableEmitter(Registry::Entity emitterId) {
	Registry& registry{ *CEO::Instance().GetManager<Registry>() };					// get the entity registry
	auto* emitter{ registry.GetComponent<ParticleEmitterComponent>(emitterId) };	// get the particle emitter component
	if (emitter == nullptr || emitter->enabled) return;		// if emitter doesn't exist or already enabled, skip
	emitter->enabled = true;								// enable the particle emitter
}

void ParticleSystem::DisableEmitter(Registry::Entity emitterId) {
	Registry& registry{ *CEO::Instance().GetManager<Registry>() };					// get the entity registry
	auto* emitter{ registry.GetComponent<ParticleEmitterComponent>(emitterId) };	// get the particle emitter component
	if (emitter == nullptr || !emitter->enabled) return;	// if emitter doesn't exist or already disabled, skip

	emitter->enabled = false;								// disable the particle emitter
	emitter->timeElapsed = emitter->spawnAccumulator = 0.f;	// reset counters
}

void ParticleSystem::UpdateInstance(Registry& registry) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	float normalisedDepthDivisor{ 1.f / renderUtils.GetMaxRenderDepth() };    // pre-calculate the divisor needed to normalise depth

	// Loop through each particle batch
	for (auto& [tex, particleBatch] : particleInstance) {
		const auto& particlesAlive{ particles[tex].second };	// number of particles alive in this batch
		for (size_t i{}; i < particlesAlive; ++i) {			// Loop through to update the GPU-side particles buffer for this batch
			const Particle& p{ particles[tex].first[i] };
			LayerComponent* layerComp{ registry.GetComponent<LayerComponent>(p.emitterId) };
			particleBatch[i].xform = p.transform;
			particleBatch[i].xform.m[8] = layerComp != nullptr ? layerComp->renderPriority * normalisedDepthDivisor : 1.f;     // normalise depth value between [0.f, 1.f]
			particleBatch[i].sSize = { 1.f, 1.f };
			particleBatch[i].color = p.color;
			particleBatch[i].emissiveState = p.isEmissive;
		}
		renderUtils.UpdateParticleInstanceBuffer(particleBatch, tex, particleLimit[tex], particlesAlive);	// Update this particleInstance batch to GPU
	}
}

void ParticleSystem::ResizeParticleBuffer(TextureObj* tex, size_t size) {
	auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
	if (particleLimit[tex] == 0) { // if limit is 0 means a new buffer needs to be created
		renderUtils.UpdateParticleInstanceBuffer(particleInstance[tex], tex, 
			particleLimit[tex], particles[tex].second);
		particleLimit[tex] = renderUtils.GetParticleInstanceLimit();
	}
	else if (particleLimit[tex] * 2 <= size)		// if 2 * limit is still smaller than size
		particleLimit[tex] = size * 2;		// use 2 * size as new particle instance limit
	else particleLimit[tex] *= 2;			// otherwise, use 2 * instance limit as new particle instance limit

	particles[tex].first.resize(particleLimit[tex]);
	particleInstance[tex].resize(particleLimit[tex]);	// resize the particleInstance vector

	// update the particle instance GPU buffer
	renderUtils.GetParticlesInstanceVBO(tex).SetBuffer(particleInstance[tex].data(),
		particleLimit[tex] * sizeof(InstanceData));
}

Vec2 ParticleSystem::RandomVec2(const Vec2& min, const Vec2& max) {
	std::uniform_real_distribution<float> randX{ min.x, max.x };
	std::uniform_real_distribution<float> randY{ min.y, max.y };
	return { randX(rng), randY(rng) };
}

float ParticleSystem::RandomFloat(float min, float max) {
	if (min > max)
		std::swap(min, max);
	std::uniform_real_distribution<float> rand{ min, max };
	return rand(rng);
}