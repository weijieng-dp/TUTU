/*!
@file       EditorActions.cpp
@author     Ou Yukang (yukang.ou) 100%
@date       15/11/2025
@brief		Handles editor action history for undo redo functionalities


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/

#include "EditorAction.h"

void EditorActions::Clear()
{
	undoStack.clear();
	redoStack.clear();
}

void EditorActions::PopCache()
{
	if (frontCache.has_value())
	{
		frontCache.reset(); // destroy value in cache
		frontCache.swap(backCache); // bring back cache to the front
	}
	else if (backCache.has_value())
	{
		backCache.reset(); // destroy value in cache
	}
}

void EditorActions::FlushCache()
{
	//LOGI("Flushing");
	frontCache.reset();
	backCache.reset();
	//std::cout << "front[" << (frontCache.has_value() ? "x" : " ") << "]";
	//std::cout << "back [" << (backCache.has_value() ? "x" : " ") << "]\n";
}

void EditorActions::Push(std::unique_ptr<ActionBase> undo)
{
	// push action into undo stack
	undoStack.push_back(std::move(undo));

	// ensure stack doesn't exceed max stack size
	if (undoStack.size() > maxStackSize)
		undoStack.pop_front();

	// clear redo stack when a new actions is pushed
	redoStack.clear();
}
