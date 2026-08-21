/*!
@file       GraphicsSystem.cpp
@author     Tan Jun Jie (t.junjie)  70%
@author	    Ou Yukang (yukang.ou)   30%
@date       25/09/2025
@brief		Handles all the drawing / rendering for the game. Draws all
			entities and UI entities, minimap, and applies post processing
			(if applicable).

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "GraphicsSystem.h"
#include <pch.h>
#include "DebugRender.h"
#include "ParticleSystem.h"
#include "UpdateStackManager.h"
#include "MapManager.h"
#include "PersistentDataManager.h"
#include "UserSettingsManager.h"

static bool init{ false }; // To make sure this only runs once
void GraphicsSystem::Init() {
    if(!init) {
#ifdef _DEBUG
        LOGI("Initializing GraphicsManager");
#endif
        CEO::Get<RenderUtils>()->Init();                        // initialise render utils

        batchOffSets.reserve(32);                    // allocate some memory for batchOffsets
        xforms.resize(CEO::Get<RenderUtils>()->GetEntityInstanceLimit());  // set the size of xforms to instanceLimit
#ifdef EditorFlag
        CEO::Get<DebugRender>()->Init();    // initialise debug render
#endif
        CEO::Get<EventsDispatcher>()->Subscribe<Events::SceneChanged>([](const auto&) {
            CEO::Get<PersistentDataManager>()->Set("MinimapCamEnt", 0);
            CEO::Get<PersistentDataManager>()->Set("MinimapEnt", 0);
            CEO::Get<PersistentDataManager>()->Set("IsMinimapCamDirty", true);
            });

        UserSettingsManager& settings = *CEO::Get<UserSettingsManager>();
        if (settings.settings.HasMember("postProcessFlag"))
            postProcessFlag = settings.settings["postProcessFlag"].GetBool();

        // event for toggling postProcessFlag
        CEO::Get<EventsDispatcher>()->Subscribe<Events::TogglePostProcessEvent>([this](Events::TogglePostProcessEvent event) { postProcessFlag = event.isPostProcessOn; });
        init = true;
#ifdef _DEBUG
        LOGI("GraphicsManager initialized");
#endif
    }
}

void GraphicsSystem::Draw(Registry& registry) {
#ifdef EditorFlag
    DebugRender& debugRenderer{ *CEO::Instance().GetManager<DebugRender>() };
#endif
    // clear colorbuffer with RGBA value in glClearColor ...
    glClearColor(clear_colour[0], clear_colour[1], clear_colour[2], clear_colour[3]);
    glClearDepthf(0.f);      // set the value to clear the depth buffer with
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear the colour & depth buffer
    // get the shaders to use for rendering
    GLSLShader& shader{ CEO::Instance().GetManager<ResourceManager>()->GetShader("sprite") };              // non-instance shader
    GLSLShader& instancedShader{ CEO::Instance().GetManager<ResourceManager>()->GetShader("instance") };   // instance shader
    shader.Use();       // use the non-instance shader

    auto postProcess = registry.GetEntitiesWithComponent<PostProcessComponent>();
    bool isUIVignette{ postProcess.size() > 0  ? registry.GetComponent<PostProcessComponent>(postProcess[0])->isUIVignette  : false};

    if(useTexAsBackground) DrawBackground(shader);  // if drawing background as texture enabled
    if (!instancingFlag) {  // check if instancing
        DrawEntities(shader);                       // draw active world entities
        CEO::Get<ParticleSystem>()->Draw(instancedShader);
        if (postProcessFlag)
        {
			LightingPass(registry);
			Bloom(postProcess);
        }
        if (!isUIVignette)
        {
            // only write to color buffer, prevents writing into entity ID buffer
            GLenum bufs[1] = { GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, bufs);
            Vignette(postProcess);
            ScreenManager::screenBuffer.Bind();
        }
        DrawUIEntities(registry, shader);           // draw active UI entities
        if (isUIVignette)  {
            // only write to color buffer, prevents writing into entity ID buffer
            GLenum bufs[1] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, bufs);
            Vignette(postProcess);
            ScreenManager::screenBuffer.Bind();
        }
#ifdef EditorFlag
        debugRenderer.DrawDebug(shader);            // draw debug (if enabled)
#endif
        shader.UnUse();                             // unuse the non-instance shader
    }
    else {
        DrawEntitiesInstanced(instancedShader);                 // draw active world entities with instancing
        CEO::Instance().GetManager<ParticleSystem>()->Draw(instancedShader);
        if (postProcessFlag)
        {
			LightingPass(registry);
			Bloom(postProcess);
        }
        if (!isUIVignette)
        {
            // only write to color buffer, prevents writing into entity ID buffer
            GLenum bufs[1] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, bufs);
            Vignette(postProcess);
            ScreenManager::screenBuffer.Bind();
        }        
        DrawUIEntities(registry, shader);                       // draw active UI entities (non-instance)
        if (isUIVignette) {
            // only write to color buffer, prevents writing into entity ID buffer
            GLenum bufs[1] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, bufs);
            Vignette(postProcess);
            ScreenManager::screenBuffer.Bind();
        }        
#ifdef EditorFlag
        debugRenderer.DrawDebugInstanced(instancedShader);      // draw debug (if enabled) with instancing
#endif
    }
}

void GraphicsSystem::Update(Registry& registry) {
    normalisedDepthDivisor =  1.f / CEO::Get<RenderUtils>()->GetMaxRenderDepth();    // pre-calculate the divisor needed to normalise depth

    UpdateInstancedEntitiesTransform(registry);         // update InstanceData buffer with entities' info
#ifdef EditorFlag
    CEO::Get<DebugRender>()->UpdateInstancedTransform(registry);    // update transform buffer for DebugRender
#endif
}

void GraphicsSystem::UpdateInstancedEntitiesTransform(Registry& registry) {
    auto& renderUtils{ *CEO::Get<RenderUtils>() };
    auto& camManager{ *CEO::Get<CameraManager>() };

    Vec2 camPos{ camManager.GetPosition() }, vpSize{camManager.GetViewportSize() }; // get camera's (world) position (for view culling)
    float vpScale{ std::max(vpSize.x, vpSize.y) };
    Vec2 camHalfExtent{ vpSize * 0.5f }, camPaddedHalfExtent{vpScale, vpScale};     // get the cam's view port half extent

    uint64_t cameraMask{ camManager.GetMainCamMask()};
    LayerManager& layerManager{ *CEO::Get<LayerManager>() };

    UpdateStackManager& usm{ *CEO::Get<UpdateStackManager>() };

    // Get the opaque entities and tilemap entities
    auto entities{ layerManager.GetEntitiesVisibleWithComponents<TransformComponent, SpriteRendererComponent>(cameraMask, false) };
    auto tilemaps{ layerManager.GetEntitiesVisibleWithComponent<TilemapComponent>(cameraMask, false) };

    // get the translucent / transparent entities and tilemap entities
    auto transparentEnt{ layerManager.GetEntitiesVisibleWithComponents<TransformComponent, SpriteRendererComponent>(cameraMask, true) };
    auto transparentTilemaps{ layerManager.GetEntitiesVisibleWithComponent<TilemapComponent>(cameraMask, true) };

    batchOffSets.clear();       // clear offsets

    // calculate instances needed for tilemaps
    auto countTileChunks = [&](const auto& tmList) {
        size_t n{};
        for (auto tilemapEnt : tmList) n += registry.GetComponent<TilemapComponent>(tilemapEnt)->chunks.size();
        return n;
    };

    size_t instanceCount = entities.size() + transparentEnt.size() +
        countTileChunks(tilemaps) + countTileChunks(transparentTilemaps) + TilemapComponent::CHUNK_SIZE * TilemapComponent::CHUNK_SIZE;

    if(instanceCount >= renderUtils.GetEntityInstanceLimit()) ResizeInstanceBuffer(instanceCount);  // if active entities exceed instance limit, increase instance buffer size

    int count{};                // count to track how many entities to draw per texture batch
    TextureObj* prev{ nullptr };
    TextureObj* nullTex{ &CEO::Get<ResourceManager>()->GetErrorTex()};
    // Lambda function for processing tilemap entities and update instance buffer. (reused for both opaque and transparent tilemap entities)
    auto processTilemaps = [&](const auto& tmList) {
        const float halfChunkSize{ TilemapComponent::CHUNK_SIZE * 0.5f };
        for (auto tilemapEnt : tmList) {
            if (!usm.IsActive(tilemapEnt)) continue;    // skip if not active

            TilemapComponent& tilemap{ *registry.GetComponent<TilemapComponent>(tilemapEnt) };
            if (!tilemap.tileset) continue;             // no tileset, go next

            auto& layerComp{ *registry.GetComponent<LayerComponent>(tilemapEnt) };
            TransformComponent& transform{ *registry.GetComponent<TransformComponent>(tilemapEnt) };    // get tilemap's transform
            const Vec2 tilemapPos{ transform.transform.m[6], transform.transform.m[7] },
                chunkSize{ transform.scale.x * TilemapComponent::CHUNK_SIZE, transform.scale.y * TilemapComponent::CHUNK_SIZE };

            if (tilemap.tileset->texture != prev) {     // check if current tilemap is the start of a new batch (has diff texture)
                batchOffSets.emplace_back(tilemap.tileset->texture, nullTex, count); // add the current texture and count to get batch info
                prev = tilemap.tileset->texture;        // update prev texture
            }
            for (auto& chunk : tilemap.chunks) {
                Vec2 chunkPos{
                    tilemapPos.x + chunk.first.chunkX * TilemapComponent::CHUNK_SIZE + halfChunkSize,
                    tilemapPos.y + chunk.first.chunkY * TilemapComponent::CHUNK_SIZE + halfChunkSize
                };
               
                if (!IsInView(chunkPos, chunkSize, camPos, camPaddedHalfExtent)) continue;  // skip if not within camera's view

                for (int row = 0; row < TilemapComponent::CHUNK_SIZE; ++row) {
                    for (int col = 0; col < TilemapComponent::CHUNK_SIZE; ++col) {
#ifdef EditorFlag
                        if (chunk.second[row * TilemapComponent::CHUNK_SIZE + col].spriteID >= tilemap.tileset->sprites.size())
                            chunk.second[row * TilemapComponent::CHUNK_SIZE + col].spriteID = -1;
#endif
                        int spriteID = chunk.second[row * TilemapComponent::CHUNK_SIZE + col].spriteID;
                        if (spriteID == -1) continue;

                        const SpriteInfo& spriteInfo{ tilemap.tileset->sprites[spriteID] };
                        xforms[count].xform = transform.transform * Mat3::Translation(chunk.first.chunkX * TilemapComponent::CHUNK_SIZE + col + 0.5f,
                            chunk.first.chunkY * TilemapComponent::CHUNK_SIZE + row + 0.5f);
                        xforms[count].xform.m[8] = layerComp.renderPriority * normalisedDepthDivisor;     // normalise depth value between [0.f, 1.f]
                        xforms[count].sOrigin = spriteInfo.min;
                        xforms[count].sSize = spriteInfo.max - spriteInfo.min;
                        xforms[count].id = tilemapEnt;
                        xforms[count].color = Color{ 1.f, 1.f, 1.f, 1.f };
                        xforms[count].sTile = Vec2{ 1.f, 1.f };
                        xforms[count].emissiveState = 0;
                        ++count;        // increment count
                    }
                }
            }
        }
    };
    // Lambda function for processing entities and update instance buffer. (reused for both opaque and transparent entities)
    auto processEntities = [&](const auto& entList) {
        for (auto ent : entList) {
            if (!usm.IsActive(ent)) continue;   // skip if not active

            auto& renderer{ *registry.GetComponent<SpriteRendererComponent>(ent) };
            if (!renderer.visible) continue;    // skip entities that are not visible or rendered on layers that is not enabled on camera

            auto& transform{ *registry.GetComponent<TransformComponent>(ent) };     // get entity's transform
            if (!IsInView(transform.transform, camPos, camHalfExtent)) continue;    // skip if entity is not in camera's view

            auto& layerComp{ *registry.GetComponent<LayerComponent>(ent) };

            if (renderer.texture != prev) {     // check if current entity is the start of a new batch (has diff texture)
                batchOffSets.emplace_back(renderer.texture,renderer.emissiveTexture, count); // add the current texture and count to get batch info
                prev = renderer.texture;        // update prev texture
            }

            // fill xforms container with instance data
            xforms[count].xform = transform.transform;
            xforms[count].xform.m[8] = layerComp.renderPriority * normalisedDepthDivisor;     // normalise depth value between [0.f, 1.f]
            xforms[count].sOrigin = renderer.start;
            xforms[count].sSize = renderer.size;
            xforms[count].id = ent;
            xforms[count].color = renderer.color;
            xforms[count].sTile = renderer.tile;
            xforms[count].emissiveState = renderer.isEmissive;
            if (renderer.isEmissive && renderer.emissiveTexture != nullTex)
            {
                xforms[count].emissiveState = SpriteRendererComponent::EMISSIVE_TEXTURED;
            }
            ++count;        // increment count
        }
    };
    // Rendering order: opaque tilemaps -> opaque entities -> translucent / transparent tilemaps -> translucent / transparent entities
    processTilemaps(tilemaps);              // update instance buffer with opaque tilemap entities
    processEntities(entities);              // update instance buffer with opaque entities
    processTilemaps(transparentTilemaps);   // update instance buffer with transparent / translucent tilemap entities
    processEntities(transparentEnt);        // update instance buffer with transparent / translucent tilemap entities

    batchOffSets.emplace_back(nullptr,nullptr, count);                          // terminate the batchOffsets
    renderUtils.UpdateEntityInstanceBuffer(xforms, batchOffSets.back().offset); // update the instance buffer with data in xforms
}

void GraphicsSystem::DrawEntitiesInstanced(GLSLShader& shader) {
    if(batchOffSets.size() <= 1) return;    // if there are no active entities, skip drawing
    auto& renderUtils{ *CEO::Get<RenderUtils>() };
    shader.Use();
    shader.SetUniform("uTex2d", 0);                 // set entity texture unit as 0
    shader.SetUniform("uEmmissionTex2d", 1);        // set emission texture unit as 1
    shader.SetUniform("uUseTex", GL_TRUE);          // set use texture flag to true
    shader.SetUniform("uUseDebugColor", GL_FALSE);  // set use debug colour flag to false
    shader.SetUniform("uViewProj", CEO::Get<CameraManager>()->GetViewProjection()); // upload camera's view projection

    RenderAttributes& instanceVAO{ renderUtils.GetEntityInstanceVAO() }; // get the instance VAO from RenderUtils
    RenderBuffer& instanceVBO{ renderUtils.GetEntitynstanceVBO() };     // get the instance buffer from RenderUtils
    uintptr_t offset{};                     // to calculate the batch offset in bytes

    instanceVAO.Bind();     // bind instance vao
    instanceVBO.Bind();     // bind instance buffer
    for (size_t i{}; i < batchOffSets.size() - 1; ++i) {    // increment through the batches to render
        GLuint count{ batchOffSets[i + 1].offset - batchOffSets[i].offset };   // calculate how many entities to render in this instance
        
        glActiveTexture(GL_TEXTURE0 + 1);               // Texture unit 1 (emissive texture)
        batchOffSets[i].emissiveTexture->BindTexture(); // bind the emissive texture
        glActiveTexture(GL_TEXTURE0);                   // Texture unit 0 (entity's texture)
        batchOffSets[i].texture->BindTexture();         // bind the texture of the current instance
        batchOffSets[i].texture->ApplyBlend();          // Apply blend for this instance if texture requires

        offset = batchOffSets[i].offset * sizeof(InstanceData); // calculate the byte offset

        // adjust the VAO based on the offset (to proceed to next instance batch for rendering)
        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM1,
            3, sizeof(InstanceData), offset, offsetof(InstanceData, xform));
        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM2,
            3, sizeof(InstanceData), offset, offsetof(InstanceData, xform) + sizeof(GLfloat) * 3);
        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TRANSFORM3,
            3, sizeof(InstanceData), offset, offsetof(InstanceData, xform) + sizeof(GLfloat) * 6);

        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_SPRITE_ORIGIN,
            2, sizeof(InstanceData), offset, offsetof(InstanceData, sOrigin));
        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_SPRITE_SIZE,
            2, sizeof(InstanceData), offset, offsetof(InstanceData, sSize));

        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_ID,
            1, sizeof(InstanceData), offset, offsetof(InstanceData, id), GL_UNSIGNED_INT);

        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_COLOR,
            4, sizeof(InstanceData), offset, offsetof(InstanceData, color));
        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_TILE,
            2, sizeof(InstanceData), offset, offsetof(InstanceData, sTile));
        instanceVAO.SetAttribute(RenderAttributes::ATTRIBUTE_INDEX::ATTRIB_EMISSION,
            1, sizeof(InstanceData), offset, offsetof(InstanceData, emissiveState),GL_INT);

        renderUtils.RenderInstanced(count);    // render the instance

        if (batchOffSets[i].texture->IsUseBlend()) glDisable(GL_BLEND);   // disable blend if it was enabled
    }
    instanceVBO.Unbind(instanceVBO.BufferType());   // unbind instance buffer
    instanceVAO.Unbind();                           // unbind instance vao
    shader.UnUse();
}

void GraphicsSystem::DrawEntities(GLSLShader& shader) {
    if (batchOffSets.size() <= 1) return;    // if there are no active entities, skip drawing
    auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
    const Mat3 viewProj{ CEO::Instance().GetManager<CameraManager>()->GetViewProjection()};

    shader.SetUniform("uTex2d", 0);             // set entity texture unit as 0
    shader.SetUniform("uEmmissionTex2d", 1);    // set emissive texture unit as 1
    shader.SetUniform("uUseTex", GL_TRUE);      // set use texture flag to true

    renderUtils.BindQuadVAO();         // bind quad vao first (to prevent constant bind and unbinding)
    for (size_t i{}; i < batchOffSets.size() - 1; ++i) {    // iterate through each entity batch
        glActiveTexture(GL_TEXTURE0 + 1);                   // Texture unit 1 (for emissive texture)
        batchOffSets[i].emissiveTexture->BindTexture();     // bind the emissive texture
        glActiveTexture(GL_TEXTURE0);                       // Texture unit 1 (for entity's texture)
        batchOffSets[i].texture->BindTexture();             // bind the texture of the current instance
        //batchOffSets[i].texture->ApplyBlend();    // Apply blend for this instance if texture requires
        for (size_t j{ batchOffSets[i].offset }; j < batchOffSets[i + 1].offset; ++j) {    // iterate through the number of entities in current batch to render
            Mat3 xform{ xforms[j].xform };          // extract the entity transform
            float normalisedDepth{ xform.m[8] };    // extract the depth value saved inside transform matrix
            xform.m[8] = 1.f;                       // reset depth value holder in transform matrix back to 1
            xform = viewProj * xform;

            shader.SetUniform("uEntityID", xforms[j].id);           // pass in the entity id for object picking
            shader.SetUniform("uColour", xforms[j].color);          // set color modulation
            shader.SetUniform("spriteOrigin", xforms[j].sOrigin);   // set spritesheet origin for spritesheet animation
            shader.SetUniform("spriteSize", xforms[j].sSize);       // set spritesheet size for spritesheet animation
            shader.SetUniform("tile", xforms[j].sTile);             // set spritesheet size for spritesheet animation
            shader.SetUniform("mdl_to_ndc", xform);                 // pass entity transform into shader
            shader.SetUniform("ndcDepth", normalisedDepth);         // pass normalised depth into shader
            shader.SetUniform("uEmissionState", xforms[j].emissiveState);   // emission state

            renderUtils.RenderQuadRaw();   // render the quad
        }
        //if(batchOffSets[i].texture->IsUseBlend()) glDisable(GL_BLEND);    // disable blend if blend was applied
    }
    RenderAttributes::Unbind();             // unbind the quad vao
}

void GraphicsSystem::DrawUIEntities(Registry& registry, GLSLShader& shader) {
    // get all the entities that has Sprite Renderer component
    auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };
    LayerManager& layerManager{ *CEO::Instance().GetManager<LayerManager>() };
    UpdateStackManager* usm = CEO::Get<UpdateStackManager>();
    uint64_t cameraMask{ CEO::Instance().GetManager<CameraManager>()->GetMainCamMask()};

    // Get the opaque UI entities
    auto uiEntities{ layerManager.GetEntitiesVisibleWithComponents<SpriteRendererComponent, UITransformComponent>(cameraMask, false) };
    auto worldTextEntities{ layerManager.GetEntitiesVisibleWithComponents<TextRendererComponent, TransformComponent>(cameraMask, false) };
    auto uiTextEntities{ layerManager.GetEntitiesVisibleWithComponents<TextRendererComponent, UITransformComponent>(cameraMask, false) };

    // Get the translucent and transparent UI entities
    auto transparentUIEntities{ layerManager.GetEntitiesVisibleWithComponents<SpriteRendererComponent, UITransformComponent>(cameraMask, true) };
    auto transparentWorldTextEntities{ layerManager.GetEntitiesVisibleWithComponents<TextRendererComponent, TransformComponent>(cameraMask, true) };
    auto transparentUITextEntities{ layerManager.GetEntitiesVisibleWithComponents<TextRendererComponent, UITransformComponent>(cameraMask, true) };

    Mat3 const& projection{ CEO::Instance().GetManager<CameraManager>()->GetProjection() };
    Mat3 const& viewProjection{ CEO::Instance().GetManager<CameraManager>()->GetViewProjection() };

    // For drawing opaque UI entities
    auto drawUIEntities = [&](const auto& tUIEntList, auto& shdr) {
        for (auto entity : tUIEntList) {
            if (!usm->IsActive(entity)) continue;

            auto& renderer{ *registry.GetComponent<SpriteRendererComponent>(entity) };
            if (!renderer.visible) continue;    // skip entities rendered on layers that is not enabled on camera

            auto& transform{ *registry.GetComponent<UITransformComponent>(entity) };
            auto& layerComp{ *registry.GetComponent<LayerComponent>(entity) };

            renderer.texture->BindTexture();    // bind the texture of the current entity
            if (!renderer.texture->ApplyBlend(renderer.color.a)) glDisable(GL_BLEND);   // Apply texture blending if current texture supports

            float normalisedDepth{ layerComp.renderPriority * normalisedDepthDivisor };    // normalise depth value between [0.f, 1.f]
            shdr.SetUniform("uEntityID", entity);                 // pass in the entity id for object picking
            shdr.SetUniform("uColour", renderer.color);           // set color modulation
            shdr.SetUniform("spriteOrigin", renderer.start);      // set spritesheet origin for spritesheet animation
            shdr.SetUniform("spriteSize", renderer.size);         // set sritesheet size for spritesheet animation
            shdr.SetUniform("tile", renderer.tile);               // set tile amount
            shdr.SetUniform("mdl_to_ndc", projection * transform.transform);  // pass in the transformation matrix
            shdr.SetUniform("ndcDepth", normalisedDepth);         // pass normalised depth into shader
            shdr.SetUniform("uEmissionState", 0);                   // ui shouldn't be affected by emission
            renderUtils.RenderQuadRaw();  // render the quad
        }
    };

    // For drawing opaque world text entities
    auto drawWorldTextEntities = [&](const auto& wtEntList, auto& shdr) {
        for (auto entity : wtEntList) {
            if (!usm->IsActive(entity)) continue;

            auto& transform{ *registry.GetComponent<TransformComponent>(entity) };
            auto& renderer{ *registry.GetComponent<TextRendererComponent>(entity) };
            auto& layerComp{ *registry.GetComponent<LayerComponent>(entity) };

            float normalisedDepth{ layerComp.renderPriority * normalisedDepthDivisor };    // normalise depth value between [0.f, 1.f]

            shdr.SetUniform("uEntityID", entity);               // pass in the entity id for object picking
            shdr.SetUniform("ndcDepth", normalisedDepth);       // pass normalised depth into shader
            shdr.SetUniform("uColour", renderer.color);	        // pass in the colour to use
            float offsetX{};
            // move text based on it's alignment
            switch (renderer.alignment) {
                case TextRendererComponent::LEFT: break;
            case TextRendererComponent::CENTER:
                for (int i{}; i < renderer.text.size(); ++i) {
                    FontObj::Character const& character = renderer.font->map[renderer.text[i]];
                    offsetX += (character.advance) * (static_cast<float>(renderer.fontSize) / renderer.font->resolution);
                }
                offsetX *= -0.5f;
                break;
            case TextRendererComponent::RIGHT:
                for (int i{}; i < renderer.text.size(); ++i) {
                    FontObj::Character const& character = renderer.font->map[renderer.text[i]];
                    offsetX += (character.advance) * (static_cast<float>(renderer.fontSize) / renderer.font->resolution);
                }
                offsetX = -offsetX;
                break;
            default:
                LOGE("Uknown text alignment of TextRendererComponent");
                break;
            }
            renderUtils.RenderTextRaw(renderer.text, viewProjection * transform.transform, 
                Vec2(offsetX, 0), static_cast<float>(renderer.fontSize), shdr, *renderer.font);
        }
    };

    // For drawing opaque UI text entities
    auto drawUITextEntities = [&](const auto& uitEntList, auto& shdr) { 
        Mat3 textTransform{};
        for (auto entity : uitEntList) {
            if (!usm->IsActive(entity)) continue;

            auto& layerComp{ *registry.GetComponent<LayerComponent>(entity) };
            auto& transform{ *registry.GetComponent<UITransformComponent>(entity) };
            auto& renderer{ *registry.GetComponent<TextRendererComponent>(entity) };

            textTransform = transform.viewportTransform * Mat3::Scale(1.f / transform.size.x, 1.f / transform.size.y); // size used for the box to fit the text in, remove scaling for text
            float normalisedDepth{ layerComp.renderPriority * normalisedDepthDivisor };    // normalise depth value between [0.f, 1.f]

            shdr.SetUniform("uEntityID", entity);                 // pass in the entity id for object picking
            shdr.SetUniform("ndcDepth", normalisedDepth);         // pass normalised depth into shader
            shdr.SetUniform("uColour", renderer.color);	        // pass in the colour to use
            std::vector<std::string> lines;
            std::stringstream sstream{ renderer.text };
            std::string buffer;
            
            while (std::getline(sstream, buffer)) { lines.push_back(buffer); }  // extract text into lines

            Vec2 offset{};
            std::string token;
            std::string drawStringBuffer;
            for (std::string const& line : lines) {
                offset.x = 0.f;
                switch (renderer.alignment) {  // move text based on it's alignment
                    case TextRendererComponent::LEFT:
                        offset.x = -transform.size.x / 2.f;
                        break;
                    case TextRendererComponent::CENTER:
                        for (int i{}; i < line.size(); ++i) {
                            FontObj::Character const& character = renderer.font->map[line[i]];
                            offset.x += (character.advance) * (static_cast<float>(renderer.fontSize) / renderer.font->resolution);
                        }
                        offset.x *= -0.5f;
                        break;
                    case TextRendererComponent::RIGHT:
                        for (int i{}; i < line.size(); ++i) {
                            FontObj::Character const& character = renderer.font->map[line[i]];
                            offset.x += (character.advance) * (static_cast<float>(renderer.fontSize) / renderer.font->resolution);
                        }
                        offset.x = transform.size.x / 2.f - offset.x;
                        break;
                    default:
                        LOGE("Uknown text alignment of TextRendererComponent");
                        break;
                }
                offset.y -= (renderer.font->resolution / 2.f + renderer.linePadding);
                renderUtils.RenderTextRaw(line, projection * transform.xTransform, offset, static_cast<float>(renderer.fontSize), shdr, *renderer.font);
            }
        }
    };
    
    GLSLShader& textShader{ CEO::Instance().GetManager<ResourceManager>()->GetShader("font") };
    textShader.Use();                                       // use text shader for drawing text entities
    renderUtils.BindQuadVAO();

    // Render Order: Opaque[World Text -> UI Text -> UI Entities] -> Translucent/Transparent[World Text -> UI Text -> UI Entities]
    drawWorldTextEntities(worldTextEntities, textShader);   // draw opaque world text entities
    drawUITextEntities(uiTextEntities, textShader);         // draw opaque UI text entities
    shader.Use();                                           // use shader for drawing UI entities
    shader.SetUniform("uTex2d", 0);                         // set texture unit as 0
    shader.SetUniform("uUseTex", GL_TRUE);                  // set use texture flag to true
    drawUIEntities(uiEntities, shader);                     // draw opaque UI entities
    
    DrawMinimap(registry, shader);                          // draw minimap

    // Draw transparent entities
    renderUtils.BindQuadVAO();
    textShader.Use();                                       // use text shader for drawing text entities
    drawWorldTextEntities(transparentWorldTextEntities, textShader);    // draw translucent / transparent world text entities
    drawUITextEntities(transparentUITextEntities, textShader);          // draw translucent / transparent UI text entities
    shader.Use();                                           // use shader for drawing UI entities
    drawUIEntities(transparentUIEntities, shader);          // draw translucent / transparent UI entities
    RenderAttributes::Unbind();                             // unbind quad vao after drawing all UI entities
}

void GraphicsSystem::DrawBackground(GLSLShader& shader) const {
    glDisable(GL_DEPTH_TEST);                               // disable depth test for the background
    TextureObj& tex{ CEO::Instance().GetManager<ResourceManager>()->GetTexture(background) };  // get the background texture
    
    tex.BindTexture();                                      // bind current texture for drawing
    tex.ApplyBlend();                                       // apply texture blending (if enabled)

    shader.SetUniform("uTex2d", 0);                         // set texture unit as 0
    shader.SetUniform("uUseTex", GL_TRUE);                  // set use texture flag to true
    shader.SetUniform("uEntityID", 0u);                     // set entity id to 0
    shader.SetUniform("uColour", Color{1.f,1.f,1.f,1.f});   // reset colour to white

    Mat3 xform{Mat3::Scale(2.f, 2.f)};                      // make the quad cover the whole screen
    shader.SetUniform("mdl_to_ndc", xform);                 // pass in the transform

    shader.SetUniform("spriteOrigin", backgroundOffset);    // pass in the sprite offset
    shader.SetUniform("spriteSize", backgroundSize);        // pass in the sprite size
    CEO::Instance().Get<RenderUtils>()->RenderQuad();       // render the background
    glEnable(GL_DEPTH_TEST);    // re-enable depth test after drawing background
}

void GraphicsSystem::UpdateMinimap(Registry& registry) {
    // Get minimap trackers
    auto isMinimapCamDirty{ CEO::Get<PersistentDataManager>()->Get<bool>("IsMinimapCamDirty") };
    auto minimapCamEnt{ CEO::Get<PersistentDataManager>()->Get<EntityRegistry::Entity>("MinimapCamEnt") },
        minimapEnt{ CEO::Get<PersistentDataManager>()->Get<EntityRegistry::Entity>("MinimapEnt") };
    
    if (!minimapCamEnt || !minimapEnt || !*minimapCamEnt || !*minimapEnt) return;   // if trackers are invalid, end function early

    auto toggle{ CEO::Get<PersistentDataManager>()->Get<bool>("ToggleMinimapOffDuringCombat") };    // get toggle flag for toggling minimap off during combat
    auto isCombat{ CEO::Get<PersistentDataManager>()->Get<bool>("InCombat") };                      // get combat flag for tracking whether player is in combat

    if (toggle == nullptr || isCombat == nullptr || !*toggle || !*isCombat) {
        auto& renderUtils{ *CEO::Get<RenderUtils>() };
        auto& layerManager{ *CEO::Get<LayerManager>() };
        auto& usm{ *CEO::Get<UpdateStackManager>() };

        auto& shader{ CEO::Get<ResourceManager>()->GetShader("sprite") };               // get shader for drawing minimap
        auto& minimapCam{ *registry.GetComponent<CameraComponent>(*minimapCamEnt) };    // get the camera component of minimap camera
        const auto& minimapCamTransformComp{ *registry.GetComponent<TransformComponent>(*minimapCamEnt) };

        const Vec2 camHalfExtent{ minimapCam.viewportSize * 0.75f };                    // get camera's viewport halfextent for view culling
        shader.Use();
        if (isMinimapCamDirty && *isMinimapCamDirty) {          // update minimap environment/background if dirty flag is set 
            const float halfChunkSize{ TilemapComponent::CHUNK_SIZE * 0.5f };
            ScreenManager::minimapStaticScreenBuffer.Bind();    // bind the framebuffer
            glViewport(0, 0, ScreenManager::minimapStaticScreenBuffer.Width(), ScreenManager::minimapStaticScreenBuffer.Height());
            glClearColor(clear_colour[0], clear_colour[1], clear_colour[2], 0.f);
            glClearDepthf(0.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Get all visible tilemap entities
            auto tilemaps{ CEO::Get<LayerManager>()->GetEntitiesVisibleWithComponent<TilemapComponent>(minimapCam.cullingMask) };
            // Get visible environment entities + lave entities
            auto environments{ layerManager.GetEntitiesVisibleWithComponentsByLayerName<SpriteRendererComponent, TransformComponent>(minimapCam.cullingMask, "Environment") };
            auto lava{ layerManager.GetEntitiesVisibleWithComponentsByLayerName<SpriteRendererComponent, TransformComponent>(minimapCam.cullingMask, "Lava") };
            // Lambda function for rendering environment entities
            auto renderEnvironment = [&](const std::vector<EntityRegistry::Entity>& entity) {
                for (auto ent : entity) {               // loop through each environment entity
                    if (!usm.IsActive(ent)) continue;   // skip if entity is inactive
                    auto& sprite{ *registry.GetComponent<SpriteRendererComponent>(ent) };
                    if (!sprite.visible) continue;      // skip if entity is not visible
                    // check and skip if environment entity is not within camera's view
                    auto& transformComp{ *registry.GetComponent<TransformComponent>(ent) };
                    if (!IsInView(transformComp.transform, minimapCamTransformComp.translate, camHalfExtent)) continue;

                    float normalisedDepth{ registry.GetComponent<LayerComponent>(ent)->renderPriority * normalisedDepthDivisor };
                    Mat3 xform{ minimapCam.vp * transformComp.transform };          // extract the entity transform

                    sprite.texture->BindTexture();                      // bind environment entity's texture
                    shader.SetUniform("uColour", sprite.color);         // set color modulation
                    shader.SetUniform("spriteOrigin", sprite.start);    // set environment entity's sprite origin
                    shader.SetUniform("spriteSize", sprite.size);       // set environment entity's sprite size
                    shader.SetUniform("tile", sprite.tile);             // set environment entity's tile amount
                    shader.SetUniform("ndcDepth", normalisedDepth);     // pass normalised depth into shader
                    shader.SetUniform("uEmissionState", 0);             // set environment entity's emissive state to 0
                    shader.SetUniform("mdl_to_ndc", xform);             // pass entity transform into shader

                    renderUtils.RenderQuad();                           // render the environment entity as a quad
                    sprite.texture->UnbindTexture();                    // unbind environment entity's texture
                }
            };

            renderUtils.BindQuadVAO();

            shader.SetUniform("uColour", Color{ 1.f, 1.f, 1.f, 1.f });
            shader.SetUniform("uUseTex", true);             // set use texture flag to true
            shader.SetUniform("uTex2d", 0);                 // set texture unit as 0
            shader.SetUniform("uEmissionState", 0);         // disable emission
            for (auto ent : tilemaps) {                     // draw tilemaps to minimap static framebuffer
                if (!usm.IsActive(ent)) continue;           // skip if tilemap is inactive

                auto& tileMap{ *registry.GetComponent<TilemapComponent>(ent) };
                if (!tileMap.tileset) continue;             // skip if there is no tileset

                auto& transform{ *registry.GetComponent<TransformComponent>(ent) };
                auto& layerComp{ *registry.GetComponent<LayerComponent>(ent) };

                const Vec2 tilemapPos{ transform.transform.m[6], transform.transform.m[7] },
                    chunkSize{ transform.scale.x * TilemapComponent::CHUNK_SIZE, transform.scale.y * TilemapComponent::CHUNK_SIZE };

                tileMap.tileset->texture->BindTexture();
                shader.SetUniform("ndcDepth", layerComp.renderPriority * normalisedDepthDivisor);
                for (auto& chunk : tileMap.chunks) {
                    Vec2 chunkPos{
                        tilemapPos.x + chunk.first.chunkX * TilemapComponent::CHUNK_SIZE + halfChunkSize,
                        tilemapPos.y + chunk.first.chunkY * TilemapComponent::CHUNK_SIZE + halfChunkSize
                    };
                    // skip if tilemap is not within camera's view
                    if (!IsInView(chunkPos, chunkSize, minimapCamTransformComp.translate, camHalfExtent)) continue;

                    for (int row{}; row < TilemapComponent::CHUNK_SIZE; ++row) {
                        for (int col{}; col < TilemapComponent::CHUNK_SIZE; ++col) {
                            int spriteID{ chunk.second[row * TilemapComponent::CHUNK_SIZE + col].spriteID };
                            if (spriteID == -1 || spriteID >= tileMap.tileset->sprites.size()) continue;

                            const auto& spriteInfo{ tileMap.tileset->sprites[spriteID] };
                            shader.SetUniform("mdl_to_ndc", minimapCam.vp * transform.transform *
                                Mat3::Translation(chunk.first.chunkX * TilemapComponent::CHUNK_SIZE + col + 0.5f,
                                    chunk.first.chunkY * TilemapComponent::CHUNK_SIZE + row + 0.5f));
                            shader.SetUniform("spriteOrigin", spriteInfo.min);
                            shader.SetUniform("spriteSize", spriteInfo.max - spriteInfo.min);
                            renderUtils.RenderQuadRaw();
                        }
                    }
                }
                tileMap.tileset->texture->UnbindTexture();
            }

            renderEnvironment(environments);        // render the environment entities (excluding lava)
            renderEnvironment(lava);                // render the lava environment entities
            ScreenManager::minimapStaticScreenBuffer.Unbind();
        }
        // Draw the dynamic portion of minimap (such as entities)
        ScreenManager::minimapScreenBuffer.Bind();
        glViewport(0, 0, ScreenManager::minimapScreenBuffer.Width(), ScreenManager::minimapScreenBuffer.Height());
        glClearColor(clear_colour[0], clear_colour[1], clear_colour[2], 0.f);
        glClearDepthf(0.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // if blind mode is on, skip updating dynamic portion of minimap
        auto check{ CEO::Get<PersistentDataManager>()->Get<bool>("IsBlindModeOn") };
        if (!(check && *check)) {
            // Get all visible entities (player + enemies)
            auto players{ layerManager.GetEntitiesVisibleWithComponentsByLayerName<SpriteRendererComponent, TransformComponent>(minimapCam.cullingMask, "Player", false) };
            auto enemies{ layerManager.GetEntitiesVisibleWithComponentsByLayerName<SpriteRendererComponent, TransformComponent>(minimapCam.cullingMask, "Enemy", false) };
            
            auto renderEntities = [&](const std::vector<EntityRegistry::Entity>& entities, const Color& clr) {
                const float ar{ static_cast<float>(ScreenManager::minimapScreenBuffer.Width()) / ScreenManager::minimapScreenBuffer.Height()};
                float baseScale{ 0.0001f };
                float scaleX{ baseScale * camHalfExtent.x }, scaleY{ baseScale * camHalfExtent.y };
                scaleX /= ar;
                for (auto ent : entities) {
                    if (!usm.IsActive(ent)) continue;   // skip if entity is inactive
                    auto& sprite{ *registry.GetComponent<SpriteRendererComponent>(ent) };
                    if (!sprite.visible) continue;      // skip if entity is not visible
                    // skip if entity is not within camera's view
                    auto& transformComp{ *registry.GetComponent<TransformComponent>(ent) };
                    if (!IsInView(transformComp.transform, minimapCamTransformComp.translate, camHalfExtent)) continue;

                    float normalisedDepth{ registry.GetComponent<LayerComponent>(ent)->renderPriority * normalisedDepthDivisor };
                    Mat3 xform{ minimapCam.vp * transformComp.transform };  // extract the entity transform
                    xform *= Mat3::Scale(scaleX, scaleY);                   // scale down
                    shader.SetUniform("ndcDepth", normalisedDepth);         // pass normalised depth into shader
                    shader.SetUniform("uColour", clr);                      // draw specified colour
                    shader.SetUniform("mdl_to_ndc", xform);                 // pass entity transform into shader
                    renderUtils.RenderCircleRaw();                          // render the circle
                }
                };

            shader.SetUniform("uUseTex", false);        // set use texture flag to false
            shader.SetUniform("uTex2d", 0);             // set texture unit as 0
            shader.SetUniform("uEmissionState", 0);     // set emission state to 0

            renderUtils.BindCircleVAO();                // bind circle vao
            renderEntities(enemies, enemyColour);       // draw the enemy entities
            renderEntities(players, playerColour);      // draw the player entities
            shader.UnUse();
        }
        ScreenManager::minimapScreenBuffer.Unbind();
    }
}

void GraphicsSystem::DrawMinimap(Registry& registry, GLSLShader& shader) {
    auto minimapEnt{ CEO::Get<PersistentDataManager>()->Get<EntityRegistry::Entity>("MinimapEnt") };
    if (minimapEnt == nullptr || *minimapEnt == 0) return;
    auto toggle{ CEO::Get<PersistentDataManager>()->Get<bool>("ToggleMinimapOffDuringCombat") };
    auto isCombat{ CEO::Get<PersistentDataManager>()->Get<bool>("InCombat") };

    if (toggle == nullptr || isCombat == nullptr || !*toggle || !*isCombat) {   // Draw minimap onto game
        auto& renderUtils{ *CEO::Get<RenderUtils>() };

        const auto& minimapTransform{ *registry.GetComponent<UITransformComponent>(*minimapEnt) };
        auto minimapRenderPriority{ registry.GetComponent<LayerComponent>(*minimapEnt)->renderPriority };

        shader.Use();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindTexture(GL_TEXTURE_2D, ScreenManager::minimapStaticScreenBuffer.ColorBufferTex());
        
        shader.SetUniform("uTex2d", 0);                             // set texture unit as 0
        shader.SetUniform("uUseTex", true);                         // set use texture flag to true
        //shader.SetUniform("uEmmissionTex2d", 1);     
        shader.SetUniform("uColour", Color(1.f, 1.f, 1.f, 1.f));    // reset colour to white
        shader.SetUniform("spriteOrigin", Vec2(0.f, 0.f));          // set sprite origin to default
        shader.SetUniform("spriteSize", Vec2(1.f, 1.f));            // set sprite size to default
        shader.SetUniform("ndcDepth", (minimapRenderPriority + 1) * normalisedDepthDivisor);    // pass normalised depth into shader
        shader.SetUniform("mdl_to_ndc", CEO::Get<CameraManager>()->GetProjection() * minimapTransform.transform * Mat3::Scale(0.8f, 0.8f));
        shader.SetUniform("uEmissionState", 0);                     // set emission state to 0
        renderUtils.RenderQuad();   // draw minimap background (the static tilemaps)

        glBindTexture(GL_TEXTURE_2D, ScreenManager::minimapScreenBuffer.ColorBufferTex());
        shader.SetUniform("ndcDepth", (minimapRenderPriority + 2) * normalisedDepthDivisor);
        renderUtils.RenderQuad();   // draw the dynamic portion of minimap (entities, etc...)

        shader.UnUse();
        glDisable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void GraphicsSystem::Free() {

    if (!init) return;
#ifdef _DEBUG
        LOGI("Start of GraphicsSystem Cleanup");
#endif
        CEO::Instance().Get<RenderUtils>()->Free();        // free up the vaos and vbos in renderUtils
#ifdef EditorFlag
        CEO::Instance().GetManager<DebugRender>()->Free();  // free up the vaos and vbos used in DebugRender (for instancing)
#endif
#ifdef _DEBUG
        LOGI("End of GraphicsSystem Cleanup");
#endif
        init = false;
}

void GraphicsSystem::ResizeInstanceBuffer(size_t size) {
    auto& renderUtils{ *CEO::Instance().Get<RenderUtils>() };

    if (renderUtils.GetEntityInstanceLimit() * 2 <= size)       // if 2 * limit is still smaller than size
        renderUtils.SetEntityInstanceLimit(size * 2);           // use 2 * size as new instance limit
    else renderUtils.SetEntityInstanceLimit(renderUtils.GetEntityInstanceLimit() * 2);           // otherwise, use 2 * instance limit as new instance limit
    
    xforms.resize(renderUtils.GetEntityInstanceLimit());      // resize the xforms vector

    // update the entity instance GPU buffer
    renderUtils.GetEntitynstanceVBO().SetBuffer(xforms.data(), 
    renderUtils.GetEntityInstanceLimit() * sizeof(InstanceData));

}

void GraphicsSystem::Bloom(const std::vector<EntityRegistry::Entity>& postProcess) {
    Registry& registry = *CEO::Get<Registry>();

    float bloomThreshold = 0.9f;
    if (postProcess.size() > 0)
        bloomThreshold  = registry.GetComponent<PostProcessComponent>(postProcess[0])->bloomThreshold;

    
    // setup buffers for bright extraction pass
    ScreenManager::brightBuffer.Bind();
    glClearColor(0.f, 0.f, 0.f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // extract out bright pixels into buffer to use for blurring
    GLSLShader& brightExtractShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame.vert", "brightextract.frag");
    brightExtractShader.Use();

    // bind the emissive colors in the scene
    glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
    glBindTexture(GL_TEXTURE_2D, ScreenManager::screenBuffer.SecondaryColorBufferTex());
    brightExtractShader.SetUniform("uEmissiveTex", 1);
    // bind main buffer's current color output for extraction
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ScreenManager::screenBuffer.ColorBufferTex());
    brightExtractShader.SetUniform("uSceneTex", 0);
    brightExtractShader.SetUniform("uThreshold", bloomThreshold);

    // extract bright pixels
    RenderUtils& renderUtils = *CEO::Get<RenderUtils>();
    renderUtils.RenderQuad();

    // taken from https://learnopengl.com/Advanced-Lighting/Bloom
    // blur pass
    GLSLShader& blurShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame.vert", "gaussian.frag");

    // repeat horizontal and vertical burring
    int horizontal = 1;
    int amount = 10;
    blurShader.Use();
    for (int i = 0; i < amount; i++)
    {
        ScreenManager::postProcessBuffers[horizontal].Bind();
        blurShader.SetUniform("horizontal", horizontal);
        // first iteration grab initial data from where we store the "bright" colors
        glBindTexture(GL_TEXTURE_2D, i == 0 ? ScreenManager::brightBuffer.ColorBufferTex() : ScreenManager::postProcessBuffers[!horizontal].ColorBufferTex());
        blurShader.SetUniform("uTex2d", 0);
        renderUtils.RenderQuad();

        // flip
        horizontal = horizontal?0:1;
    }

    int sceneTexIndex = horizontal ? 1 : 0;
    int blurTexIndex = horizontal ? 0 : 1;

    // copy scene output to use as a input texture for subsequent pass
    GLSLShader& passthroughShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame");
    passthroughShader.Use();
    ScreenManager::postProcessBuffers[blurTexIndex].Bind();
    glBindTexture(GL_TEXTURE_2D, ScreenManager::screenBuffer.ColorBufferTex());
    passthroughShader.SetUniform("uTex2d", 0);
    renderUtils.RenderQuad();

    // combine and blend blur output with scene output
    ScreenManager::screenBuffer.Bind();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    GLSLShader& bloomShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame.vert", "bloom.frag");
    bloomShader.Use();
    GLuint sceneTex = ScreenManager::postProcessBuffers[blurTexIndex].ColorBufferTex();
    GLuint blurTex = ScreenManager::postProcessBuffers[sceneTexIndex].ColorBufferTex();

    glActiveTexture(GL_TEXTURE0 + 1); // Texture unit 1
    glBindTexture(GL_TEXTURE_2D, blurTex);
    bloomShader.SetUniform("blurTex", 1);

    glActiveTexture(GL_TEXTURE0 + 0); // Texture unit 0
    glBindTexture(GL_TEXTURE_2D, sceneTex);
    bloomShader.SetUniform("sceneTex", 0);

    if (postProcess.size() > 0)
        bloomShader.SetUniform("exposure", registry.GetComponent<PostProcessComponent>(postProcess[0])->bloomExposure);
    else
        bloomShader.SetUniform("exposure", 0.5f);

    // combine outputs
    renderUtils.RenderQuad();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
}

void GraphicsSystem::Vignette(const std::vector<EntityRegistry::Entity>& postProcess) {   
    Registry& registry = *CEO::Get<Registry>();

    float intensity = 0.f;
    float falloff = 1.0f;

    // get properties from post process component
    if (postProcess.size() > 0)
    {
        falloff = registry.GetComponent<PostProcessComponent>(postProcess[0])->vignetteFalloff;
        intensity = registry.GetComponent<PostProcessComponent>(postProcess[0])->vignetteIntensity;
    }

    // near 0 intensity, don't even vignette
	if (intensity < FLT_EPSILON)
        return;

    // draw vignette
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    GLSLShader& vignetteShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame.vert", "vignette.frag");
    vignetteShader.Use();
	vignetteShader.SetUniform("uIntensity", intensity);
	vignetteShader.SetUniform("uFalloff", falloff);
    RenderUtils& renderUtils = *CEO::Get<RenderUtils>();
    renderUtils.RenderQuad();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void GraphicsSystem::LightingPass(Registry& registry) {
    auto lightEntities = registry.GetEntitiesWithComponents<LightComponent, TransformComponent>();

    // no lights, we do no lighting
    if (lightEntities.size() == 0)
        return;

    // clear buffer
    ScreenManager::postProcessBuffers[1].Bind();
    glClearColor(0.f, 0.f, 0.f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    ScreenManager::postProcessBuffers[0].Bind();
    glClearColor(0.f, 0.f, 0.f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    // initialize opengl for lighting pass
    glDisable(GL_DEPTH_TEST);    // disable depth test for drawing lighting overlay
    glEnable(GL_BLEND);


    GLSLShader& emissivePassShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame.vert", "emissive.frag");
    GLSLShader& lightShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("light");
    GLSLShader& globalLightShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame.vert", "GlobalLight.frag");
    GLSLShader& lightCopyShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame.vert", "lightcopy.frag");
    Mat3 VP = CEO::Instance().GetManager<CameraManager>()->GetViewProjection();

    lightCopyShader.Use();
    lightCopyShader.SetUniform("uColourTex", 3);
    lightCopyShader.SetUniform("uIntensityTex", 4);

    globalLightShader.Use();
    globalLightShader.SetUniform("uCurrColorTex", 1);
    globalLightShader.SetUniform("uCurrIntensityTex", 2);

	int curr = 0;
    int other = 1;
    RenderUtils& renderUtils = *CEO::Get<RenderUtils>();
    renderUtils.BindQuadVAO();         // bind quad vao first (to prevent constant bind and unbinding)
    ScreenManager::postProcessBuffers[curr].Bind();
    glDisable(GL_BLEND);

    // draw emissive colors into light color/intensity buffers
    emissivePassShader.Use();
    emissivePassShader.SetUniform("uEmissiveTex", 0);
    ScreenManager::postProcessBuffers[other].Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ScreenManager::screenBuffer.SecondaryColorBufferTex());
    // draw light data onto buffers
    renderUtils.RenderQuadRaw();


    lightShader.Use();
    lightShader.SetUniform("uLightShapeTex", 0);
    lightShader.SetUniform("uCurrColorTex", 1);
    lightShader.SetUniform("uCurrIntensityTex", 2);

    glActiveTexture(GL_TEXTURE0);
    TextureObj& defaultLightShape = CEO::Get<ResourceManager>()->GetTexture("radialFade.png");
    defaultLightShape.BindTexture();

    renderUtils.BindQuadVAO();         // bind quad vao first (to prevent constant bind and unbinding)
    ScreenManager::postProcessBuffers[curr].Bind();
    glDisable(GL_BLEND);

    UpdateStackManager* usm = CEO::Get<UpdateStackManager>();

    for (int i = 0; i < lightEntities.size(); ++i) {
        if (!usm->IsActive(lightEntities[i])) continue; // skip

        LightComponent* light{ registry.GetComponent<LightComponent>(lightEntities[i]) };

        if (light->type == LightComponent::POINT)
        {
			lightShader.Use();
			//lightShader.SetUniform("uFirstPass", i == 0);
			// pass previous light pass to curr pass
            glActiveTexture(GL_TEXTURE0); // bind screen space light color buffer
            if (light->lightShapeTex)
                light->lightShapeTex->BindTexture();
            else
                defaultLightShape.BindTexture();
			glActiveTexture(GL_TEXTURE0 + 1); // bind screen space light color buffer
			glBindTexture(GL_TEXTURE_2D, ScreenManager::postProcessBuffers[other].ColorBufferTex());
			glActiveTexture(GL_TEXTURE0 + 2); // bind screen space light intensity buffer
			glBindTexture(GL_TEXTURE_2D, ScreenManager::postProcessBuffers[other].LightBufferTex());
			lightShader.SetUniform("uScreenResolution", Vec2{ static_cast<float>(ScreenManager::screenBuffer.Width()), static_cast<float>(ScreenManager::screenBuffer.Height()) });

			TransformComponent* transform{ registry.GetComponent<TransformComponent>(lightEntities[i]) };

			// set uniforms for light
			Mat3 xform = VP * transform->transform;
			lightShader.SetUniform("mdl_to_ndc", xform);
			lightShader.SetUniform("uColour", light->color);
			lightShader.SetUniform("uIntensity", light->intensity);
        }
        else if (light->type == LightComponent::GLOBAL)
        {
            globalLightShader.Use();
            //globalLightShader.SetUniform("uFirstPass", i == 0);
            // pass previous light pass to curr pass
            glActiveTexture(GL_TEXTURE0 + 1); // bind screen space light color buffer
            glBindTexture(GL_TEXTURE_2D, ScreenManager::postProcessBuffers[other].ColorBufferTex());
            glActiveTexture(GL_TEXTURE0 + 2); // bind screen space light intensity buffer
            glBindTexture(GL_TEXTURE_2D, ScreenManager::postProcessBuffers[other].LightBufferTex());
            globalLightShader.SetUniform("uScreenResolution", Vec2{ static_cast<float>(ScreenManager::screenBuffer.Width()), static_cast<float>(ScreenManager::screenBuffer.Height()) });

            TransformComponent* transform{ registry.GetComponent<TransformComponent>(lightEntities[i]) };
            (void)transform;
            // set uniforms for light
            globalLightShader.SetUniform("uColour", light->color);
            globalLightShader.SetUniform("uIntensity", light->intensity);
        }

        // draw light data onto buffers
        renderUtils.RenderQuadRaw();

        std::swap(curr, other);

        // copy data to next pass
        ScreenManager::postProcessBuffers[curr].Bind();
        lightCopyShader.Use();

        glActiveTexture(GL_TEXTURE0 + 3); // bind screen space light color buffer
        glBindTexture(GL_TEXTURE_2D, ScreenManager::postProcessBuffers[other].ColorBufferTex());
        glActiveTexture(GL_TEXTURE0 + 4); // bind screen space light intensity buffer
        glBindTexture(GL_TEXTURE_2D, ScreenManager::postProcessBuffers[other].LightBufferTex());

        renderUtils.RenderQuadRaw();
    }

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLSLShader& passthroughShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame");
    passthroughShader.Use();

    curr = curr ? 1 : 0;
	other = curr ? 0 : 1;

    // copy scene into post process buffer for illumination
    ScreenManager::postProcessBuffers[curr].Bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ScreenManager::screenBuffer.ColorBufferTex());
    passthroughShader.SetUniform("uTex2d", 0);
    renderUtils.RenderQuad();

    // get textures to combine for final result
    GLuint sceneTex = ScreenManager::postProcessBuffers[curr].ColorBufferTex();
    GLuint lightColorTex = ScreenManager::postProcessBuffers[other].ColorBufferTex();
    GLuint lightIntensityTex = ScreenManager::postProcessBuffers[other].LightBufferTex();

    ScreenManager::screenBuffer.Bind();
    // only write to main color buffer
    GLenum bufs[3] = { GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, bufs);
    GLSLShader& illuminationShader = CEO::Instance().GetManager<ResourceManager>()->GetShader("frame.vert", "illumination.frag");
    illuminationShader.Use();
    glActiveTexture(GL_TEXTURE0 + 2); // bind screen space light intensity buffer
    glBindTexture(GL_TEXTURE_2D, lightIntensityTex);
    illuminationShader.SetUniform("lightIntensityTex", 2);

    glActiveTexture(GL_TEXTURE0 + 1); // bind screen space light color buffer
    glBindTexture(GL_TEXTURE_2D, lightColorTex);
    illuminationShader.SetUniform("lightColorTex", 1);

    glActiveTexture(GL_TEXTURE0 + 0); // bind scene texture rendered in camera viewport
    glBindTexture(GL_TEXTURE_2D, sceneTex);
    illuminationShader.SetUniform("sceneTex", 0);

    // final pass with shaders
    
    renderUtils.RenderQuad();

    glEnable(GL_DEPTH_TEST);    // re-enable depth test after drawing lighting overlay

}

bool GraphicsSystem::IsInView(const Mat3& transform, const Vec2& camPos, const Vec2& camHalfExtent) {
    Vec2 pos{ transform.m[6], transform.m[7] };
    float scaleXSq{ transform.m[0] * transform.m[0] + transform.m[1] * transform.m[1] },
        scaleYSq{ transform.m[3] * transform.m[3] + transform.m[4] * transform.m[4] };

    if (scaleXSq == 0.f || scaleYSq == 0.f) return false;
    Vec2 scale{ std::sqrt(scaleXSq), std::sqrt(scaleYSq) };
    return IsInView(pos, scale, camPos, camHalfExtent);
}

bool GraphicsSystem::IsInView(const Vec2& pos, const Vec2& scale, const Vec2& camPos, const Vec2& camHalfExtent) {
    Vec2 scaleHalfExtent{ scale.x * 0.5f, scale.y * 0.5f };
    return pos.x + scaleHalfExtent.x >= camPos.x - camHalfExtent.x &&
        pos.x - scaleHalfExtent.x <= camPos.x + camHalfExtent.x &&
        pos.y + scaleHalfExtent.y >= camPos.y - camHalfExtent.y &&
        pos.y - scaleHalfExtent.y <= camPos.y + camHalfExtent.y;
}