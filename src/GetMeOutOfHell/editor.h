/*!
@file       editor.h
@author     Kaeden Tan (kaedenjiawei.tan)
@date       10/01/2025
@brief		Declaration of the Editor class, a singleton that manages the
			entire editor UI, including windows for hierarchy, inspector,
			and scene selection.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.
*//*______________________________________________________________________*/
#pragma once
#include "Platform.h"
#ifdef PLATFORM_WINDOWS


#include <string>
#include <vector>
#include <set>
#include "imgui.h"
#ifdef PLATFORM_WINDOWS
#ifdef __has_include
#   if __has_include(<afxole.h>)
#       define _AFXDLL
#       include <afxole.h>
#       undef DELETE // the afx header has delete defined to a value, we have it as an enum here so I'm undefining it as we most likely wont be using their delete macro
#       define HAS_MFC
#   endif
#endif
#endif

#ifdef PLATFORM_WINDOWS
#include <ImGuizmo.h>
#endif
#include "Registry.h"
#include "EditorAction.h"
#include "SceneManager.h"
#include <filesystem>
#include "GameObjects.h"
#include "Components.h"
#include "TilesetEditor.h"

class Editor {
public:
    /*!
    * \brief
    *	Gets the singleton instance of the Editor.
    *
    * \return
    *	A reference to the singleton Editor instance.
    */
    static Editor& Instance();

    // Delete Copy Constructor & Assignment Operator (Singleton)
    Editor(const Editor&) = delete;
    Editor& operator=(const Editor&) = delete;

    /*!
    * \brief
    *	Cleans up and shuts down the ImGui editor context.
    */
    void EditorCleanup();
    /*!
    * \brief
    *	Initializes the editor UI, including ImGui context and backends.
    *
    * \param[in] window_width
    *	- The width of the application window.
    * \param[in] window_height
    *	- The height of the application window.
    * \param[in] componentRegistry
    *	- A reference to the component registry.
    */
    bool InitEditor(float window_width, float window_height, ComponentRegistry& componentRegistry);
    /*!
    * \brief
    *	Draws all editor UI elements for the current frame.
    *
    * \param[in] registry
    *	- A reference to the entity-component registry.
    * \param[in] componentRegistry
    *	- A reference to the component registry.
    */
    void DrawEditor(Registry& registry, ComponentRegistry& componentRegistry, float windowX, float windowY);


    /*!
    * \brief
    *   Represents the current runtime state of the editor.
    */
    enum class EditorState{
        Edit,       //!< Scene editing only
        Play,       //!< Simulation running
        Pause       //!< Simulation paused
    };

    /*!
    * \brief Gets or sets the current editor runtime state.
    */
    EditorState GetState() const { return state; }
    void SetState(EditorState newState) { state = newState; }

    /*!
    * \brief Returns true if the editor should advance the simulation by one frame.
    */
    bool ShouldStepForward() const { return stepForward; }

    /*!
    * \brief Returns true if the editor should step the simulation backwards by one frame.
    */
    bool ShouldStepBackward() const { return stepBackward; }

    /*!
    * \brief Marks that a single forward step should occur next frame.
    */
    void TriggerStepForward() { stepForward = true; }

    /*!
    * \brief Marks that a single backward step should occur next frame.
    */
    void TriggerStepBackward() { stepBackward = true; }

    /*!
    * \brief Clears one-shot step flags.
    */
    void ClearStepFlags() { stepForward = false; stepBackward = false; }

    /*!
    * \brief Undo previous editor action
    */
    void Undo() { actions.Undo(); }

    /*!
    * \brief Redo previous editor action
    */
    void Redo() { actions.Redo(); }

    /*!
    * \brief Redo previous editor action
    */
    void Save() {
        if (!sceneName.empty()) {
			Registry& reg = *CEO::Instance().GetManager<Registry>();
            for (Registry::Entity ent : reg.GetDummyEntities()) {
                reg.DestroyEntity(ent);
            }
            SceneManager::SerializeSceneRJson(reg, sceneName);
#ifdef PLATFORM_WINDOWS
            try {
                // Iterate over directory entries
                for (const auto& entry : std::filesystem::directory_iterator("Assets\\Scenes\\")) {
                    // Check if the entry is a regular file
                    if (std::filesystem::is_regular_file(entry.path())) {
                        UpdateAssetsFolders(entry.path().relative_path().string(), "Scenes");
                    }
                }
            }
            catch (const std::filesystem::filesystem_error& e) {
                LOGE("Error accessing directory: %s", e.what());
            }
#endif
        }
    }

#ifdef PLATFORM_WINDOWS
    ImGuizmo::OPERATION currentGuizmoOp{ ImGuizmo::TRANSLATE };     // to track current ImGuizmo operation
    ImGuizmo::MODE currentGuizmoMode{ ImGuizmo::WORLD };        // to track current ImGuizmo mode
#endif

    bool IsViewportHovered() { return isViewportHovered; }
    bool IsViewportFocused() { return isViewportFocused; }
    bool IsHierarchyFocused() { return isHierarchyFocused; }


    /*!
* \brief
*   Copies/removes data from all assets folders
*
* \param [const std::string&] ABSOLUTE path of the file
* \param [const std::string&] RELATIVE path from Assets/
* \param [bool] if the function is using the filepath to delete or not. If deleting it only looks at relpath and ignores filename.
*/
    void UpdateAssetsFolders(const std::string& filename, const std::string& relpath = "", bool remove = false);


    bool AddedScriptFile = false;
    bool RemoveScriptFile = false;
private:
    using filepath = std::filesystem::path;
    using directory = std::filesystem::directory_entry;

    //Runtime State Variables
    EditorState state = EditorState::Edit;  //Current runtime mode
    bool stepForward = false;               //Step one frame forward when paused
    bool stepBackward = false;              //Step one frame backward when paused

    // Flag indicating if a project is currently open.
    bool projOpen{ false };
    // Stores the dimensions of the main application window.
    ImVec2 windowSize{};

    // Toggles the visibility of the Hierarchy window.
    bool showHierarchyWindow{ true };
    // Toggles the visibility of the Inspector window.
    bool showInspectorWindow{ true };


    std::string sceneName{};

    bool showGuizmos{ false };          // flag to indicate whether ImGuizmos is currently visible

    bool showDebugWindow{ false };      // flag to toggle whether to show debug window

    bool showLayersWindow{ false };     // flag for whether to show layers window

    bool showSceneCreateWindow{ false };     // flag for whether to show layers window

    bool showScriptCreateWindow{ false };     // flag for whether to show layers window
    bool isViewportHovered{ false };
    bool listFormat{ false };

    bool isViewportFocused{ false };
    Vec2 viewportOrigin{};
    Vec2 viewportSize{};

    bool isHierarchyFocused{ false };

	bool displayPrefabHierarchy{ false };
    std::pair<std::string, Registry::Entity> selectedPrefab{};

    // State of component checkboxes in the Inspector window.
    std::map<std::string, bool> componentCheckbox;

	// for text input buffering in ImGui windows
    std::unordered_map<std::string,std::string> bufferedInputStrings{};

    // The ID of the currently selected entity in the editor.
    EntityRegistry::Entity prevSelectedEntityId{}, selectedEntityId{};

    // Undo-Redo handler
    EditorActions actions;

    GameObject copyentity;

    TilesetEditor tilesetEditor{};

    std::string ENGINENAME{ "GAM200-ENGINE" };                      // Name of our github folder. Used as an upper limit for std::filesystems
    filepath mainPath;                                              // Path to the main folder
    filepath ProjectDirectory;                                      // Path to the  project

    std::map<filepath, bool> assetSelectionMap;                     // Storage for the selection.
    std::vector<filepath> assetPath;                                // Path to the multiple asset folders
    std::set<std::string> imgExt{ ".gif",".jpg",".bmp",".png" };    // Common image extensions

    // Handles editor commands like copy, paste, and delete for entities and components.
    class CommandHandler {
    public:
        enum commands {
            NONE = 0,   // No command
            DELETE,     // Delete an item
            PASTE,      // Paste an item from the clipboard
            INSERT      // Insert a new item
        };
    private:
        // Holds the copied data (e.g., an entity or component).
        std::any clipboard{};
        // The type of the data currently in the clipboard.
        std::type_index type{ typeid(void) };

        // The target of the current command (e.g., the entity to paste into or delete).
        std::any target{};
        // The type of the data targeted by the command.
        std::type_index targetType{ typeid(void) };
        
        commands command{NONE};
    public:
        /*!
        * \brief
        *	Gets the currently pending command.
        *
        * \return
        *	The current command enum.
        */
        commands const & Command() const {
            return command;
        }

        /*!
        * \brief
        *	Gets the type of the item currently in the clipboard.
        *
        * \return
        *	The type_index of the clipboard item.
        */
        std::type_index const& ClipboardType() const {
            return type;
        }
        /*!
        * \brief
        *	Gets the type of the item targeted by the current command.
        *
        * \return
        *	The type_index of the target item.
        */
        std::type_index const& TargetType() const {
            return targetType;
        }

        /*!
        * \brief
        *	Clears the current command and target. Optionally clears the clipboard.
        *
        * \param[in] resetClip
        *	- If true, the clipboard will also be cleared.
        */
        void Clear(bool resetClip = false) {
            if (resetClip) {
                // Clear the clipboard content.
                clipboard.reset();
                type = std::type_index{ typeid(void) };
            };

            // Clear the command target.
            target.reset();
            targetType = std::type_index{ typeid(void) };

            command = NONE;
        }
        /*!
        * \brief
        *   Stores a value for deletion operation and sets the command to DELETE.
        *
        * \tparam T
        *   Type of the value to be deleted.
        * \param[in] val
        *   The value to be deleted.
        * \param[in] delType
        *   Type index of the value being deleted.
        */
        template <typename T>
        void Delete(T&& val, std::type_index delType) {
            target = std::any(val);
            targetType = delType;
            command = CommandHandler::commands::DELETE;
        }
        
        /*!
        * \brief
        *   Retrieves the stored value for deletion operation.
        *
        * \tparam T
        *   Type of the value to retrieve.
        * \return
        *   The stored value cast to type T.
        */
        template <typename T>
        T Delete() {
            return std::any_cast<T>(target);
        }

        /*!
        * \brief
        *   Stores a value in the clipboard for copy operation.
        *
        * \tparam T
        *   Type of the value to be copied.
        * \param[in] val
        *   The value to be copied to clipboard.
        * \param[in] cpType
        *   Type index of the value being copied.
        */
        template <typename T>
        void Copy(T&& val, std::type_index cpType) {
            clipboard = std::any(val);
            type = cpType;
            command = CommandHandler::commands::NONE;
        }

        /*!
        * \brief
        *   Stores a target for paste operation and sets the command to PASTE.
        *
        * \tparam T
        *   Type of the target value.
        * \param[in] val
        *   The target value where data will be pasted.
        * \param[in] pasType
        *   Type index of the target value.
        */
        template <typename T>
        void Paste(T&& val, std::type_index pasType) {
            target = std::any(val);
            targetType = pasType;
            command = CommandHandler::commands::PASTE;
        }

        /*!
        * \brief
        *   Retrieves both target and clipboard values for paste operation.
        *
        * \tparam T
        *   Type of the target value.
        * \tparam R
        *   Type of the clipboard value.
        * \return
        *   Pair containing the target value and clipboard value.
        */
        template <typename T, typename R>
        std::pair<T, R> Paste() {
            return std::pair{ std::any_cast<T>(target),std::any_cast<R>(clipboard) };
        }

        /*!
        * \brief
        *   Stores a value for insertion operation and sets the command to INSERT.
        *
        * \tparam T
        *   Type of the value to be inserted.
        * \param[in] val
        *   The value to be inserted.
        * \param[in] pasType
        *   Type index of the value being inserted.
        */
        template <typename T>
        void Insert(T&& val, std::type_index pasType) {
            target = std::any(val);
            targetType = pasType;
            command = CommandHandler::commands::INSERT;
        }

        /*!
        * \brief
        *   Retrieves the stored value for insertion operation.
        *
        * \tparam T
        *   Type of the value to retrieve.
        * \return
        *   Reference to the stored value cast to type T.
        */
        template <typename T>
        T& Insert() {
            return std::any_cast<T>(target);
        }

    } buffer;

    void GuizmoWindow();
    /*!
    * \brief
    *	Draws the Edit Transform window for user to edit the transform
    *   component for selected entity.
    * 
    * \param[in, out] registry
    *   - A reference to the entity-component registry.
    */
    void EditTransformWindow(Registry& registry);

    /*!
    * \brief
    *	Draws the Edit UITransform window for user to edit the UITransform
    *   component for selected entity.
    *
    * \param[in, out] registry
    *   - A reference to the entity-component registry.
    */
    void EditUITransformWindow(Registry& registry);

    /*!
    * \brief Draws a window for various debugging toggles.
    * 
    * \param[in, out] setFocus
    *   - A bool reference on whether to set focus to DebugWindow. Will
    *   automatically set it to false after setting focus.
    */
    void DebugWindow();

    /*!
    * \brief Draws a window for editing layers.
    */
    void EditLayersWindow();

    /*!
    * \brief 
    *   Sets up the ImGui Inputs for editing the bitmask saved in
    *   camera component that is used for rendering culling 
    *   (if camera is main cam).
    * 
    * \param[in, out] registry
    *   - A reference to the entity-component registry.
    *       
    */
    void EditCameraCullingMaskWindow(Registry& registry);

    /*!
    * \brief
    *   Sets up the ImGui window for displaying and editing
    *   layer component.
    *
    * \param[in, out] registry
    *   - A reference to the entity-component registry.
    *
    */
    void EditLayerComponentWindow(Registry& registry);

    /*! 
    * \brief
    *	Sets up the main dockspace for the editor windows.
    */
    void DockspaceWindow();


    /*!
    * \brief
    *	Draws the Inspector window, allowing for component inspection and modification.
    *
    * \param[in] registry
    *	- A reference to the entity-component registry.
    * \param[in] componentRegistry
    *	- A reference to the component registry.
    */
    void InspectorWindow(Registry& registry, ComponentRegistry& componentRegistry);
#ifdef PLATFORM_WINDOWS
    /*!
    * \brief
    *	Draws ImGuizmos for the current selected entity for user to edit the transform.
    *
    * \param[in, out] registry
    *   - A reference to the entity-component registry.
    * \param[in] drawList
    *   - Which ImGui drawlist to draw the ImGuizmo
    * \param[in] imageSize
    *   - Size of the viewport to render the ImGuizmo to.
    * \param[in] imagePos
    *   - The top left position for the viewport to render the ImGuizmo to.
    */
    void EditTransformGuizmos(Registry& registry, ImDrawList* drawList, const ImVec2& imageSize, const ImVec2& imagePos);
    /*!
    * \brief
    *	Draws ImGuizmos for the current selected entity for user to edit the ui transform.
    *
    * \param[in, out] registry
    *   - A reference to the entity-component registry.
    * \param[in] drawList
    *   - Which ImGui drawlist to draw the ImGuizmo
    * \param[in] imageSize
    *   - Size of the viewport to render the ImGuizmo to.
    * \param[in] imagePos
    *   - The top left position for the viewport to render the ImGuizmo to.
    */
    void EditUITransformGuizmos(Registry& registry, ImDrawList* drawList, const ImVec2& imageSize, const ImVec2& imagePos);
#endif // PLATFORM_WINDOWS




    void RuntimeToolbar(Registry& registry, ComponentRegistry& componentRegistry);


    /*!
    * \brief
    *	Draws the Profiler window.
    */
    void ProfilerWindow();

    /*!
    * \brief
    *	Draws the Asset Browser window
    */
    void AssetBrowserWindow(Registry& registry);

    void AssetBuildSizeWindow();

    void InspectorMemberName(PrefabComponent* prefabComp, std::string const& component, std::string const& memberName);



    // Private constructor for singleton design pattern.
    Editor() {} // private constructor for singleton design patter

    // DragDrop implementation
#ifdef HAS_MFC
#define UNUSED_PARAMETER(x) (void)(x)
    friend class DragDrop;
    class DragDrop : public COleDropTarget {
    public:
        DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) override { 
            UNUSED_PARAMETER(pWnd);
            UNUSED_PARAMETER(pDataObject);
            UNUSED_PARAMETER(dwKeyState);
            UNUSED_PARAMETER(point);
            return DROPEFFECT_COPY;
        }
        DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) override { return OnDragOver(pWnd,pDataObject,dwKeyState,point); }
        BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) override;
    };

    DragDrop DropViewer; // to keep track of drag and drop operations
    CWnd window; // provides base functionality for OLE library
#undef UNUSED_PARAMETER
#endif


    /*!
    * \brief
    *   Clears all other asset folders other than mainpath, then proceeds to copy everything back in
    */
    void SyncAssetsFolders();

    /*!
* \brief
*    Draws the window used to create new scripts.
*    The window visibility is controlled by the provided flag.
*
* \param
*    showWindow - Reference flag indicating whether the window should be shown.
*/
    void DrawCreateScriptWindow(bool& showWindow);
    /*!
* \brief
*    Draws a single node in the hierarchy view.
*    This function is typically called recursively or iteratively
*    to render child entities.
*
* \param
*    e - The entity represented by the node.
* \param
*    depth - The current depth level in the hierarchy.
*
* \return
*    [bool] True if the node is expanded or interacted with.
*/
    bool DrawNode(EntityRegistry::Entity e, int depth);

    /*!
* \brief
*    Draws the entity hierarchy starting from the specified root entity
*    using an iterative traversal approach.
*
* \param
*    root - The root entity from which hierarchy rendering begins.
*/
    void DrawHierarchyIterative(EntityRegistry::Entity root);

    /*!
* \brief
*    Generates a new hierarchy layout.
*    This function is typically used to rebuild or refresh the hierarchy
*    structure displayed in the editor.
*/
    void NewHiearchyLayout();
    /*!
* \brief
*    Displays the UI for adding a component to the currently selected entity.
*/
    void AddComponent();

    /*!
* \brief
*    Checks whether a given entity is a descendant of another entity
*    within the hierarchy.
*
* \param
*    possibleChild - The entity that may be a child or descendant.
* \param
*    possibleParent - The entity that may be an ancestor.
*
* \return
*    [bool] True if possibleChild is a descendant of possibleParent.
*/
    bool IsDescendantOf(EntityRegistry::Entity possibleChild, EntityRegistry::Entity possibleParent);
    EntityRegistry::Entity dragEntity = 0;
    EntityRegistry::Entity dropEntity = 0;
};
#endif
