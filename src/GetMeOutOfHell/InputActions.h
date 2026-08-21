/*!
@file       InputAction.h

@author     j.junbo@digipen.edu(jun bo)(16.6%)
@author     weijie.ng@digipen.edu(Ng Wei Jie)(16.6%)
@author     t.junjie@digipen.edu(Jun Jie)(16.6%)
@author     kaedenjiawei.tan@digipen.edu(kaeden)(16.6%)
@author     yukang.ou@digipen.edu(yukang)(16.6%)
@author     mingyang.zhang@digipen.edu(mingyang)(16.6%)

@date       25/09/2025
@brief		Handles all input 


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
/*________________________________________________________________________*/

#pragma once
#include "InputManager.h"
#include "PhysicsSystem.h"
#include "DebugRender.h"
#include "Savestates.h"
#include "Camera.h"
#include "GraphicsSystem.h"
#include "ResourceManager.h"
#include <UIManager.h>

#include "MapManager.h"
#ifdef PLATFORM_WINDOWS

namespace InputAction {
	/*!
	* \brief
	*	Any input based action is to be placed into here.
	*	This is to allow for a more centralized place such that
	*	if an issue arise with any form of input, it will be much
	*	easier to debug.
	*
	* \param null
	*
	* \return null
	*/
	inline void Action() {
		if (Input::IsKeyHeld(GLFW_KEY_LEFT_CONTROL) || Input::IsKeyHeld(GLFW_KEY_RIGHT_CONTROL)) {
#ifdef EditorFlag

			if (Input::IsKeyPressed(GLFW_KEY_I)) {
				CEO::Instance().GetManager<DebugRender>()->debugDrawEnabled = !CEO::Instance().GetManager<DebugRender>()->debugDrawEnabled;
			}

			if (Input::IsKeyPressed(GLFW_KEY_ENTER)) {
				CEO::Instance().GetManager<GraphicsSystem>()->instancingFlag = !CEO::Instance().GetManager<GraphicsSystem>()->instancingFlag;
			}

			// Editor undo-redo
			if (Input::IsKeyPressed(GLFW_KEY_Z)) { Editor::Instance().Undo(); }
			if (Input::IsKeyPressed(GLFW_KEY_Y)) { Editor::Instance().Redo(); }

			// Save scene
			if (Input::IsKeyPressed(GLFW_KEY_S)) { Editor::Instance().Save(); };

			// ====== ImGuizmos Hotkey ========
			// For ImGuizmos hotkey
			// Hotkey for setting ImGuizmos operations to Scale
			if (Input::IsKeyPressed(GLFW_KEY_E)) { Editor::Instance().currentGuizmoOp = ImGuizmo::SCALE; }
			// Hotkey for setting ImGuizmos operations to Rotate
			if (Input::IsKeyPressed(GLFW_KEY_R)) { Editor::Instance().currentGuizmoOp = ImGuizmo::ROTATE; }
			// Hotkey for setting ImGuizmos operations to Translate
			if (Input::IsKeyPressed(GLFW_KEY_T)) { Editor::Instance().currentGuizmoOp = ImGuizmo::TRANSLATE; }
			// ================================

			if (!Input::IsKeyHeld(GLFW_KEY_LEFT_SHIFT)) {
				/*		if (Input::IsKeyPressed(GLFW_KEY_F)) {
					std::cout << "stepping\n";
					ss.StepForward();
				}

				if (Input::IsKeyPressed(GLFW_KEY_B)) {
					ss.StepBackward();
				}*/
			}
			else {
		/*		if (Input::IsKeyPressed(GLFW_KEY_F) || Input::IsKeyHeld(GLFW_KEY_F)) {
					ss.StepForwardSlow();
				}

				if (Input::IsKeyPressed(GLFW_KEY_B) || Input::IsKeyHeld(GLFW_KEY_B)) {
					ss.StepBackwardSlow();
				}*/
			}

			// ---- DO NOT ADD ADDITIONAL CASES TO THIS ---- //
			//if (!Input::IsKeyHeld(GLFW_KEY_LEFT_SHIFT)) {
			//	if (Input::IsKeyPressed(GLFW_KEY_1)) {
			//		ss.CopyState(1);
			//	}
			//	if (Input::IsKeyPressed(GLFW_KEY_2)) {
			//		ss.CopyState(2);
			//	}
			//	if (Input::IsKeyPressed(GLFW_KEY_3)) {
			//		ss.CopyState(3);
			//	}
			//	if (Input::IsKeyPressed(GLFW_KEY_4)) {
			//		ss.CopyState(4);
			//	}
			//	if (Input::IsKeyPressed(GLFW_KEY_5)) {
			//		ss.CopyState(5);
			//	}
			//}
			//else {
			//	if (Input::IsKeyPressed(GLFW_KEY_1)) {
			//		ss.LoadState(1);
			//	}
			//	if (Input::IsKeyPressed(GLFW_KEY_2)) {
			//		ss.LoadState(2);
			//	}
			//	if (Input::IsKeyPressed(GLFW_KEY_3)) {
			//		ss.LoadState(3);
			//	}
			//	if (Input::IsKeyPressed(GLFW_KEY_4)) {
			//		ss.LoadState(4);
			//	}
			//	if (Input::IsKeyPressed(GLFW_KEY_5)) {
			//		ss.LoadState(5);
			//	}
			//}
			// ----------- END OF THE SAVESTATES ---------- //
#endif
		}
		else {
#ifdef EditorFlag
			if (Input::IsKeyHeld(GLFW_KEY_RIGHT_SHIFT) && Input::IsKeyPressed(GLFW_KEY_EQUAL))
			{
				GLApp::ImguiFlag = GLApp::ImguiFlag ? false : true;
				CEO::Get<EventsDispatcher>()->Dispatch<Events::ImGuiHidden>(Events::ImGuiHidden{ !GLApp::ImguiFlag });
			}
			if(Editor::Instance().IsViewportFocused())
				CEO::Instance().GetManager<CameraManager>()->UpdateEditorCameraKeyboardInput(static_cast<float>(GLApp::delta_time));

			if (Editor::Instance().IsViewportHovered())
				CEO::Instance().GetManager<CameraManager>()->UpdateEditorCameraMouseInput();

			if (Input::IsKeyPressed(GLFW_KEY_P)) {
				PhysicsSystem::Instance().SetPause() = (PhysicsSystem::Instance().SetPause() == false);
			}

			if (Input::IsKeyPressed(GLFW_KEY_O)) {
				PhysicsSystem::Instance().SetSingleStep() = (PhysicsSystem::Instance().SetSingleStep() == false);
			}
#endif //  EditorFlag

			if (Input::IsKeyPressed(GLFW_KEY_F11)) {	// change window mode
				CEO::Get<EventsDispatcher>()->Dispatch<Events::ChangeWindowMode>({});
			}
		}
		UIManager::UpdateUIInput();
		UIManager::UpdateControllerNavigation(static_cast<float>(GLApp::delta_time));
		if (Input::IsKeyPressed(GLFW_KEY_Z))
		{
			GLApp::displayFPS = !GLApp::displayFPS;
		}
	}
}
#endif
