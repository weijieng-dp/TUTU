/*!
@file       TilesetEditor.cpp
@author     Ou Yukang (yukang.ou) 100%
@date       11/01/2026
@brief		Handles editing tilesets and rendering ImGui widgets for the editor

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "TilesetEditor.h"
#include "pch.h"
#include "editor.h"
#include "imgui.h"

void TilesetEditor::DrawGrid(Registry::Entity tilemapEntity)
{
    // get draw list to add custom draws to the imgui window
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    Registry& registry = *CEO::Get<Registry>();
    TransformComponent& transform = *registry.GetComponent<TransformComponent>(tilemapEntity);

    Vec2 camViewportSize = CEO::Get<CameraManager>()->GetViewportSize();
    ImVec2 editorViewportSize = ImGui::GetItemRectSize();

    // get scaling to scale from game coordinates to imgui window coordinates
    float camToEditorViewportScaling = editorViewportSize.x / camViewportSize.x;

    Vec2 tilemapPos = transform.translate * camToEditorViewportScaling;
    Vec2 cameraPos = (CEO::Get<CameraManager>()->GetPosition() - 0.5f * CEO::Get<CameraManager>()->GetViewportSize()) * camToEditorViewportScaling;
    ImVec2 viewportOrigin = ImGui::GetItemRectMin();

    // draw tile grid
	float GRID_STEP_X = transform.scale.x * camToEditorViewportScaling;
    float GRID_STEP_Y = transform.scale.y * camToEditorViewportScaling;
    for (float x = fmodf(tilemapPos.x - cameraPos.x, GRID_STEP_X); x < editorViewportSize.x; x += GRID_STEP_X)
		draw_list->AddLine(ImVec2(viewportOrigin.x + x, viewportOrigin.y), ImVec2(viewportOrigin.x + x, viewportOrigin.y + editorViewportSize.y), IM_COL32(50, 50, 50, 200), 5.f);
    for (float y = editorViewportSize.y - fmodf(tilemapPos.y - cameraPos.y, GRID_STEP_Y); y > 0.f; y -= GRID_STEP_Y)
        draw_list->AddLine(ImVec2(viewportOrigin.x, viewportOrigin.y + y), ImVec2(viewportOrigin.x + editorViewportSize.x, viewportOrigin.y + y), IM_COL32(50, 50, 50, 200), 5.f);

    // draw chunk grid
    GRID_STEP_X *= TilemapComponent::TilemapProperties::CHUNK_SIZE;
    GRID_STEP_Y *= TilemapComponent::TilemapProperties::CHUNK_SIZE;
    for (float x = fmodf(tilemapPos.x - cameraPos.x, GRID_STEP_X); x < editorViewportSize.x; x += GRID_STEP_X)
        draw_list->AddLine(ImVec2(viewportOrigin.x + x, viewportOrigin.y), ImVec2(viewportOrigin.x + x, viewportOrigin.y + editorViewportSize.y), IM_COL32(255, 50, 50, 200), 5.f);
    for (float y = editorViewportSize.y - fmodf(tilemapPos.y - cameraPos.y, GRID_STEP_Y); y > 0.f; y -= GRID_STEP_Y)
        draw_list->AddLine(ImVec2(viewportOrigin.x, viewportOrigin.y + y), ImVec2(viewportOrigin.x + editorViewportSize.x, viewportOrigin.y + y), IM_COL32(255, 50, 50, 200), 5.f);
}

void TilesetEditor::DrawWindow()
{
    ImGui::Begin("Tileset Editor", &showWindow);
    ImGui::InputText("Tileset name", createNewTilemapName, sizeof(createNewTilemapName));
    if (ImGui::Button("Create New"))
    {
        if (CEO::Get<ResourceManager>()->SaveTileset(Tileset{}, createNewTilemapName))
            SetSelectedTileset(CEO::Get<ResourceManager>()->GetTileset(std::string(createNewTilemapName) + ".tileset"));
    }
    //ImGuiIO& io = ImGui::GetIO();
    ImVec2 pos = ImGui::GetCursorScreenPos();

    if (selectedTileset)
    {
		//TextureObj* tilemapTex = selectedTileset->texture;
		//ImGui::ImageWithBg(tilemapTex.TexId(), ImVec2(tilemapTex.Width(), tilemapTex.Height()), ImVec2{ 0,1 }, ImVec2{ 1,0 }, ImVec4{ 0.2f,0.2f,0.2f,1.f });
        ImGui::InputText("Tileset texture", setNewTextureName,sizeof(setNewTextureName));
        HandleDragDrop();
        if (ImGui::Button("Set texture"))
        {
            selectedTileset->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(setNewTextureName);
            // regenerate tileset with new texture
            GenerateTiles(*selectedTileset);
            CEO::Get<ResourceManager>()->SaveTileset(*selectedTileset, selectedTileset->path);
        }

        // gui for rendering sprites within tileset
        if(selectedTileset->texture)
        {
            ImGui::InputInt("Tile Size", &newTileSize);
            if (ImGui::Button("Generate tiles"))
            {
                // auto generate tiles based on tile size
                GenerateTiles(*selectedTileset);
                CEO::Get<ResourceManager>()->SaveTileset(*selectedTileset, selectedTileset->path);
            }

            // draw selectible tiles for drawing in tilemap
            ImGui::NewLine();
            DrawPalette();
        }

    }
	ImGui::End();
}

void TilesetEditor::GenerateTiles(Tileset& tileset)
{
    tileset.tileSize = newTileSize;
    float xStep = tileset.tileSize / static_cast<float>(tileset.texture->Width());
    float yStep = tileset.tileSize / static_cast<float>(tileset.texture->Height());
    int rows = tileset.texture->Height() / tileset.tileSize;
    int cols = tileset.texture->Width() / tileset.tileSize;
    tileset.sprites.clear();
    tileset.sprites.reserve(rows * cols);

    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            SpriteInfo info{ Vec2{col * xStep ,(rows - row - 1) * yStep}, Vec2{(col + 1) * xStep ,(rows - row) * yStep } };
            tileset.sprites.push_back(info);
        }
    }
}

void TilesetEditor::SetSelectedTileset(std::shared_ptr<Tileset> tileset)
{
    selectedTileset = tileset;
    selectedSpriteID = -1;
    if(selectedTileset)
		newTileSize = selectedTileset->tileSize;
}

void TilesetEditor::PlaceSelectedTile(Registry::Entity tilemapEntity, float mouseX, float mouseY)
{
    if (!selectedTileset || selectedSpriteID == -1)
        return;

    TilemapComponent& tilemap = *CEO::Get<Registry>()->GetComponent<TilemapComponent>(tilemapEntity);
    TransformComponent& transform = *CEO::Get<Registry>()->GetComponent<TransformComponent>(tilemapEntity);

    CameraManager& camManager = *CEO::Get<CameraManager>();
    Vec2 camViewportSize = camManager.GetViewportSize();
    ImVec2 editorViewportSize = ImGui::GetItemRectSize();

    ImVec2 editorViewportOrigin = ImGui::GetItemRectMin();

    // get mouse position relative to editor and invert y
    mouseX = mouseX - editorViewportOrigin.x;
    mouseY = editorViewportSize.y - (mouseY - editorViewportOrigin.y);

    // get scaling to scale from imgui window coordinates to game coordinates
    float editorToCamViewportScaling = camViewportSize.x / editorViewportSize.x ;

    // screen to world position
    Vec2 worldPos = camManager.GetView().Inversed()* Vec2 { mouseX * editorToCamViewportScaling - camViewportSize.x * 0.5f, 
                                                            mouseY * editorToCamViewportScaling - camViewportSize.y * 0.5f};

    // set sprite id at coordinate to selected spriteID
    std::pair<int, int> colRow = TilesetManager::GetTileCoordinates(transform, worldPos);
    ChunkCoordinates chunkCoord = TilesetManager::GetChunkCoordinates(transform, worldPos);
    Tile& tile = tilemap.chunks[chunkCoord][colRow.second * TilemapComponent::CHUNK_SIZE + colRow.first];
    tile.spriteID = selectedSpriteID;
}

void TilesetEditor::RemoveTile(Registry::Entity tilemapEntity, float mouseX, float mouseY)
{
    TilemapComponent& tilemap = *CEO::Get<Registry>()->GetComponent<TilemapComponent>(tilemapEntity);
    TransformComponent& transform = *CEO::Get<Registry>()->GetComponent<TransformComponent>(tilemapEntity);

    CameraManager& camManager = *CEO::Get<CameraManager>();
    Vec2 camViewportSize = camManager.GetViewportSize();
    ImVec2 editorViewportSize = ImGui::GetItemRectSize();

    ImVec2 editorViewportOrigin = ImGui::GetItemRectMin();

    // get mouse position relative to editor and invert y
    mouseX = mouseX - editorViewportOrigin.x;
    mouseY = editorViewportSize.y - (mouseY - editorViewportOrigin.y);

    // get scaling to scale from imgui window coordinates to game coordinates
    float editorToCamViewportScaling = camViewportSize.x / editorViewportSize.x;

    // screen to world position
    Vec2 worldPos = camManager.GetView().Inversed() * Vec2 {
        mouseX* editorToCamViewportScaling - camViewportSize.x * 0.5f,
            mouseY* editorToCamViewportScaling - camViewportSize.y * 0.5f
    };

    // set sprite id at coordinate to -1 (empty)
    std::pair<int, int> colRow = TilesetManager::GetTileCoordinates(transform, worldPos);
    ChunkCoordinates chunkCoord = TilesetManager::GetChunkCoordinates(transform, worldPos);
    Tile& tile = tilemap.chunks[chunkCoord][colRow.second * TilemapComponent::CHUNK_SIZE + colRow.first];
    tile.spriteID = -1;
}

void TilesetEditor::DrawPalette()
{
    TextureObj* tilemapTex = selectedTileset->texture;
    float xStep = selectedTileset->tileSize / static_cast<float>(tilemapTex->Width());
    float yStep = selectedTileset->tileSize / static_cast<float>(tilemapTex->Height());
    char buf[64];
    int rows = tilemapTex->Height() / selectedTileset->tileSize;
    int cols = tilemapTex->Width() / selectedTileset->tileSize;

    // draw sprites as buttons that can be selected
    for (int row = 0; row < rows; ++row)
    {
        for (int col = 0; col < cols; ++col)
        {
            if (col < cols)
                ImGui::SameLine();
            snprintf(buf, 64, "%d,%d", row, col);
            if (ImGui::ImageButton(buf, tilemapTex->TexId(), ImVec2{ 32,32 }, ImVec2{ col * xStep ,(rows - row) * yStep }, ImVec2{ (col + 1) * xStep ,(rows - row - 1) * yStep }))
            {
                selectedSpriteID = row * cols + col;
            }
        }
        ImGui::NewLine();
    }
}

void TilesetEditor::HandleDragDrop()
{
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
            
        {
            std::string texturepath = std::string((const char *)payload->Data);
            if (texturepath.find("Textures\\") == std::string::npos) return;
            selectedTileset->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(texturepath.substr(texturepath.find("Textures\\") + 9));
            // regenerate tileset with new texture
            GenerateTiles(*selectedTileset);
            CEO::Get<ResourceManager>()->SaveTileset(*selectedTileset, selectedTileset->path);
        }
        ImGui::EndDragDropTarget();
    }
}