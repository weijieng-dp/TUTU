/*!
@file       TilesetEditor.h
@author     Ou Yukang (yukang.ou) 100%
@date       11/01/2026
@brief		Handles editing tilesets and rendering ImGui widgets for the editor
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*________________________________________________________________________*/
#pragma once
#include "Components.h" 

class TilesetEditor {
private:
	std::shared_ptr<Tileset> selectedTileset{};
	int newTileSize{ 16 };
	int selectedSpriteID{ -1 };
	char createNewTilemapName[128] = "";
	char setNewTextureName[128] = "";

public:
	bool showWindow{ true };

	/*!
	* \brief
	*   Draws grid for selected tilemap on viewport
	* \param
	*   tilemapEntity - entity containing tilemap component to render grid for
	*/
	void DrawGrid(Registry::Entity tilemapEntity);
	/*!
	* \brief
	*   Draws window for editing tileset
	*/
	void DrawWindow();
	/*!
	* \brief
	*   Automatically subdivides selected tileset textures into tiles of 
	*	equal sizes specified by "newTileSize"
	* \param
	*   tileset - tileset to generate tiles for
	*/
	void GenerateTiles(Tileset& tileset);
	/*!
	* \brief
	*   set the tileset asset to be edited by the window
	* \param
	*   tileset - new selected tileset
	*/
	void SetSelectedTileset(std::shared_ptr<Tileset> tileset);
	/*!
	* \brief
	*   Place selected tile onto tilemap in the specified tilemapEntity
	* \param
	*   tilemapEntity - entity containing tilemap to place tile in
	* \param
	*   mouseX - x position of mouse
	* \param
	*   mouseY - y position of mouse
	*/
	void PlaceSelectedTile(Registry::Entity tilemapEntity, float mouseX, float mouseY);
	/*!
	* \brief
	*   Remove tile from tilemap in the specified tilemapEntity
	* \param
	*   tilemapEntity - entity containing tilemap to remove tile from
	* \param
	*   mouseX - x position of mouse
	* \param
	*   mouseY - y position of mouse
	*/
	void RemoveTile(Registry::Entity tilemapEntity, float mouseX, float mouseY);
	/*!
	* \brief
	*   Draw grid of tiles to select from
	*/
	void DrawPalette();

	/*!
	* \brief
	*   Handle drag drop of texture asset into window to use for tileset
	*/

	void HandleDragDrop();
};