#pragma once
/**___________________________________________________________________________/
@file          Events.h
@brief         Contains the events for EventsDispatcher.

Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*____________________________________________________________________________*/
#include <cstdint>
#include "MathLib.h"

namespace Events{
	struct CollisionEvent {	// this is temporary, for testing
		uint32_t entityA;
		uint32_t entityB;
	};

	struct CollisionEnterEvent {
		uint32_t entityA;
		uint32_t entityB;
	};

	struct CollisionStayEvent {
		uint32_t entityA;
		uint32_t entityB;
	};

	struct CollisionExitEvent {
		uint32_t entityA;
		uint32_t entityB;
	};

	struct TriggerEnterEvent {
		uint32_t trigger;
		uint32_t other;
	};

	struct TriggerStayEvent {
		uint32_t trigger;
		uint32_t other;
	};

	struct TriggerExitEvent {
		uint32_t trigger;
		uint32_t other;
	};

	struct FramebufferResizeEvent {
		int width;
		int height;
	};

	struct CloseWindow { };

	struct SceneChanged {};
	struct JoystickConnected {
		int jid;
	};
	struct JoystickDisconnected {
		int jid;
	};


	struct ScenePushed {};
	struct ScenePop{};

	struct UpdateSelectedEntity
	{
		unsigned int e;
	};


	struct EditorEnterPlay{};

	struct SyncFile { std::string filename; std::string relativepath; };

	struct MouseScroll { float scrollOffset; };

	struct EditorViewportResized { Vec2 newViewportSize; };

	struct EditorViewportMoved { Vec2 newViewportPos; };

	struct ImGuiHidden { bool hidden; };

	struct ChangeWindowMode { };

	// hot x is relative to the left, hot y is relative to the top
	struct ChangeCursor { std::string cursorName; };

	struct EntityModified {		// for when entity gets ADDED or REMOVED
		uint32_t ent;
		uint32_t layerIndex;

		enum class MODIFICATION {
			ADD_ENTITY,
			REMOVE_ENTITY,
			REMOVE_ALL_ENTITY,
			MODIFY_ENTITY_LAYER
		} modification;
	};

	struct UpdateLayersEvent { };	// when a layer gets added or removed

	struct TogglePostProcessEvent { bool isPostProcessOn; };
}