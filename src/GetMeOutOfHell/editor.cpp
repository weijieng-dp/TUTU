/*!
@file       editor.cpp
@author     Kaeden Tan (kaedenjiawei.tan) (70%)
@co-author  Zhang Mingyang (mingyang.zhang) (10%)
@co-author  Tan Jun Jie (t.junjie) (10%)
@co-author  Ng Wei Jie (weijie.ng) (10%)

@date       01/10/2025
@brief		Implementation of the Editor class, which manages the ImGui-based
			user interface for scene hierarchy, entity inspection, and project
			management.


Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

*//*______________________________________________________________________*/
#include "pch.h"
#include "Platform.h"
#ifdef PLATFORM_WINDOWS
#ifdef EditorFlag

#include "editor.h"
#include "glapp.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "implot.h"
#include "implot_internal.h"
#include "imgui_impl_opengl3.h"

#include <stdio.h>
#include "Registry.h"
#include "imgui_stdlib.h"
#include "Savestates.h"
#include "Camera.h"

#include "GraphicsSystem.h"
#include "DebugRender.h"
#include "PhysicsSystem.h"
#include "UIManager.h"
#include "ScreenManager.h"
#include "ScriptingAPI.h"
#include "CEO.h"
#include "SceneManager.h"
#include "HierarchyManager.h"
#include <algorithm>
#include "GameObjects.h"
#include "EventsDispatcher.h"
#include "TilesetEditor.h"
#include "UpdateStackManager.h"
#include "VideoManager.h"
#include <UserSettingsManager.h>
#include "ItemManager.h"

#include "rapidjson/document.h"
#undef GetObject

namespace {
    void OverridePopup(PrefabComponent* prefabComp, std::string const& component, std::string const & memberName) {
        if (prefabComp &&
            prefabComp->overriddenComponents.find(component) != prefabComp->overriddenComponents.end() &&
            prefabComp->overriddenComponents[component] & rtr::TypeInfo::GetByName(component).GetMemberFlag(memberName))
        {
            if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
            {
                ImGui::OpenPopup((component + memberName + "OverridePopup").c_str());
            }
            if (ImGui::BeginPopupContextItem((component + memberName + "OverridePopup").c_str())) {
                if (ImGui::MenuItem(("Revert Override for " + memberName + "##" + component).c_str()))
                {
                    prefabComp->overriddenComponents[component] &= ~rtr::TypeInfo::GetByName(component).GetMemberFlag(memberName);
                    if (prefabComp->overriddenComponents[component] == 0) {
                        prefabComp->overriddenComponents.erase(component);
                    }
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::Text("Reverting will only be reflected on reloading of scene (after save)");
                }
                ImGui::EndPopup();
            }
        }
        
    }
}

// Implementation of the singleton pattern's instance getter.

Editor& Editor::Instance() {
    // Declare a static instance of the Editor, which is created only once.
    static Editor instance;
    return instance;
}
// Initializes the editor UI and ImGui context.
bool Editor::InitEditor(float window_width, float window_height,ComponentRegistry& componentRegistry)
{
    // Initialize checkboxes for each registered component type for the Inspector window.
    for (std::string comp : componentRegistry.GetComponentTypes()) {
        componentCheckbox.insert({ comp, false });
    };
    
    // Store the initial window dimensions.
    windowSize.x = window_width, windowSize.y = window_height;
    // Setup Dear ImGui context
    // Check the ImGui version to ensure compatibility.
    IMGUI_CHECKVERSION();
    // Create the ImGui & ImPlot context.
    ImGui::CreateContext();
#ifdef PLATFORM_WINDOWS
    ImPlot::CreateContext();
#endif
    // Get a reference to the ImGui I/O object.
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    // Set the display size for ImGui to match the application window.
    io.DisplaySize = ImVec2((float)window_width, (float)window_height);
    // Enable keyboard navigation.
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    // Enable gamepad navigation.
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // Enable the docking feature for editor windows.
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.IniFilename = "../../../ini/imgui.ini";

    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;

    // Base Colors
    ImVec4 bgColor = ImVec4(0.10f, 0.105f, 0.11f, 1.00f);
    ImVec4 lightBgColor = ImVec4(0.15f, 0.16f, 0.17f, 1.00f);
    ImVec4 panelColor = ImVec4(0.17f, 0.18f, 0.19f, 1.00f);
    ImVec4 panelHoverColor = ImVec4(0.25f, 0.35f, 0.45f, 1.00f);
    ImVec4 panelActiveColor = ImVec4(0.20f, 0.30f, 0.40f, 1.00f);
    ImVec4 textColor = ImVec4(0.86f, 0.87f, 0.88f, 1.00f);
    ImVec4 textDisabledColor = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    ImVec4 borderColor = ImVec4(0.14f, 0.16f, 0.18f, 1.00f);

    // Text
    colors[ImGuiCol_Text] = textColor;
    colors[ImGuiCol_TextDisabled] = textDisabledColor;

    // Windows
    colors[ImGuiCol_WindowBg] = bgColor;
    colors[ImGuiCol_ChildBg] = bgColor;
    colors[ImGuiCol_PopupBg] = bgColor;
    colors[ImGuiCol_Border] = borderColor;
    colors[ImGuiCol_BorderShadow] = borderColor;

    // Headers
    colors[ImGuiCol_Header] = panelColor;
    colors[ImGuiCol_HeaderHovered] = panelHoverColor;
    colors[ImGuiCol_HeaderActive] = panelActiveColor;

    // Buttons
    colors[ImGuiCol_Button] = panelColor;
    colors[ImGuiCol_ButtonHovered] = panelHoverColor;
    colors[ImGuiCol_ButtonActive] = panelActiveColor;

    // Frame BG
    colors[ImGuiCol_FrameBg] = lightBgColor;
    colors[ImGuiCol_FrameBgHovered] = panelHoverColor;
    colors[ImGuiCol_FrameBgActive] = panelActiveColor;

    // Tabs
    colors[ImGuiCol_Tab] = panelColor;
    colors[ImGuiCol_TabHovered] = panelHoverColor;
    colors[ImGuiCol_TabActive] = panelActiveColor;
    colors[ImGuiCol_TabUnfocused] = panelColor;
    colors[ImGuiCol_TabUnfocusedActive] = panelHoverColor;

    // Title
    colors[ImGuiCol_TitleBg] = bgColor;
    colors[ImGuiCol_TitleBgActive] = bgColor;
    colors[ImGuiCol_TitleBgCollapsed] = bgColor;

    // Scrollbar
    colors[ImGuiCol_ScrollbarBg] = bgColor;
    colors[ImGuiCol_ScrollbarGrab] = panelColor;
    colors[ImGuiCol_ScrollbarGrabHovered] = panelHoverColor;
    colors[ImGuiCol_ScrollbarGrabActive] = panelActiveColor;

    // Checkmark
    colors[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

    // Slider
    colors[ImGuiCol_SliderGrab] = panelHoverColor;
    colors[ImGuiCol_SliderGrabActive] = panelActiveColor;

    // Resize Grip
    colors[ImGuiCol_ResizeGrip] = panelColor;
    colors[ImGuiCol_ResizeGripHovered] = panelHoverColor;
    colors[ImGuiCol_ResizeGripActive] = panelActiveColor;

    // Separator
    colors[ImGuiCol_Separator] = borderColor;
    colors[ImGuiCol_SeparatorHovered] = panelHoverColor;
    colors[ImGuiCol_SeparatorActive] = panelActiveColor;

    // Plot
    colors[ImGuiCol_PlotLines] = textColor;
    colors[ImGuiCol_PlotLinesHovered] = panelActiveColor;
    colors[ImGuiCol_PlotHistogram] = textColor;
    colors[ImGuiCol_PlotHistogramHovered] = panelActiveColor;

    // Text Selected BG
    colors[ImGuiCol_TextSelectedBg] = panelActiveColor;

    // Modal Window Dim Bg
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.10f, 0.105f, 0.11f, 0.5f);

    // Tables
    colors[ImGuiCol_TableHeaderBg] = panelColor;
    colors[ImGuiCol_TableBorderStrong] = borderColor;
    colors[ImGuiCol_TableBorderLight] = borderColor;
    colors[ImGuiCol_TableRowBg] = bgColor;
    colors[ImGuiCol_TableRowBgAlt] = lightBgColor;

    // Styles
    style.FrameBorderSize = 1.0f;
    style.FrameRounding = 2.0f;
    style.WindowBorderSize = 1.0f;
    style.PopupBorderSize = 1.0f;
    style.ScrollbarSize = 12.0f;
    style.ScrollbarRounding = 2.0f;
    style.GrabMinSize = 7.0f;
    style.GrabRounding = 2.0f;
    style.TabBorderSize = 1.0f;
    style.TabRounding = 2.0f;

    // Reduced Padding and Spacing
    style.WindowPadding = ImVec2(5.0f, 5.0f);
    style.FramePadding = ImVec2(4.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 4.0f);
    style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);

 
    // Set the color scheme to a dark theme.
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

#ifdef HAS_MFC
    // Initializing OLE for drag and drop
    if (!AfxOleInit()) {
        std::cout << "AFXOLE initialization failed\n";
    }
    if (OleInitialize(NULL) != S_OK) { 
        std::cout << "OLE initialization failed\n";
    }
    if (!window.Attach(FindWindow(NULL, GLApp::title.c_str()))) {
        std::cout << "CWND initialization failed\n";
    }
    DropViewer.Register(&window);
#endif

    const char* glsl_version = "#version 300 es";
// Initialize the ImGui backend for GLFW on Windows.
#ifdef PLATFORM_WINDOWS
    // Initialize the ImGui backend for GLFW.
    if (!ImGui_ImplGlfw_InitForOpenGL(GLApp::ptr_window, true)) {
        printf("Failed to initialize ImGui GLFW backend");
        assert(false);
        return false;
    }
    ImGuizmo::Enable(true);
    ImGuizmo::SetOrthographic(true);
#endif
    // Initialize the ImGui backend for OpenGL 3.
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        printf("Failed to initialize ImGui OpenGL3 backend");
        assert(false);
        return false;
    }

    bufferedInputStrings.emplace("_CreateSceneBuf", "");
#ifdef PLATFORM_WINDOWS
    // Initializing filesystems 
    mainPath = std::filesystem::current_path();
    if (std::find(mainPath.begin(), mainPath.end(), ENGINENAME) != mainPath.end())
        while (mainPath.filename() != ENGINENAME) mainPath = mainPath.parent_path();

    directory dir(mainPath);
    ProjectDirectory = mainPath;
    std::copy_if(std::filesystem::recursive_directory_iterator(dir.path()), std::filesystem::recursive_directory_iterator(), std::back_inserter(assetPath), [](const directory& d) {
        return d.path().filename() == "Assets";
        });
    // Effectively taking the shortest path as the "main path"
    mainPath = *std::min_element(assetPath.begin(), assetPath.end(), [](const filepath& p1, const filepath& p2) { return p1.compare(p2) < 0; });
#endif
    
    CEO::Get<EventsDispatcher>()->Subscribe<Events::SyncFile>([this](Events::SyncFile event) {UpdateAssetsFolders(event.filename, event.relativepath); });

    std::stringstream assetSelectionBuffer = CEO::Get<FileManager>()->ReadFile("Assets\\assetConfig.json");

    if (!assetSelectionBuffer.str().empty()) {
        rapidjson::Document doc;
        doc.Parse(assetSelectionBuffer.str());
        for (rapidjson::Value::ConstValueIterator itr = doc["ASM"].Begin(); itr != doc["ASM"].End(); ++itr) {
            std::string path = itr->GetObject()["Path"].GetString();
            bool selected = itr->GetObject()["Selected"].GetBool();
            assetSelectionMap.insert({ path,selected });
        }
    }
    std::for_each(std::filesystem::recursive_directory_iterator(mainPath), std::filesystem::recursive_directory_iterator(), [&lm_mainPath = mainPath, &lm_asm = assetSelectionMap](directory dir) {
        if (dir.is_directory()) {
            // do nothing
        }
        else {
            filepath relPath = std::filesystem::relative(dir.path(), lm_mainPath);
            if (lm_asm.find(relPath) == lm_asm.end()) lm_asm.insert({ relPath,false });
        }
        });
    return true;
}


void Editor::DebugWindow() {
    static bool sameColour{ true };
    ImGui::Begin("Debug Menu", &showDebugWindow);
    GraphicsSystem& graphics{ *CEO::Instance().GetManager<GraphicsSystem>() };
    DebugRender& debugRenderer{ *CEO::Instance().GetManager<DebugRender>() };

    if (ImGui::CollapsingHeader("Rendering")) {
        ImGui::SeparatorText("Rendering Mode");     // a toggle on ImGui for changing between render modes
        if (ImGui::RadioButton("Instancing", graphics.instancingFlag ))
            graphics.instancingFlag = true; 
        if (ImGui::RadioButton("Post processing", graphics.postProcessFlag))
            graphics.postProcessFlag = true;
        ImGui::SameLine();
        if (ImGui::RadioButton("Non-Instancing", !graphics.instancingFlag))
            graphics.instancingFlag = false;

        ImGui::SeparatorText("Debug Render");   // A toggle on ImGui for enabling / disabling debug draw
        if(ImGui::RadioButton(debugRenderer.debugDrawEnabled ? "Disable" : "Enable", debugRenderer.debugDrawEnabled))
            debugRenderer.debugDrawEnabled = !debugRenderer.debugDrawEnabled;

        if (debugRenderer.debugDrawEnabled) {    // disply debug draw options if debug draw is enabled
            ImGui::Checkbox("Collision", &debugRenderer.collisionDebugEnabled);  // toggle collision debug draw
            ImGui::SameLine();
            ImGui::Checkbox("Velocity", &debugRenderer.velocityDebugEnabled);    // toggle velocity debug draw
            
            ImGui::Checkbox("UI", &debugRenderer.uiDebugEnabled);               // toggle UI debug draw
            ImGui::SameLine();
            ImGui::Checkbox("UI", &debugRenderer.uiDebugEnabled);               // toggle UI debug draw
            ImGui::SameLine();
            ImGui::Checkbox("Particle", &debugRenderer.particleDebugEnabled);    // toggle particle spawnbox debug draw

            ImGui::Checkbox("Use Same Colour", &sameColour);
            if (sameColour) {
                Color* clr{ &debugRenderer.DebugColour(DebugRender::DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME) };
                ImGui::SameLine();
                if (ImGui::ColorEdit4("Debug Colour", reinterpret_cast<float*>(clr)))
                    debugRenderer.SetDebugColour(DebugRender::DebugTypes::END_OF_DEBUG_TYPES, *clr);
            }
            else {
                ImGui::ColorEdit4("Collision Quad Wireframe Colour", reinterpret_cast<float*>(&debugRenderer.DebugColour(DebugRender::DebugTypes::ENTITY_COLLISION_QUAD_WIREFRAME)));
                ImGui::ColorEdit4("Collision Circle Wireframe Colour", reinterpret_cast<float*>(&debugRenderer.DebugColour(DebugRender::DebugTypes::ENTITY_COLLISION_CIRCLE_WIREFRAME)));
                ImGui::ColorEdit4("Velocity Arrow Colour", reinterpret_cast<float*>(&debugRenderer.DebugColour(DebugRender::DebugTypes::VELOCITY_ARROW)));
                ImGui::ColorEdit4("UI Wireframe Colour", reinterpret_cast<float*>(&debugRenderer.DebugColour(DebugRender::DebugTypes::UI_QUAD_WIREFRAME)));
                ImGui::ColorEdit4("Particle Spawnbox Colour", reinterpret_cast<float*>(&debugRenderer.DebugColour(DebugRender::DebugTypes::PARTICLE_SPAWNBOX_WIREFRAME)));
                ImGui::ColorEdit4("Particle Spawncircle Colour", reinterpret_cast<float*>(&debugRenderer.DebugColour(DebugRender::DebugTypes::PARTICLE_SPAWNCIRCLE_WIREFRAME)));
            }
            
            ImGui::Separator();

            static float width{ debugRenderer.GetLineWidth() };
            if (ImGui::SliderFloat("Line Width", &width, 1.f, 10.f)) debugRenderer.SetLineWidth(width);
            ImGui::SliderFloat2("Velocity Arrow Size", reinterpret_cast<float*>(&debugRenderer.velocityArrowSize), 5.f, 50.f);

        }
        ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
    }
    
    if (ImGui::CollapsingHeader("Physics")) {
        // i think this can be changed into a button for grid display instead, for later
        if(ImGui::Button(PhysicsSystem::Instance().SetPause() ? "Unpause Physics" : "Pause Physics"))
            PhysicsSystem::Instance().SetPause() = !PhysicsSystem::Instance().SetPause();
        if (PhysicsSystem::Instance().SetPause()) {
            ImGui::SameLine();
            if(ImGui::Button("Step Frame"))
                PhysicsSystem::Instance().SetSingleStep() = !PhysicsSystem::Instance().SetSingleStep();
        }

        ImGui::Separator();

        static bool broadPhaseDebugPrint = false;
        static bool physicsDebugPrint = false;
        // Broad-phase print info toggle
        if (ImGui::Checkbox("Broad-Phase Print Info", &broadPhaseDebugPrint)) {
            CollisionSystem::Instance().SetBroadPhaseDebugPrint(broadPhaseDebugPrint);
        }
        // Physics debug print toggle
        if (ImGui::Checkbox("Physics Debug Print", &physicsDebugPrint)) {
            PhysicsSystem::Instance().SetDebugPrint(physicsDebugPrint);
        }

        ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
    }
    ImGui::End();
}

void Editor::EditCameraCullingMaskWindow(Registry& registry) {
    LayerManager& layerManager{ *CEO::Instance().GetManager<LayerManager>() };
    CameraComponent& camera{ *registry.GetComponent<CameraComponent>(selectedEntityId) };
    std::string preview;
    for (int i{}; i < layerManager.MAX_LAYERS; ++i) {
        std::string layer{ layerManager.GetLayerName(i) };
        if (layer == "") continue;
        if (camera.cullingMask & layerManager.GetLayerMask(i)) {
            if (!preview.empty()) preview += ", ";
            preview += layer;
        }
    }

    if (preview.empty()) preview = "None";

    if (ImGui::BeginCombo("Layers", preview.c_str())) {
        for (int i{}; i < layerManager.MAX_LAYERS; ++i) {
            std::string layer{ layerManager.GetLayerName(i) };
            if (layer == "") {
                bool value{};
                ImGui::BeginDisabled(true);
                ImGui::Checkbox((std::string("<Unused>##") + std::to_string(i)).c_str(), &value);
                ImGui::EndDisabled();
            }
            else {
                bool selected{ (camera.cullingMask & layerManager.GetLayerMask(i)) != 0 };
                if (ImGui::Checkbox(layer.c_str(), &selected)) {
                    if (selected) camera.cullingMask |= layerManager.GetLayerMask(i);
                    else camera.cullingMask &= ~layerManager.GetLayerMask(i);
                }
            }
        }
        ImGui::Separator();
        ImGui::Selectable("Edit Layers...", &showLayersWindow);
        ImGui::EndCombo();
    }
}

void Editor::EditLayerComponentWindow(Registry& registry) {
    LayerManager& layerManager{ *CEO::Instance().GetManager<LayerManager>() };
    LayerComponent& layerComp{ *registry.GetComponent<LayerComponent>(selectedEntityId) };
    std::string preview;
    if (ImGui::BeginCombo("Entity Layer", layerManager.GetLayerName(layerComp.layer).c_str())) {
        for (GLuint i{}; i < layerManager.MAX_LAYERS; ++i) {    // iterate through all layers
            std::string layerName{ layerManager.GetLayerName(i) };  // get the layer name
            if (layerName.empty()) continue;                          // if layer name is empty, skip
                
            if (layerManager.CollisionEnabled(i, layerComp.layer)) {    // check if collision is enabled with entity's layer
                if (!preview.empty()) preview += ", ";
                preview += layerName;
            }

            bool isSelected{ layerComp.layer == i };        // flag for whether current layer is selected
            if (ImGui::Selectable(layerName.c_str(), isSelected)) {     // if current layer is selected
                layerComp.layer = i;                                // update entity's layer
                CEO::Get<EventsDispatcher>()->Dispatch<Events::EntityModified>(Events::EntityModified{ selectedEntityId, i, Events::EntityModified::MODIFICATION::MODIFY_ENTITY_LAYER });
                if(PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                    PrefabManager::OverrideMember(*prefabComp, "LayerComponent", "layer");
				}
            }

            if (isSelected) ImGui::SetItemDefaultFocus();   // set focus to this layer if it is selected
        }
        ImGui::EndCombo();
    } 
    else {  // otherwise iterate to get preview string if Entity Layer drop box is not open
        for (int i{}; i < layerManager.MAX_LAYERS; ++i) {
            std::string layerName{ layerManager.GetLayerName(i) };
            if (layerName.empty()) continue;        // if layer name is empty, skip

            if (layerManager.CollisionEnabled(i, layerComp.layer)) {    // check if collision is enabled with entity's layer
                if (!preview.empty()) preview += ", ";
                preview += layerName;
            }
        }
    }

    if (preview.empty()) preview = "None";
    ImGui::Spacing();
    // =================== for choosing which layer the current entity's collision layer can interact with =========
    if (ImGui::BeginCombo("Layers Interactions", preview.c_str())) {
        for (int i{}; i < layerManager.MAX_LAYERS; ++i) {       // iterate through each layer
            std::string layerName{ layerManager.GetLayerName(i) };  // get the name of the layer
            if (layerName.empty()) {        // if layer name is empty, this means it has not been set up yet
                bool value{};
                ImGui::BeginDisabled(true);
                ImGui::Checkbox((std::string("<Unused>##") + std::to_string(i)).c_str(), &value);
                ImGui::EndDisabled();
            }
            else {
                bool selected{ layerManager.CollisionEnabled(i, layerComp.layer) };
                if (ImGui::Checkbox(layerName.c_str(), &selected)) {
                    layerManager.SetCollisionEnabled(i, layerComp.layer, selected);
                }
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::Button("Edit Layers...")) { showLayersWindow = !showLayersWindow; }
}

void Editor::EditLayersWindow() {
    LayerManager& layerManager{ *CEO::Instance().GetManager<LayerManager>() };
    ImGui::Begin("Layers Configuration", &showLayersWindow);

    if (ImGui::Button("Save Layers")) { // for saving layers to config json
        json config("Assets/config.json");              // get the config file
        layerManager.UpdateLayers(config);              // update the layers config in config file with current layers
        config.Serialize("Assets/config.json");         // serialise the config file
        UpdateAssetsFolders("Assets/config.json");      // update all the asset folders
        CEO::Get<EventsDispatcher>()->Dispatch<Events::UpdateLayersEvent>({});
    }

    if (ImGui::BeginTable("Layers", 3, ImGuiTableFlags_Borders)) {
        ImGui::TableSetupColumn("Layer No.");
        ImGui::TableSetupColumn("Layer Name");
        ImGui::TableSetupColumn("Layer Interactions");

        ImGui::TableHeadersRow();
        for (int i{}; i < layerManager.MAX_LAYERS; ++i) {
            ImGui::TableNextRow();

            ImGui::SetNextItemWidth(-1.f);      // make input take up the entire table cell
            ImGui::TableSetColumnIndex(0);      // cell is row i, column 0
            ImGui::Text("Layer %d", i);

            ImGui::SetNextItemWidth(-1.f);      // make input take up the entire table cell
            ImGui::TableSetColumnIndex(1);      // cell is row i, column 1
            std::string layerName{ layerManager.GetLayerName(i)};
            if(ImGui::InputText(("##1" + std::to_string(i)).c_str(), &layerName))
				layerManager.RenameLayer(i, layerName);

            ImGui::SetNextItemWidth(-1.f);     // make input take up the entire table cell
            ImGui::TableSetColumnIndex(2);      // cell is row 2, column 2
            std::string layers;
            for (int j{}; j < layerManager.MAX_LAYERS; ++j) {
                std::string layer{layerManager.GetLayerName(j)};
                if (layer.empty()) continue;

                if (layerManager.CollisionEnabled(i, j)) {
                    if (!layers.empty()) layers += ", ";
                    layers += layer;
                }
            }
            if (ImGui::BeginCombo(("##2" + std::to_string(i)).c_str(), layers.c_str())) {
                for (int j{}; j < layerManager.MAX_LAYERS; ++j) {       // iterate through each layer
                    std::string layer{ layerManager.GetLayerName(j) };  // get the name of the layer
                    if (layer.empty()) {        // if layer name is empty, this means it has not been set up yet
                        bool value{};
                        ImGui::BeginDisabled(true);
                        ImGui::Checkbox((std::string("<Unused>##") + std::to_string(j)).c_str(), &value);
                        ImGui::EndDisabled();
                    }
                    else {
                        bool selected{ layerManager.CollisionEnabled(i, j) };
                        if (ImGui::Checkbox(layer.c_str(), &selected)) {
                            layerManager.SetCollisionEnabled(i, j, selected);
                        }
                    }
                }
                ImGui::EndCombo();
            }
        }

        ImGui::EndTable();
    }
    ImGui::End();
}

#ifdef PLATFORM_WINDOWS
void Editor::EditTransformGuizmos(Registry& registry, ImDrawList* drawList, const ImVec2& imageSize, const ImVec2& imagePos) {
    if(registry.GetComponent<PrefabDummyMetatag>(selectedEntityId)) return;

    ImGuizmo::BeginFrame();     // start of ImGuizmo
    static bool currUsing = false, prevUsing = false; // if gizmo is currently active or active in prev frame
    // get transform component of the selected entity
    TransformComponent* transform{ registry.GetComponent<TransformComponent>(selectedEntityId) };
    HierarchyComponnent* HC{ registry.GetComponent<HierarchyComponnent>(selectedEntityId) };
    if (!transform) // if transform component doesn't exist
        return;

    Mat3 const& viewMat3{ CEO::Instance().GetManager<CameraManager>()->GetView() };        // get view matrix from camera
    Mat3 const& projMat3 { CEO::Instance().GetManager<CameraManager>()->GetProjection()};   // get projection matrix from camera

    float view[16] {    // re-construct view matrix as a 4x4 matrix (ImGuizmos works with 4x4 matrices)
        1.f,                0.f,            0.f,    0.f,
        0.f,                1.f,            0.f,    0.f,
        0.f,                0.f,            1.f,    0.f,
        viewMat3.m[6],      viewMat3.m[7],  0.f,    1.f
    };

    float proj[16]{     // re-construct projection matrix as a 4x4 matrix (ImGuizmos works with 4x4 matrices)
        projMat3.m[0],      0.f,           0.f,     0.f,
        0.f,                projMat3.m[4], 0.f,     0.f,
        0.f,                0.f,           1.f,     0.f,
        0.f,                0.f,           0.f,     1.f
    };



    if (HC->parent == 0)
    {


        float model[16]{    // get the model transform for selected entity
            transform->transform.m[0], transform->transform.m[1] , 0.f, transform->transform.m[2],
            transform->transform.m[3], transform->transform.m[4], 0.f, transform->transform.m[5],
            0.f, 0.f, 1.f, 0.f,
            transform->transform.m[6] , transform->transform.m[7], 0.f, transform->transform.m[8]
        };

        ImGuizmo::SetDrawlist(drawList);        // draw imguizmos on specified ImGui drawlist (on the editor)
        ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);    // where to draw the imguizmos

        ImGuizmo::Manipulate(   // update model matrix based on ImGuizmo manipulation
            view,               // the view matrix
            proj,               // the projection matrix
            currentGuizmoOp,    // the operation for imguizmos to perform (scale, rotate, translate)
            currentGuizmoMode,  // the mode for imguizmos (Local or World)
            model               // the model matrix of the selected entity to manipulate
        );
        currUsing = ImGuizmo::IsUsing();
        if (currUsing) {      // if user is using the imguizmos, update the entity's info
            transform->translate = { model[12], model[13] };    // update entity's transform
            transform->scale = {                                // update entity's scale
                sqrtf(model[0] * model[0] + model[1] * model[1]),
                sqrtf(model[4] * model[4] + model[5] * model[5])
            };
            transform->rotation = atan2(model[1], model[0]);    // update entity's rotation
            if (PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                switch (currentGuizmoOp) {
                    case ImGuizmo::TRANSLATE:
                        PrefabManager::OverrideMember(*prefabComp, "TransformComponent", "translate");
						break;
                    case ImGuizmo::ROTATE:
						PrefabManager::OverrideMember(*prefabComp, "TransformComponent", "rotation");
                        break;
                    case ImGuizmo::SCALE:
						PrefabManager::OverrideMember(*prefabComp, "TransformComponent", "scale");
                        break;
                }
            }
        }

    }
    else
    {


        float model[16]{    // get the model transform for selected entity
            transform->transform.m[0], transform->transform.m[1] , 0.f, transform->transform.m[2],
            transform->transform.m[3], transform->transform.m[4], 0.f, transform->transform.m[5],
            0.f, 0.f, 1.f, 0.f,
            transform->transform.m[6] , transform->transform.m[7], 0.f, transform->transform.m[8]
        };


        ImGuizmo::SetDrawlist(drawList);        // draw imguizmos on specified ImGui drawlist (on the editor)
        ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);    // where to draw the imguizmos

        ImGuizmo::Manipulate(   // update model matrix based on ImGuizmo manipulation
            view,               // the view matrix
            proj,               // the projection matrix
            currentGuizmoOp,    // the operation for imguizmos to perform (scale, rotate, translate)
            ImGuizmo::LOCAL,  // the mode for imguizmos (Local or World)
            model               // the model matrix of the selected entity to manipulate
               
        );

        Mat3 WorldMat = Mat3();
        WorldMat.m[0] = model[0];
        WorldMat.m[3] = model[4];
        WorldMat.m[6] = model[12];

        WorldMat.m[1] = model[1];
        WorldMat.m[4] = model[5];
        WorldMat.m[7] = model[13];

        WorldMat.m[2] = model[3];
        WorldMat.m[5] = model[7];
        WorldMat.m[8] = model[15];

        TransformComponent* ParentTransform{ registry.GetComponent<TransformComponent>(HC->parent) };

        Mat3 LocalMat = ParentTransform->transform.Inversed() * WorldMat;


        currUsing = ImGuizmo::IsUsing();
        if (currUsing) {      // if user is using the imguizmos, update the entity's info
            transform->translate = { LocalMat.m[6] , LocalMat.m[7]  };    // update entity's transform
            transform->scale = {                                // update entity's scale
                sqrtf(LocalMat.m[0] * LocalMat.m[0] + LocalMat.m[1] * LocalMat.m[1]),
                sqrtf(LocalMat.m[3] * LocalMat.m[3] + LocalMat.m[4] * LocalMat.m[4])
            };
            transform->rotation = atan2(LocalMat.m[1], LocalMat.m[0]);    // update entity's rotation
        }

    }



    // state changed!
    if (currUsing != prevUsing)
    {
        if (currUsing) //turned active
            actions.Cache(*transform);
        else // turned inactive
        {
            actions.Push(*transform);

        }
    }
    prevUsing = currUsing;
}

void Editor::EditUITransformGuizmos(Registry& registry, ImDrawList* drawList, const ImVec2& imageSize, const ImVec2& imagePos) {
    ImGuizmo::BeginFrame();     // start of ImGuizmo
    static bool currUsing = false, prevUsing = false; // if gizmo is currently active or active in prev frame
    // get transform component of the selected entity
    UITransformComponent* transform{ registry.GetComponent<UITransformComponent>(selectedEntityId) };
    if (!transform) // if transform component doesn't exist
        return;

    Mat3 const& viewMat3{ CEO::Instance().GetManager<CameraManager>()->GetView() };        // get view matrix from camera
    Mat3 const& projMat3{ CEO::Instance().GetManager<CameraManager>()->GetProjection() };   // get projection matrix from camera

    float view[16]{    // re-construct view matrix as a 4x4 matrix (ImGuizmos works with 4x4 matrices)
        1.f,                0.f,            0.f,    0.f,
        0.f,                1.f,            0.f,    0.f,
        0.f,                0.f,            1.f,    0.f,
        viewMat3.m[6],      viewMat3.m[7],  0.f,    1.f
    };

    float proj[16]{     // re-construct projection matrix as a 4x4 matrix (ImGuizmos works with 4x4 matrices)
        projMat3.m[0],      0.f,           0.f,     0.f,
        0.f,                projMat3.m[4], 0.f,     0.f,
        0.f,                0.f,           1.f,     0.f,
        0.f,                0.f,           0.f,     1.f
    };

    float cosR{ cosf(transform->rotation) };    // pre-calculate cosine of rotation
    float sinR{ sinf(transform->rotation) };    // pre-calculate sine of rotation

    float model[16]{    // get the model transform for selected entity
        transform->size.x * cosR, transform->size.x * sinR, 0.f, 0.f,
        -transform->size.y * sinR, transform->size.y * cosR, 0.f, 0.f,
        0.f, 0.f, 1.f, 0.f,
        transform->relativePos.x, transform->relativePos.y, 0.f, 1.f
    };

    ImGuizmo::SetDrawlist(drawList);        // draw imguizmos on specified ImGui drawlist (on the editor)
    ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);    // where to draw the imguizmos

    ImGuizmo::Manipulate(   // update model matrix based on ImGuizmo manipulation
        view,               // the view matrix
        proj,               // the projection matrix
        currentGuizmoOp,    // the operation for imguizmos to perform (scale, rotate, translate)
        currentGuizmoMode,  // the mode for imguizmos (Local or World)
        model               // the model matrix of the selected entity to manipulate
    );

    currUsing = ImGuizmo::IsUsing();
    if (currUsing) {      // if user is using the imguizmos, update the entity's info
        transform->relativePos = { model[12], model[13] };    // update entity's transform
        transform->size = {                                // update entity's scale
            sqrtf(model[0] * model[0] + model[1] * model[1]),
            sqrtf(model[4] * model[4] + model[5] * model[5])
        };
        transform->rotation = atan2(model[1], model[0]);    // update entity's rotation

        if (PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
            switch (currentGuizmoOp) {
            case ImGuizmo::TRANSLATE:
                PrefabManager::OverrideMember(*prefabComp, "UITransformComponent", "relativePos");
                break;
            case ImGuizmo::ROTATE:
                PrefabManager::OverrideMember(*prefabComp, "UITransformComponent", "rotation");
                break;
            case ImGuizmo::SCALE:
                PrefabManager::OverrideMember(*prefabComp, "UITransformComponent", "size");
                break;
            }
        }

    }
    // state changed!
    if (currUsing != prevUsing)
    {
        if (currUsing) //turned active
            actions.Cache(*transform);
        else // turned inactive
            actions.Push(*transform);
    }
    prevUsing = currUsing;
    }
#endif


void Editor::GuizmoWindow()
{
#ifdef PLATFORM_WINDOWS
    ImGui::SeparatorText("Guizmo Operation");       // Changing ImGuizmo operation through ImGui
    if (ImGui::RadioButton("Scale", currentGuizmoOp == ImGuizmo::SCALE)) currentGuizmoOp = ImGuizmo::SCALE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Rotate", currentGuizmoOp == ImGuizmo::ROTATE)) currentGuizmoOp = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::RadioButton("Translation", currentGuizmoOp == ImGuizmo::TRANSLATE)) currentGuizmoOp = ImGuizmo::TRANSLATE;

    ImGui::SeparatorText("Guizmo Mode");            // Changing ImGuizmo mode through ImGui
    if (ImGui::RadioButton("World", currentGuizmoMode == ImGuizmo::WORLD)) currentGuizmoMode = ImGuizmo::WORLD;
    ImGui::SameLine();
    if (ImGui::RadioButton("Local", currentGuizmoMode == ImGuizmo::LOCAL)) currentGuizmoMode = ImGuizmo::LOCAL;
#endif
}

void Editor::EditTransformWindow(Registry& registry) {
    // get transform component of the selected entity
    TransformComponent* transform{ registry.GetComponent<TransformComponent>(selectedEntityId) };
    //HierarchyComponnent* HC{ registry.GetComponent<HierarchyComponnent>(selectedEntityId) };
    if (transform != nullptr) { // check if transform component exists
#ifdef PLATFORM_WINDOWS
        GuizmoWindow();
#endif // PLATFORM_WINDOWS

        PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId);
        // We have to specially store values for sliders because they modify the value as soon as it activates
        // which breaks default handling of undo redo action value caching

        InspectorMemberName(prefabComp, "TransformComponent", "scale");
        float oldRotationValue{};
        ImGui::InputFloat2("Scale [x, y]", reinterpret_cast<float*>(&transform->scale));            // Update scale value
        if (actions.TryCacheOrPush(transform->scale) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                PrefabManager::OverrideMember(*m_prefabComp, "TransformComponent", "scale");
            }
        }

        InspectorMemberName(prefabComp, "TransformComponent", "rotation");
        oldRotationValue = transform->rotation;
        ImGui::SliderAngle("Rotation", &transform->rotation);                                       // update rotation value
        if (actions.TryCacheOrPush(transform->rotation, oldRotationValue) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                    PrefabManager::OverrideMember(*m_prefabComp, "TransformComponent", "rotation");
            }
        }

        InspectorMemberName(prefabComp, "TransformComponent", "translate");
        ImGui::InputFloat2("Translate [x, y]", reinterpret_cast<float*>(&transform->translate));    // update translate value
        if (actions.TryCacheOrPush(transform->translate) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                    PrefabManager::OverrideMember(*m_prefabComp, "TransformComponent", "translate");
            }
        }
    }
}

void Editor::EditUITransformWindow(Registry& registry) {
    UITransformComponent* transform{ registry.GetComponent<UITransformComponent>(selectedEntityId) };
    if (transform != nullptr) {
#ifdef PLATFORM_WINDOWS
        GuizmoWindow();
#endif // PLATFORM_WINDOWS

        PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId);
        // We have to specially store values for sliders because they modify the value as soon as it activates
        // which breaks default handling of undo redo action value caching
        float floatValueCache{};
        Vec2 vec2ValueCache{};

        // Update scale value
        InspectorMemberName(prefabComp, "UITransformComponent", "size");
        ImGui::DragFloat2("size [x, y]", reinterpret_cast<float*>(&transform->size));
        if (actions.TryCacheOrPush(transform->size) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                PrefabManager::OverrideMember(*m_prefabComp, "UITransformComponent", "size");
            }
        }

        InspectorMemberName(prefabComp, "UITransformComponent", "rotation");
        floatValueCache = transform->rotation;
        ImGui::SliderAngle("Rotation", &transform->rotation);   // update rotation value
        if (actions.TryCacheOrPush(transform->rotation, floatValueCache) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                PrefabManager::OverrideMember(*m_prefabComp, "UITransformComponent", "rotation");
            }
        }

        InspectorMemberName(prefabComp, "UITransformComponent", "min");
        vec2ValueCache = transform->min;
        ImGui::SliderFloat2("Min [x, y]", reinterpret_cast<float*>(&transform->min), -0.5, 0.5);
        if (actions.TryCacheOrPush(transform->min, vec2ValueCache) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                PrefabManager::OverrideMember(*m_prefabComp, "UITransformComponent", "min");
            }
        }

        InspectorMemberName(prefabComp, "UITransformComponent", "max");
        vec2ValueCache = transform->max;
        ImGui::SliderFloat2("Max [x, y]", reinterpret_cast<float*>(&transform->max), -0.5, 0.5);
        if (actions.TryCacheOrPush(transform->max, vec2ValueCache) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                PrefabManager::OverrideMember(*m_prefabComp, "UITransformComponent", "max");
            }
        }

        InspectorMemberName(prefabComp, "UITransformComponent", "anchor");
        vec2ValueCache = transform->anchor;
        ImGui::SliderFloat2("Anchor [x, y]", reinterpret_cast<float*>(&transform->anchor), 0.f, 1.f);
        if (actions.TryCacheOrPush(transform->anchor, vec2ValueCache) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                PrefabManager::OverrideMember(*m_prefabComp, "UITransformComponent", "anchor");
            }
        }

        InspectorMemberName(prefabComp, "UITransformComponent", "relativePivot");
        vec2ValueCache = transform->relativePivot;
        ImGui::SliderFloat2("Pivot [x, y]", reinterpret_cast<float*>(&transform->relativePivot), -1, 1);
        if (actions.TryCacheOrPush(transform->relativePivot, vec2ValueCache) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                PrefabManager::OverrideMember(*m_prefabComp, "UITransformComponent", "relativePivot");
            }
        }

        InspectorMemberName(prefabComp, "UITransformComponent", "relativePos");
        vec2ValueCache = transform->relativePos;
        ImGui::DragFloat2("Position [x, y]", reinterpret_cast<float*>(&transform->relativePos));
        if (actions.TryCacheOrPush(transform->relativePos, vec2ValueCache) & EditorActions::CachePushResult_Pushed) {
            if (PrefabComponent* m_prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId)) {
                PrefabManager::OverrideMember(*m_prefabComp, "UITransformComponent", "relativePos");
            }
        }
    }
}

void DrawConsole()
{
    ImGui::Begin("Console");

    // Toggle bar
    ImGui::Checkbox("Auto-scroll", &(CEO::Instance().GetManager<Logger>()->autoScroll));
    ImGui::SameLine();
    ImGui::Checkbox("Stacked", &(CEO::Instance().GetManager<Logger>()->Stacked));
    ImGui::SameLine();
    if (ImGui::Button("Clear")) CEO::Instance().GetManager<Logger>()->GetLogs().clear();

    ImGui::Separator();

    // Console scroll region
    ImGui::BeginChild("ConsoleRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& e : CEO::Instance().GetManager<Logger>()->GetLogs())
    {
        ImVec4 color;

        switch (e.level)
        {
            case Logger::LogLevel::Info:    color = { 1,1,1,1 }; break;
            case Logger::LogLevel::Warning: color = { 1,1,0,1 }; break;
            case Logger::LogLevel::Debug: color = { 0,1,0,1 }; break;
            case Logger::LogLevel::Error:   color = { 1,0,0,1 }; break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        if (CEO::Instance().GetManager<Logger>()->Stacked)
        {
            if (e.count > 1)
                ImGui::Text("%s (x%d)", e.message.c_str(), e.count);
            else
                ImGui::TextUnformatted(e.message.c_str());
        }
        else
        {
            for (int i = 0 ; i < e.count; i ++)
            {
                ImGui::TextUnformatted(e.message.c_str());
            }
        }

        ImGui::PopStyleColor();
    }

    // Auto-scroll to bottom
    if (CEO::Instance().GetManager<Logger>()->autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

bool IsValidScriptName(const char* name)
{
    if (!name || name[0] == '\0')
        return false;

    if (!(std::isalpha(name[0]) || name[0] == '_'))
        return false;

    for (const char* c = name; *c; ++c)
    {
        if (!(std::isalnum(*c) || *c == '_'))
            return false;
    }

    return true;
}

bool ScriptExists(const std::string& scriptName, const std::string& basepath)
{
    std::filesystem::path base = basepath; // adjust to your path
    return std::filesystem::exists(base / (scriptName + ".h")) ||
        std::filesystem::exists(base / (scriptName + ".cpp"));
}

void DrawItemManager(Registry&)
{
    ImGui::Begin("Item Manager");
    if (ImGui::TreeNodeEx("Stats")) {
        for (auto& stat : CEO::Get<StatsManager>()->StatsView()) {
			ImGui::Text("%s: %s", stat.first.c_str(), stat.second.c_str());
            if (ImGui::Button(("+##"+stat.first).c_str())) {
				CEO::Get<StatsManager>()->GetByName(stat.first)->operator+=(1);
            }
            ImGui::SameLine(); 
            if(ImGui::Button(("-##" + stat.first).c_str())) {
				CEO::Get<StatsManager>()->GetByName(stat.first)->operator-=(1);
            }
        };
        if (ImGui::Button("Reset")) {
            CEO::Get<StatsManager>()->ResetPlayer();
            CEO::Get<ItemManager>()->ClearActiveItems();
        }
        ImGui::TreePop();
    }
    for (auto& item : CEO::Get<ItemManager>()->GetItemList()) {
        if (ImGui::TreeNodeEx(item.first.c_str())) {
            ImGui::Text("Description: %s", item.second.description.c_str());
            if (ImGui::Button(("Add item##" + item.first).c_str()))
            {
                CEO::Get<ItemManager>()->AddItem(item.second, *CEO::Get<StatsManager>());
            }
            ImGui::TreePop();
        }

    }
    ImGui::End();
}
void Editor::DrawCreateScriptWindow(bool& showWindow)
{
    static char scriptName[128] = "";

    if (!showWindow)
        return;

    ImGui::Begin("Create Script", &showWindow,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Text("New Script");
    ImGui::Separator();

    ImGui::InputText("Script Name", scriptName, sizeof(scriptName));

    bool validName = IsValidScriptName(scriptName);
    filepath ScriptsDirectory = ProjectDirectory;
    ScriptsDirectory /= "src";
    ScriptsDirectory /= "Scripts";
    bool exists = validName && ScriptExists(scriptName, ScriptsDirectory.string());

    if (!validName)
    {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1),
            "Invalid name (letters, digits, '_' only)");
    }
    else if (exists)
    {
        ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1),
            "A script with this name already exists");
    }

    ImGui::Spacing();

    ImGui::BeginDisabled(!validName || exists);

    if (ImGui::Button("Create"))
    {

        std::ofstream h(ScriptsDirectory / (std::string(scriptName) + ".h"));
        std::ofstream cpp(ScriptsDirectory / (std::string(scriptName) + ".cpp"));

        std::string headerfile =
            R"(
#pragma once
#include "../CoreLib/Components.h"
#include "../CoreLib/ScriptingAPI.h"


class )" + std::string(scriptName) +

R"(: public ScriptInstance
{
public:
	void BindFrom() { 
        GetComponent<)" + std::string(scriptName) + R"(>(*CEO::Get<Registry>())->entity = entity;
        *this = *GetComponent<)" + std::string(scriptName) + R"(>(*CEO::Get<Registry>()); };
	void BindTo() { *GetComponent<)" + std::string(scriptName) + R"(>(*CEO::Get<Registry>()) = *this; };

    void OnStart(Registry& registry);
    void OnUpdate(Registry& registry,float dt, bool firstframe);
    void OnFixedUpdate(Registry& registry,float dt, bool firstframe);
   
    REFLECTABLE_PROPERTIES;
};
REFL_AUTO(
    type()" + std::string(scriptName) + ")\n)";

        h << headerfile;




        cpp << "#include \"" << scriptName << ".h\"\n";

        std::string cppfiles =

            R"(void )" + std::string(scriptName) + R"(::OnStart(Registry & r)
{
};


void )" + std::string(scriptName) + R"(::OnUpdate(Registry & registry, float dt, bool firstframe)
{
};

void )" + std::string(scriptName) + R"(::OnFixedUpdate(Registry&, float, bool)
{
};
)";
        cpp << cppfiles;

        h.close();
        cpp.close();
        AddedScriptFile = true;
        scriptName[0] = '\0';
        showWindow = false;
    }

    ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Cancel"))
    {
        scriptName[0] = '\0';
        showWindow = false;
    }

    ImGui::End();
}

// Draws all the editor UI elements for the current frame.
void Editor::DrawEditor(Registry& registry, ComponentRegistry& componentRegistry, float windowX, float windowY)
{
    prevSelectedEntityId = selectedEntityId;
    if (!displayPrefabHierarchy && selectedEntityId!=selectedPrefab.second && selectedPrefab.second != 0) {
        for (Registry::Entity ent : registry.GetDummyEntities()) {

            registry.DestroyEntity(ent);
        }
        selectedPrefab.first.clear();
        selectedPrefab.second = 0;
    }
    //if (hierarchySelection.size() == 1) selectedEntityId = hierarchySelection[0];
    //else {
    //    selectedEntityId = 0;
    //    showGuizmos = false;
    //}
    windowSize.x = windowX; windowSize.y = windowY;

    // Define a clear color for the background (currently unused in this function).
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Prepare ImGui for a new frame using the OpenGL 3 backend.
    ImGui_ImplOpenGL3_NewFrame();

    // Prepare ImGui for a new frame using the GLFW backend on Windows.
#ifdef PLATFORM_WINDOWS
    ImGui_ImplGlfw_NewFrame();
#endif
    // Start a new ImGui frame.
    ImGui::NewFrame();
    DockspaceWindow();

    if (Input::IsKeyHeld(GLFW_KEY_LEFT_CONTROL) && Input::IsKeyPressed(GLFW_KEY_C))
    {
        if (selectedEntityId)
        {
            copyentity = selectedEntityId;
        }
    }
    if (Input::IsKeyHeld(GLFW_KEY_LEFT_CONTROL) && Input::IsKeyPressed(GLFW_KEY_V))
    {
        if (copyentity.IsValid() && (IsViewportFocused() || IsHierarchyFocused()))
            copyentity.Instantiate();
    }
    ImGui::Begin("Viewport");
    isViewportHovered = ImGui::IsWindowHovered();
    isViewportFocused = ImGui::IsWindowFocused();
    
    CEO::Get<CameraManager>()->suppressEditorCameraMouseScroll = !isViewportHovered;

    ImVec2 displaySize{ ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x, 
        ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y };
    float resizedX = GLApp::ar * displaySize.y;
    // resizing with original displayY exceeds window display
    if (resizedX > displaySize.x) {
        displaySize.y = 1.f / GLApp::ar * displaySize.x;
    }
    else displaySize.x = resizedX;

    ImGui::Image(ScreenManager::screenBuffer.ColorBufferTex(), displaySize, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
#ifdef PLATFORM_WINDOWS
    ImDrawList* editorDrawList{ ImGui::GetWindowDrawList() };   // get the drawlist of the editor to pass into EditTransformWindows()
    ImVec2 newViewportSize{ ImGui::GetItemRectSize() };       // get the size of the drawn viewport
    ImVec2 newViewportOrigin{ ImGui::GetItemRectMin() };         // get the (top-left) position of the drawn viewport
    if (viewportOrigin.x != newViewportOrigin.x || viewportOrigin.y != newViewportOrigin.y)
    {
        viewportOrigin = Vec2{ newViewportOrigin.x, newViewportOrigin.y };
		CEO::Get<EventsDispatcher>()->Dispatch<Events::EditorViewportMoved>(Events::EditorViewportMoved{ viewportOrigin });
    }
    if (viewportSize.x != newViewportSize.x || viewportSize.y != newViewportSize.y)
    {
        viewportSize = Vec2{ newViewportSize.x, newViewportSize.y };
		CEO::Get<EventsDispatcher>()->Dispatch<Events::EditorViewportResized>(Events::EditorViewportResized{ viewportSize });
    }
    {
        // resize editor cam if viewport size differs
        CameraManager& camManager = *CEO::Get<CameraManager>();
        if (camManager.IsEditorCam() && (viewportSize.x != camManager.GetViewportSize().x || viewportSize.y != camManager.GetViewportSize().y))
			camManager.SetEditorCameraViewportSize(Vec2{ viewportSize.x,viewportSize.y });
    }

    if (selectedEntityId && registry.GetComponent<TilemapComponent>(selectedEntityId))
    {
        tilesetEditor.DrawGrid(selectedEntityId);

        // prevent mouse from moving camera if drawing on tilemap
        CameraManager& camManager = *CEO::Get<CameraManager>();
        if (camManager.IsEditorCam())
            camManager.suppressEditorCameraMouseMovement = true;
    }
    else
    {
		// restore camera movement with mouse
		CameraManager& camManager = *CEO::Get<CameraManager>();
		if (camManager.IsEditorCam())
			camManager.suppressEditorCameraMouseMovement = false;
	}

    if (ImGui::IsItemHovered()) {                   // if the viewport window is being hovered over
        ImVec2 mousePos{ ImGui::GetMousePos() };    // get the mouse position
        float localX{ mousePos.x - newViewportOrigin.x };    // translate the mouse x pos in respect to the editor window
        float localY{ mousePos.y - newViewportOrigin.y };    // translate the mouse y pos in respect to the editor window
        // check if mouse is within the editor
        if (localX >= 0 && localY >= 0 && localX <= viewportSize.x && localY <= viewportSize.y) {
            localY = viewportSize.y - localY;      // flip the y-axis
            // scale the mouse position based on the image's aspect ratio
            GLint mouseX{ static_cast<int>((localX / viewportSize.x) * ScreenManager::screenBuffer.Width()) };
            GLint mouseY{ static_cast<int>((localY / viewportSize.y) * ScreenManager::screenBuffer.Height()) };

            mouseX = std::clamp(mouseX, 0, ScreenManager::screenBuffer.Width());    // clamp it to between [0, width]
            mouseY = std::clamp(mouseY, 0, ScreenManager::screenBuffer.Height());   // clamp it to between [0, height]
            UIManager::gameMousePos = { static_cast<float>(mouseX), static_cast<float>(mouseY) };
            // if user left clicks on the viewport window and is not hovering over ImGuizmos 
            if (!(ImGuizmo::IsOver() && showGuizmos) && CEO::Instance().GetManager<CameraManager>()->IsEditorCam())
			{
				if (selectedEntityId && registry.GetComponent<TilemapComponent>(selectedEntityId)) // handle tilemap input if a tilemap is selected
                {
                    if(ImGui::IsMouseDown(0))
						tilesetEditor.PlaceSelectedTile(selectedEntityId, mousePos.x, mousePos.y);
                    else if(ImGui::IsMouseDown(1))
                        tilesetEditor.RemoveTile(selectedEntityId, mousePos.x, mousePos.y);
                }
                else if(ImGui::IsMouseClicked(0))// entity picking
                {
					ScreenManager::screenBuffer.Bind();
					glReadBuffer(GL_COLOR_ATTACHMENT1);
					glReadPixels(mouseX, mouseY, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, &selectedEntityId);
					ScreenManager::screenBuffer.Unbind();
					if (selectedEntityId != 0) {

						showGuizmos = true;
					}
                }
            }
        }
    }
    else
    {
        // this is super wack, to be changed in the future? -Yukang
        UIManager::gameMousePos = { -1,-1 }; // set to an invalid number
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) { // check if delete key was pressed
        if (selectedEntityId > 0) {   // check that there is an entity currently selected
            GameObject go(selectedEntityId);
            go.Destroy();
            //hierarchySelection.clear();
            selectedEntityId = 0;
        }
    }
    if (ImGui::IsKeyPressed(ImGuiKey_F) && (IsHierarchyFocused()||IsViewportFocused())) { // check if delete key was pressed
        if (selectedEntityId > 0) {   // check that there is an entity currently selected
            CEO::Get<CameraManager>()->SetEditorCameraPosition(registry.GetComponent<TransformComponent>(selectedEntityId)->translate);
        }
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB")) {
            CEO::Instance().GetManager<ResourceManager>()->InstantiatePrefab(registry, std::string{ (const char*)payload->Data });
        }
        else if (registry.HasComponent<SpriteRendererComponent>(selectedEntityId) ||
            registry.HasComponent<AnimatorComponent>(selectedEntityId) ||
            registry.HasComponent<AudioComponent>(selectedEntityId) ||
            registry.HasComponent<TilemapComponent>(selectedEntityId)) {
            if (const ImGuiPayload* m_payload = ImGui::AcceptDragDropPayload("FILE")) {
                std::string filename = (const char*)m_payload->Data;
                filename = filename.substr(filename.find("Assets\\") + std::string("Assets\\").size(), filename.size());
                filename = filename.substr(filename.find_first_of('\\') + 1);
                std::string ext = filename.substr(filename.find_last_of('.'));
                
                if (ext == ".anim") {
                    if (AnimatorComponent* ac = registry.GetComponent<AnimatorComponent>(selectedEntityId)) {
                        ac->currAnim = &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(filename);
                    }
                    else ImGui::OpenPopup("Error Window");
                }
                else if (ext == ".wav" || ext == ".mp3") {
                    if (AudioComponent* ac = registry.GetComponent<AudioComponent>(selectedEntityId)) {
                        ac->audio = &CEO::Instance().GetManager<ResourceManager>()->GetAudio(filename);
                    }
                    else ImGui::OpenPopup("Error Window");
                }
                else if (ext == ".tileset") {
                    if (TilemapComponent* tilemap = registry.GetComponent<TilemapComponent>(selectedEntityId)) {
                        tilemap->tileset = CEO::Instance().GetManager<ResourceManager>()->GetTileset(filename);
                    }
                    else ImGui::OpenPopup("Error Window");
                }
                else if (std::any_of(imgExt.begin(), imgExt.end(), [&ext](const std::string& s) { return ext == s; })) {
                    if (SpriteRendererComponent* src = registry.GetComponent<SpriteRendererComponent>(selectedEntityId)) {
                        src->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(filename);
                    }
                    else ImGui::OpenPopup("Error Window");
                }
                else {
                    ImGui::OpenPopup("Error Window");
                    registry.GetComponent<SpriteRendererComponent>(selectedEntityId)->texture = &CEO::Instance().GetManager<ResourceManager>()->GetTexture(filename);
                    CollisionSystem::Instance().AutoFitToSpriteOpaque(registry, selectedEntityId);
                }
            }
        }

        ImGui::EndDragDropTarget();
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImVec2 size(400, 150);
    center.x -= size.x / 2; center.y -= size.y / 2;
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(center);
    if (ImGui::BeginPopup("Error Window")) {
        Editor::SetState(EditorState::Pause);
        ImGui::GetIO().MouseClicked[0] = false; // Clears the left click mouse input or smth, probably not the safest
        ImGui::Text("INVALID FILETYPE USED!!!\n");
        ImGui::Text("Supported filetypes for textures are: .gif, .jpg, .bmp, .png\n");
        ImGui::Text("Supported filetypes for animations are: .anim\n");
        ImGui::Text("Supported filetypes for audio are: .wav, .mp3\n");
        ImGui::Text("Supported filetypes for font are: .ttf\n");
        if (ImGui::Button("oops my bad")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (showSceneCreateWindow) {
        if (ImGui::Begin("Create Scene Window")) {
            ImGui::InputText("Scene Name", &bufferedInputStrings["_CreateSceneBuf"]);
            if (ImGui::Button("Create")) {
                SceneManager::CreateScene(bufferedInputStrings["_CreateSceneBuf"]);
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
                showSceneCreateWindow = false;
            } ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                showSceneCreateWindow = false;
            }
        }
        ImGui::End();
    }

    DrawCreateScriptWindow(showScriptCreateWindow);
    
#endif

    ImGui::End();
   

    // Check if a project is open and a scene is loaded.
    if (showHierarchyWindow) NewHiearchyLayout();
    if (showInspectorWindow) InspectorWindow(registry, componentRegistry);
	DrawItemManager(registry);
    // Process commands from the CommandHandler.
    switch (buffer.Command()) {
    case CommandHandler::commands::DELETE:
    case CommandHandler::commands::PASTE:
    default:
        break;
    }
    // Draw runtime toolbar only after a scene is loaded
    RuntimeToolbar(registry, componentRegistry);
    ProfilerWindow();
    AssetBrowserWindow(registry);
    AssetBuildSizeWindow();

    if(showLayersWindow) EditLayersWindow();
    if(showDebugWindow) DebugWindow();
    if (tilesetEditor.showWindow) tilesetEditor.DrawWindow();
    DrawConsole();



    if (selectedEntityId != 0 && CEO::Instance().GetManager<CameraManager>()->IsEditorCam()) {    // check if there is an entity selected
        EditTransformGuizmos(registry, editorDrawList, newViewportSize, newViewportOrigin);    // draw imguizmos if there is
        EditUITransformGuizmos(registry, editorDrawList, newViewportSize, newViewportOrigin);  // draw the ImGuizmos for UI if there is
    }

    // Render all the ImGui draw data.
    ImGui::Render();
    // Pass the ImGui draw data to the OpenGL 3 backend for rendering.
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return;
}
// Sets up the main dockspace for the editor windows.
void Editor::DockspaceWindow() {
    // Begin a full-screen, non-interactive window to act as the dockspace host.
    if (ImGui::Begin("Dockspace", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus))
    {
        // Variable to store the height of the main menu bar.
        float height{};
        if (Input::IsKeyPressed(GLFW_KEY_BACKSPACE) && (IsViewportFocused() || IsHierarchyFocused()))
        {
            if (selectedEntityId)
            {
                GameObject go(selectedEntityId);
                go.Destroy();
            }
        }
        // Begin the main menu bar at the top of the window.
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                // Create a button to save the project.
                if (ImGui::MenuItem("Create Scene")) {
                    showSceneCreateWindow = true;
                    //ImGui::OpenPopup("Create Scene Window");
                }
                if (ImGui::MenuItem("Save Project", "CTRL+S") && !sceneName.empty()) {
                    Save();
                }
                
                if (ImGui::MenuItem("Create Scripts"))
                {
                    showScriptCreateWindow = true;
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                // Add a disabled "Redo" menu item.
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {} // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                // End the "Edit" menu.
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) {
#ifdef PLATFORM_WINDOWS
                if (ImGui::MenuItem(GLApp::isFullscreen ? "Windowed Mode" : "Fullscreen Mode", "F11")) {
                    GLApp::changeWindowMode();
                }
#endif
                // End the "File" menu.
                ImGui::Checkbox("Hierarchy Window", &showHierarchyWindow);
                ImGui::Checkbox("Inspector Window", &showInspectorWindow);
                ImGui::Checkbox("Tileset Window", &tilesetEditor.showWindow);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Debug")) {
                if (ImGui::MenuItem(showDebugWindow ? "Close Debug Window" : "Open Debug Window")) {
                    showDebugWindow = !showDebugWindow;
                }

                ImGui::SeparatorText("Rendering");
                if (ImGui::BeginMenu("Debug Draw")) {
                    ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
                    DebugRender& debugRenderManager{*CEO::Instance().GetManager<DebugRender>() };
                    if (ImGui::MenuItem(debugRenderManager.debugDrawEnabled ? "Disable Debug Draw"
                        : "Enable Debug Draw", "CTRL + I", &debugRenderManager.debugDrawEnabled)) { }
                    if (ImGui::MenuItem(debugRenderManager.collisionDebugEnabled ? "Disable Collision Debug Draw"
                        : "Enable Collision Debug Draw", "", &debugRenderManager.collisionDebugEnabled, debugRenderManager.debugDrawEnabled)) { }

                    if (ImGui::MenuItem(debugRenderManager.velocityDebugEnabled ? "Disable Velocity Debug Draw"
                        : "Enable Velocity Debug Draw", "", &debugRenderManager.velocityDebugEnabled, debugRenderManager.debugDrawEnabled)) { }

                    if (ImGui::MenuItem(debugRenderManager.uiDebugEnabled ? "Disable UI Debug Draw"
                        : "Enable UI Debug Draw", "", &debugRenderManager.uiDebugEnabled, debugRenderManager.debugDrawEnabled)) {
                    }

                    if (ImGui::MenuItem(debugRenderManager.particleDebugEnabled ? "Disable Particle Debug Draw"
                        : "Enable Particle Debug Draw", "", &debugRenderManager.particleDebugEnabled, debugRenderManager.debugDrawEnabled)) {
                    }

                    ImGui::PopItemFlag();
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem(CEO::Instance().GetManager<GraphicsSystem>()->instancingFlag ? "Disable Instancing"
                    : "Enable Instancing", "CTRL + ENTER", &CEO::Instance().GetManager<GraphicsSystem>()->instancingFlag)) {
                }

                if (ImGui::MenuItem(CEO::Instance().GetManager<GraphicsSystem>()->postProcessFlag ? "Disable Postprocess"
                    : "Enable Postprocess", "", &CEO::Instance().GetManager<GraphicsSystem>()->postProcessFlag)) {
                    CEO::Instance().GetManager<UserSettingsManager>()->SetBool("postProcessFlag", CEO::Instance().GetManager<GraphicsSystem>()->postProcessFlag);
                    CEO::Instance().GetManager<UserSettingsManager>()->SaveSettings();
                }

                ImGui::EndMenu();
            }
            // Get the height of the menu bar to offset the dockspace.
            height = ImGui::GetWindowSize()[1];
            // End the main menu bar.
            ImGui::EndMainMenuBar();
        }


        // Set the size of the dockspace window to fill the remaining area.
        ImGui::SetWindowSize(ImVec2{ windowSize[0], windowSize[1] - height });
        // Set the position of the dockspace window just below the menu bar.
        ImGui::SetWindowPos(ImVec2(0.f, height));
        ImGui::DockSpace(ImGuiDockNodeFlags_NoResize | ImGuiDockNodeFlags_NoDockingOverCentralNode | ImGuiDockNodeFlags_NoDockingSplit);
    }
    // End the dockspace window.
    ImGui::End();
}

void Editor::EditorCleanup() {
#ifdef EditorFlag
    for (Registry::Entity ent : CEO::Get<Registry>()->GetDummyEntities()) {

        CEO::Get<Registry>()->DestroyEntity(ent);
    }
#endif
    // Cleanup
    // Shut down the OpenGL 3 backend for ImGui.
    ImGui_ImplOpenGL3_Shutdown();
    // Shut down the GLFW backend for ImGui on Windows.
#ifdef PLATFORM_WINDOWS
    ImGui_ImplGlfw_Shutdown();

    // Destroy the ImGui context.
    ImPlot::DestroyContext();
#endif
    ImGui::DestroyContext();

#ifdef HAS_MFC
    DropViewer.Revoke();
    window.Detach();
#endif

    rapidjson::Document doc;
    rapidjson::Value asmArray;
    doc.SetObject();
    asmArray.SetArray();
    for (const auto& p : assetSelectionMap) {
        rapidjson::Value path;
        rapidjson::Value selected;
        path.SetObject();
        selected.SetObject();
        path.SetString(p.first.string().c_str(), static_cast<rapidjson::SizeType>(p.first.string().length()), doc.GetAllocator());
        selected.SetBool(p.second);
        rapidjson::Value object;
        object.SetObject();
        object.AddMember("Path", path, doc.GetAllocator());
        object.AddMember("Selected", selected, doc.GetAllocator());
        asmArray.PushBack(object.Move(), doc.GetAllocator());
    }
    doc.AddMember("ASM", asmArray, doc.GetAllocator());
    rapidjson::StringBuffer buffer2;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer2);
    doc.Accept(writer);
    std::stringstream buf(buffer2.GetString());
    CEO::Get<FileManager>()->EditorWriteFile("Assets\\assetConfig.json", buf);
    UpdateAssetsFolders(std::filesystem::absolute("Assets\\assetConfig.json").string());

    return;
}

void Editor::AddComponent()
{
    static char searchBuf[256] = "";
    static bool popupOpen = false;
    float sizeX = ImGui::GetContentRegionAvail().x;


    if (ImGui::Button("Add Component", ImVec2(sizeX, 20)))
    {
        ImGui::OpenPopup("SearchablePopup");
        searchBuf[0] = '\0';   // clear search when opening
    }

    if (ImGui::BeginPopup("SearchablePopup"))
    {
        // Search input
        ImGui::SetNextItemWidth(sizeX);
        ImGui::InputText("##SearchBar", searchBuf, IM_ARRAYSIZE(searchBuf));

        ImGui::Separator();

        // Example list of items
        std::vector<std::string> items = CEO::Instance().GetManager<ComponentRegistry>()->GetComponentTypes();
        //Registry& reg = *CEO::Instance().GetManager<Registry>();
        for (auto& item : items)
        {
            // Filter using substring search
            std::string searchstring = searchBuf;
            std::string toloweritem = item;
           
            std::transform(searchstring.begin(), searchstring.end(), searchstring.begin(),
                [](unsigned char c) { return std::tolower(c); } // Use a lambda with explicit cast
            );
            std::transform(toloweritem.begin(), toloweritem.end(), toloweritem.begin(),
                [](unsigned char c) { return std::tolower(c); } // Use a lambda with explicit cast
            );

            if (toloweritem.find(searchstring) == std::string::npos)
                continue;

            if (ImGui::Selectable(item.c_str()))
            {
                CEO::Instance().GetManager<ComponentRegistry>()->CreateComponent(item,selectedEntityId);
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}

// Draws the Inspector window for viewing and editing component properties.
void Editor::InspectorWindow(Registry& registry, ComponentRegistry& componentRegistry)//, 
    //ImDrawList* drawList, const ImVec2& imageSize, const ImVec2& imagePos)
{
    // reset any cached action data if selected entity changed
    if (prevSelectedEntityId != selectedEntityId && prevSelectedEntityId != 0)
        actions.FlushCache();

    if (ImGui::Begin("Inspector")) {
        if (!registry.GetComponent<ActiveComponent>(selectedEntityId)) selectedEntityId = 0;

        if ((displayPrefabHierarchy|| selectedEntityId == selectedPrefab.second) && selectedEntityId !=0) {
            if (registry.GetComponent<HierarchyComponnent>(selectedEntityId)->firstChild != 0) {
                ImGui::TextColored(ImColor(1.0f, 0.5f, 0.0f), "Editing prefab root entity");  ImGui::SameLine();
                if (ImGui::Button("Edit Hierarchy")) {
                    displayPrefabHierarchy = true;
                }
            }
            else {
                ImGui::TextColored(ImColor(1.0f, 0.5f, 0.0f), "Editing prefab");
            }

            if (ImGui::Button("Save prefab")) {
                CEO::Instance().GetManager<ResourceManager>()->OverwritePrefab(registry, selectedPrefab.first, selectedPrefab.second);
                try {
                    // Iterate over directory entries
                    for (const auto& entry : std::filesystem::directory_iterator("Assets\\Prefabs\\")) {
                        // Check if the entry is a regular file
                        if (std::filesystem::is_regular_file(entry.path())) {
                            UpdateAssetsFolders(entry.path().relative_path().string(), "Prefabs");
                        }
                    }
                }
                catch (const std::filesystem::filesystem_error& e) {
                    LOGE("Error accessing directory: %s", e.what());
                }
            };
        }

        if (selectedEntityId == 0 /*|| !loadedScene->FindEntity(selectedEntityId).CurrNode()*/) ImGui::Text("Nothing Selected");
        else {
            PrefabComponent* prefabComp = registry.GetComponent<PrefabComponent>(selectedEntityId);
            if (prefabComp && !prefabComp->name.empty()) {
                ImGui::Text("Linked to prefab: %s", prefabComp->name.c_str());
                if(ImGui::Button("Unlink prefab")) {
                    auto deletePrefabComp = [&registry](Registry::Entity ent) {
                        if (PrefabComponent* comp = registry.GetComponent<PrefabComponent>(ent)) {
                            registry.RemoveComponent<PrefabComponent>(ent);
                        }
                        };
                    HierarchyManager::Traverse(registry, selectedEntityId, deletePrefabComp);
				}
            }
            else if (prefabComp && prefabComp->name.empty()) {
                ImGui::BeginDisabled();
            }
            ImGui::Checkbox("##ActiveComponent", &(registry.GetComponent<ActiveComponent>(selectedEntityId)->isActiveSelf));
            if (actions.TryCacheOrPush(registry.GetComponent<ActiveComponent>(selectedEntityId)->isActiveSelf) & EditorActions::CachePushResult_Pushed) {
                if (prefabComp) {
                    PrefabManager::OverrideMember(*prefabComp, "ActiveComponent", "isActiveSelf");
                }
            }

            ImGui::SameLine();
            ImGuiInputTextFlags flags =
                ImGuiInputTextFlags_EnterReturnsTrue |
                ImGuiInputTextFlags_AutoSelectAll;

            char name[256];
            strcpy(name,registry.GetComponent<NameComponent>(selectedEntityId)->name.c_str());

            if (ImGui::InputText("Name", name, IM_ARRAYSIZE(name), flags))
            {
                if (name[0] != '\0')   // checks first character is not null
                {
                    registry.GetComponent<NameComponent>(selectedEntityId)->name = name;
                    if (prefabComp) {
                        PrefabManager::OverrideMember(*prefabComp, "NameComponent", "name");
                    }
                }
            }
            /*---------------------------------------------------
            * CUSTOM INSEPCTOR WIDGETS
            ----------------------------------------------------*/
            if (registry.HasComponent<TransformComponent>(selectedEntityId)) {
				ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
				if (ImGui::CollapsingHeader("Transform Component", flag))
				{
					EditTransformWindow(registry);
					ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
				}
            }
            else if (registry.HasComponent<UITransformComponent>(selectedEntityId))
            {
                ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
                if (ImGui::CollapsingHeader("UITransform Component", flag))
                {
                    EditUITransformWindow(registry);
                    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
                }
            }

            for (const std::string& component : componentRegistry.GetComponentTypes()) {
                // Check if the selected entity has this component.
                rtr::Instance inst = registry.GetComponentsRTR(selectedEntityId, rtr::TypeInfo::GetByName(component));

                if (!inst.isValid() || !inst.GetType().IsEditorInspectable())
                   continue;
                std::string headerName{ component.substr(0, component.find("Component")) + " Component" };
    
                ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_None;
                if (prefabComp && prefabComp->name.empty()) {
                    flag = ImGuiTreeNodeFlags_DefaultOpen;
                }
                if (ImGui::CollapsingHeader(headerName.c_str(), flag)) {
                    /*---------------------------------------------------
                    * CUSTOM INSEPCTOR WIDGETS FOR MEMBERS
                    ----------------------------------------------------*/
                    if (component == "CollisionComponent") {
                        if (ImGui::Button("Auto-fit Collider")) {
                            CollisionSystem::Instance().AutoFitToSpriteOpaque(registry, selectedEntityId);
                        }
                        ImGui::Separator();
                    }
                    if (component == "VideoComponent") {
                        // Editor UX helpers and validation hints for video/audio authoring fields.
                        VideoComponent* vc = registry.GetComponent<VideoComponent>(selectedEntityId);
                        if (vc) {
                            // Display path format guidance for new users
                            ImGui::TextWrapped("Use project-relative paths for videoPath (e.g. Assets/Videos/intro.mpeg).");
                            ImGui::TextWrapped("For nextSceneOnEnd/nextSceneOnSkip, use scene names without .scene extension.");
                            
                            // Validation check: warn if videoPath is empty (video disabled)
                            if (vc->videoPath.empty()) {
                                ImGui::TextColored(ImVec4(1.f, 0.7f, 0.2f, 1.f), "videoPath is empty (playback disabled)");
                            }
                            
                            // Validation check: warn if videoPath appears to be absolute Windows path
                            if (vc->videoPath.find(":\\") != std::string::npos) {
                                ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "videoPath appears to be an absolute Windows path (use relative path)");
                            }
                            
                            // Validation check: warn if playback speed is negative (pl_mpeg will clamp to 0)
                            if (vc->playbackSpeed < 0.f) {
                                ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "playbackSpeed < 0 is invalid (will clamp to 0)");
                            }
                            
                            // Validation check: warn if audio volume is out of valid range
                            if (vc->audioVolume < 0.f || vc->audioVolume > 1.f) {
                                ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "audioVolume should be within [0, 1]");
                            }
                            
                            // Validation check: warn if audioType is empty (defaults to BGM)
                            if (vc->audioType.empty()) {
                                ImGui::TextColored(ImVec4(1.f, 0.4f, 0.4f, 1.f), "audioType is empty (defaults to BGM)");
                            }

                            // Runtime transport controls for quick preview/testing in editor play mode
                            if (vc->video) {
                                // Play button: starts video from current position with loop setting from component
                                if (ImGui::Button("Play Video")) {
                                    VideoManager::PlayVideo(vc->video, vc->loop);
                                }
                                ImGui::SameLine();
                                
                                // Pause button: stops playback without resetting position
                                if (ImGui::Button("Pause Video")) {
                                    VideoManager::PauseVideo(vc->video);
                                }
                                ImGui::SameLine();
                                
                                // Stop button: stops playback and resets to beginning
                                if (ImGui::Button("Stop Video")) {
                                    VideoManager::StopVideo(vc->video);
                                }
                                
                                // Restart button: rewinds to beginning without playing
                                if (ImGui::Button("Restart Video")) {
                                    VideoManager::RestartVideo(vc->video);
                                }
                                ImGui::SameLine();
                                
                                // Seek 0s button: seeks to exact frame 0 (useful for testing intro frames)
                                if (ImGui::Button("Seek 0s")) {
                                    VideoManager::SeekVideo(vc->video, 0.0, true);
                                }
                            }
                            ImGui::Separator();
                        }
                    }
                    
                    /*---------------------------------------------------
					* AUTO GENERATED INSEPCTOR WIDGETS FOR MEMBERS
                    ----------------------------------------------------*/
                    prefabComp = CEO::Get<Registry>()->GetComponent<PrefabComponent>(selectedEntityId);

                    for (std::string memberName : inst.GetType().Members()) {
						InspectorMemberName(prefabComp, component, memberName);

                        // Get the type of the current member.
                        std::type_index memberType{ inst.GetMemberType(memberName) };
                        // Check if a value for this member exists in the JSON object.
                        if (std::type_index(typeid(std::string)) == memberType) {
                            ImGui::InputTextMultiline((memberName + "##" + component+memberName).c_str(), inst.GetPtr<std::string>(memberName));
                            if (actions.TryCacheOrPush(*inst.GetPtr<std::string>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                if (prefabComp) {
									PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(int)) == memberType) {
                            ImGui::InputScalar((memberName + "##" + component+memberName).c_str(), ImGuiDataType_S32, inst.GetPtr<int>(memberName));
                            if (actions.TryCacheOrPush(*inst.GetPtr<int>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(unsigned int)) == memberType) {
                            if (memberName == "layer" ) 
                                EditLayerComponentWindow(registry);
                            else {
                                ImGui::InputScalar((memberName + "##" + component+memberName).c_str(), ImGuiDataType_U32, inst.GetPtr<unsigned int>(memberName));
                                if (memberName == "renderPriority") {
                                    // clamp renderLayer value between [1, maxDepth]
                                    *inst.GetPtr<unsigned int>(memberName) = 
                                        std::clamp(inst.GetVal<unsigned int>(memberName), 1u, CEO::Instance().Get<RenderUtils>()->GetMaxRenderDepth());
                                }
                                if (actions.TryCacheOrPush(*inst.GetPtr<unsigned int>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                    if (prefabComp) {
                                        PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                    }
                                }
                            }
                        }
                        else if (std::type_index(typeid(float)) == memberType) {
                            ImGui::InputScalar((memberName + "##" + component+memberName).c_str(), ImGuiDataType_Float, inst.GetPtr<float>(memberName));
                            if (actions.TryCacheOrPush(*inst.GetPtr<float>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(double)) == memberType) {
                            ImGui::InputScalar((memberName + "##" + component+memberName).c_str(), ImGuiDataType_Double, inst.GetPtr<double>(memberName));
                            if (actions.TryCacheOrPush(*inst.GetPtr<double>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(bool)) == memberType) {
                            ImGui::Checkbox((memberName + "##" + component+memberName).c_str(), inst.GetPtr<bool>(memberName));
                            if (actions.TryCacheOrPush(*inst.GetPtr<bool>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(Shape)) == memberType) {
                            int val = static_cast<int>(inst.GetVal<Shape>(memberName));
                            if (ImGui::Combo((memberName + "##" + component).c_str(), &val, "Box\0Circle\0Capsule\0\0"))
                            {
                                actions.Cache(*inst.GetPtr<Shape>(memberName));
                                inst.SetVal(memberName, static_cast<Shape>(val));
                                actions.Push(*inst.GetPtr<Shape>(memberName));
                            }
                        }
                        else if (std::type_index(typeid(TextRendererComponent::Alignment)) == memberType) {
                            int val = static_cast<int>(inst.GetVal<TextRendererComponent::Alignment>(memberName));
                            if (ImGui::Combo((memberName + "##" + component+memberName).c_str(), &val, "Left\0Center\0Right\0\0"))
                            {
                                actions.Cache(*inst.GetPtr<TextRendererComponent::Alignment>(memberName));
                                inst.SetVal(memberName, static_cast<TextRendererComponent::Alignment>(val));
                                actions.Push(*inst.GetPtr<TextRendererComponent::Alignment>(memberName));
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(LightComponent::LightType)) == memberType) {
                            int val = static_cast<int>(inst.GetVal<LightComponent::LightType>(memberName));
                            if (ImGui::Combo((memberName + "##" + component + memberName).c_str(), &val, "POINT\0GLOBAL\0\0"))
                            {
                                actions.Cache(*inst.GetPtr<LightComponent::LightType>(memberName));
                                inst.SetVal(memberName, static_cast<LightComponent::LightType>(val));
                                actions.Push(*inst.GetPtr<LightComponent::LightType>(memberName));
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(Mat3)) == memberType) {
                            Mat3* matTemp = inst.GetPtr<Mat3>(memberName);

                            for (int i{}; i < 9; i++) {
                                ImGui::PushItemWidth(ImGui::GetWindowWidth() / 4);
                                ImGui::DragFloat((std::to_string(i / 3) + std::to_string(i % 3) + "##" + component + memberName).c_str(), &matTemp->m[i], 0.005f);
                                if (actions.TryCacheOrPush(*inst.GetPtr<Mat3>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                    if (prefabComp) {
                                        PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                    }
                                }
                                ImGui::PopItemWidth();
                                if (i % 3 != 2) ImGui::SameLine();
                            }
                        }
                        // If the member is a Vec2, deserialize it from a float array.
                        else if (std::type_index(typeid(Vec2)) == memberType) {
                            // Create a temporary Vec2 object.
                            Vec2* vecTemp = inst.GetPtr<Vec2>(memberName);
                            ImGui::PushItemWidth(ImGui::GetWindowWidth() / 3);
							ImGui::DragFloat(("x##" + component + memberName).c_str(), &vecTemp->x, 1.f); ImGui::SameLine();
                            if (actions.TryCacheOrPush(*inst.GetPtr<Vec2>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            };

                            ImGui::DragFloat(("y##" + component + memberName).c_str(), &vecTemp->y, 1.f);
                            if (actions.TryCacheOrPush(*inst.GetPtr<Vec2>(memberName)) & EditorActions::CachePushResult_Pushed) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            };
                            ImGui::PopItemWidth();
                        }
                        else if (std::type_index(typeid(Animation*)) == memberType) {
                            if (inst.GetVal<Animation*>(memberName)) {
                                ImGui::Text("%s", ("Animation name: " + inst.GetVal<Animation*>(memberName)->animName).c_str());
                                std::unordered_map<std::string, std::string>::iterator search = bufferedInputStrings.find(component+ memberName);
                                if (search == bufferedInputStrings.end()) {
                                    bufferedInputStrings.emplace(component + memberName, "");
                                    search = bufferedInputStrings.find(component + memberName);
                                }
                                ImGui::InputText(("New animation Name" + std::string("##") + component+memberName).c_str(), &search->second);
                                if (ImGui::BeginDragDropTarget())
                                {

                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
                                    {
                                        std::filesystem::path filename = std::string{ (const char*)payload->Data };
                                        if (filename.extension() == ".anim") {
                                            search->second = filename.filename().string();
                                        }
                                        else {
                                            ImGui::OpenPopup("Error Window");
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                                if (ImGui::Button(("Apply new Animation##" + component+memberName).c_str())) {
                                    inst.SetVal(memberName, &CEO::Instance().GetManager<ResourceManager>()->GetAnimation(search->second));
                                    if (prefabComp) {
                                        PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                    }
                                }
                                ImGui::Text("%s", ("Texture: " + inst.GetVal<Animation*>(memberName)->spriteSheetName).c_str());
                                ImGui::Text("%s", ("cols: " + std::to_string(inst.GetVal<Animation*>(memberName)->cols)).c_str());
                                ImGui::Text("%s", ("rows: " + std::to_string(inst.GetVal<Animation*>(memberName)->rows)).c_str());

                                    }
                                }
                        else if (std::type_index(typeid(TextureObj*)) == memberType) {
                            if (inst.GetVal<TextureObj*>(memberName)) {
                                ImGui::Text("%s", ("Filepath: " + CEO::Instance().GetManager<ResourceManager>()->GetTexPath(*inst.GetVal<TextureObj*>(memberName))).c_str());
                                auto search = bufferedInputStrings.find(component + memberName);
                                if (search == bufferedInputStrings.end()) {
									bufferedInputStrings.emplace(component + memberName, "");
                                    search = bufferedInputStrings.find(component + memberName);
                                }
                                ImGui::InputText(("New texture file" + std::string("##") + component+memberName).c_str(), &search->second);
                                if (ImGui::BeginDragDropTarget())
                                {

                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
                                    {
										std::filesystem::path filename = std::string{ (const char*)payload->Data };
                                        if (filename.extension() == ".gif" || filename.extension() == ".jpg" || filename.extension() == ".bmp" || filename.extension() == ".png") {
                                            search->second = filename.filename().string();

                                        }
                                        else {
											ImGui::OpenPopup("Error Window");
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }
                                if (ImGui::Button(("Apply new Texture##" + component+memberName).c_str())) {
                                    inst.SetVal(memberName, &CEO::Instance().GetManager<ResourceManager>()->GetTexture(search->second));
                                    CollisionSystem::Instance().AutoFitToSpriteOpaque(registry, selectedEntityId);
                                    if (prefabComp) {
                                        PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                    }
                                }
                            }
                        }
                        else if (std::type_index(typeid(AudioObj*)) == memberType) {
							ImGui::Text("%s", ("Audio file: " + inst.GetVal<AudioObj*>(memberName)->GetPath()).c_str());
                            auto search = bufferedInputStrings.find(component + memberName);
                            if (search == bufferedInputStrings.end()) {
                                bufferedInputStrings.emplace(component + memberName, "");
                                search = bufferedInputStrings.find(component + memberName);
                            }
                            ImGui::InputText(("New audio file" + std::string("##") + component+memberName).c_str(), &search->second);
                            if (ImGui::BeginDragDropTarget())
                            {

                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
                                {
                                    std::filesystem::path filename = std::string{ (const char*)payload->Data };
                                    if (filename.extension() == ".wav" || filename.extension() == ".mp3") {
                                        search->second = filename.filename().string();
                                    }
                                    else {
                                        ImGui::OpenPopup("Error Window");
                                    }
                                }
                                ImGui::EndDragDropTarget();
                            }
                            if (ImGui::Button(("Apply new Audio file##" + component+memberName).c_str())) {
                                inst.SetVal(memberName, &CEO::Instance().GetManager<ResourceManager>()->GetAudio(search->second));
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(FontObj*)) == memberType) {
                            if (inst.GetVal<FontObj*>(memberName)) {
                                ImGui::Text("%s", ("Filepath: " + inst.GetVal<FontObj*>(memberName)->path).c_str());
                                auto search = bufferedInputStrings.find(component + memberName);
                                if (search == bufferedInputStrings.end()) {
                                    bufferedInputStrings.emplace(component + memberName, "");
                                    search = bufferedInputStrings.find(component + memberName);
                                }
                                ImGui::InputText(("New font file" + std::string("##") + component+memberName).c_str(), &search->second);

                                if (ImGui::BeginDragDropTarget())
                                {

                                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE"))
                                    {
                                        std::filesystem::path filename = std::string{ (const char*)payload->Data };
                                        if (filename.extension() == ".ttf") {
                                            search->second = filename.filename().string();
                                        }
                                        else {
                                            ImGui::OpenPopup("Error Window");
                                        }
                                    }
                                    ImGui::EndDragDropTarget();
                                }

                                if (ImGui::Button(("Apply new Font##" + component+memberName).c_str())) {
                                    inst.SetVal(memberName, &CEO::Instance().GetManager<ResourceManager>()->GetFont(search->second));
                                    if (prefabComp) {
                                        PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                    }
                                }
                            }
                            }
                        else if (std::type_index(typeid(uint64_t)) == memberType) {
							//TODO: Track changes better override prefab
							uint64_t val = inst.GetVal<uint64_t>(memberName);
                            EditCameraCullingMaskWindow(registry);
                            if (val != inst.GetVal<uint64_t>(memberName)) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(Color)) == memberType) {
                            //TODO: Track changes better override prefab
                            Color val = inst.GetVal<Color>(memberName);
                            ImGui::ColorEdit4(("RGBA##" + component + memberName).c_str(), reinterpret_cast<float*>(inst.GetPtr<Color>(memberName)));
                            if (val.r != inst.GetVal<Color>(memberName).r ||
                                val.g != inst.GetVal<Color>(memberName).g || 
                                val.b != inst.GetVal<Color>(memberName).b || 
                                val.a != inst.GetVal<Color>(memberName).a) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }
                        }
                        else if (std::type_index(typeid(GameObject)) == memberType) {
                            GameObject* Go = inst.GetPtr<GameObject>(memberName);
                            //TODO: Track changes better override prefab
                            GameObject val{ *Go };

                            std::string entityName;
                            if (Go->IsValid())
                            {
                                if (Go->GetEntityID())
                                    if(CEO::Get<Registry>()->GetComponent<NameComponent>(Go->GetEntityID()))
                                        entityName = CEO::Get<Registry>()->GetComponent<NameComponent>(Go->GetEntityID())->name;
                                else
                                    entityName = Go->GetPrefabName();
                            }
                            ImGui::BeginDisabled(true);
                            std::string GameID = Go->GetEntityID() ? "Game Object" : "Game Prefab";
                            GameID += "##" + component +memberName;
                            ImGui::InputText(GameID.c_str(), &entityName);

                            ImGui::EndDisabled();
                            if (ImGui::BeginDragDropTarget())
                            {

                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
                                {
                                    EntityRegistry::Entity incoming = *(EntityRegistry::Entity*)payload->Data;
                                    *Go = incoming;
                                }
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("PREFAB"))
                                {
                                    std::string incoming = std::string{ (const char*)payload->Data };
                                    *Go = incoming;
                                }
                                ImGui::EndDragDropTarget();
                            }
                            if (
								val.GetEntityID() ||
								(!val.GetPrefabName().empty() && Go->GetPrefabName() != val.GetPrefabName())
                                ) {
                                if (prefabComp) {
                                    PrefabManager::OverrideMember(*prefabComp, component, memberName);
                                }
                            }      
                        }
                        // else if (std::type_index(typeid(ColliderShape)) == memberType) {
                        //     bool isBox{ inst.GetVal<ColliderShape>(memberName) == ColliderShape::Box };
                        //     if (ImGui::RadioButton("Box", isBox)) {
                        //         inst.SetVal(memberName, Shape::Box);
                        //         isBox = true;
                        //     }
                        //     ImGui::SameLine();
                        //     if (ImGui::RadioButton("Circle", !isBox)) {
                        //         inst.SetVal(memberName, Shape::Circle);
                        //         isBox = false;
                        //     }
                        // }

                        // If the member type is unknown, throw an exception.
                        else {
                            ImGui::Text("%s", ("Component: '" + component + "', member: '" + memberName + "' has no inpector widget").c_str());
                        }

              
                    }
                    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
                    if (ImGui::Button(("Remove Component ##" + component).c_str()))
                    {
                        componentRegistry.RemoveComponent(component,selectedEntityId);
                    }
                    ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

                }

            }
            AddComponent();
            if (prefabComp && prefabComp->name.empty()) {
                ImGui::EndDisabled();
            }
        }
    }

    ImVec2 window_content_region = ImGui::GetContentRegionAvail();
    ImVec2 pos = ImGui::GetCursorPos();
    ImGui::Dummy(window_content_region);
    if (selectedEntityId)
    {
        if (ImGui::BeginDragDropTarget() && selectedEntityId) {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCRIPT")) {
                CEO::Instance().GetManager<ComponentRegistry>()->CreateComponent(std::string{ (const char*)payload->Data }, selectedEntityId);
            }
            ImGui::EndDragDropTarget();
        }
    }
    // End the "Inspector" window.
    ImGui::End();
}

#define UNUSED_PARAM(componentRegistry) (void)(componentRegistry)
void Editor::RuntimeToolbar(Registry& , ComponentRegistry&){

    ImGui::Begin("Runtime Controls", nullptr, ImGuiWindowFlags_NoCollapse );

    //Savestate& ss = *CEO::Instance().GetManager<Savestate>();
    // Display current mode
    ImGui::TextColored(
        (state == EditorState::Play) ? ImVec4(0, 1, 0, 1):
            (state == EditorState::Pause) ? ImVec4(1, 1, 0, 1):
                                            ImVec4(1, 1, 1, 1),
        (state == EditorState::Play) ? "PLAY MODE":
            (state == EditorState::Pause) ? "PAUSED": "EDIT MODE");
    
    ImGui::Separator();

    switch (state){
    
        // EDIT MODE - Show Play Button
        case EditorState::Edit:

            if (ImGui::Button("Play")){
                if (sceneName.empty()) {
					sceneName = "Landing Page";
                    CEO::Instance().GetManager<SceneManager>()->QueueSceneAction(sceneName, SceneManager::CHANGE);
                }
                state = EditorState::Play;

                //SceneManager::SerializeSceneRJson(registry, CEO::Instance().GetManager<SceneManager>()->BaseScene());
                CEO::Instance().GetManager<CameraManager>()->IsEditorCam(false);
                CEO::Instance().GetManager<EventsDispatcher>()->Dispatch<Events::EditorEnterPlay>(Events::EditorEnterPlay{});
            }
            break;

        // PLAY MODE - Show Pause and Stop buttons
        case EditorState::Play:
            PhysicsSystem::Instance().SetPause() = false;
            if (ImGui::Button("Pause")){
                CEO::Instance().GetManager<ResourceManager>()->PauseAllAudio(true);
                state = EditorState::Pause;
            }
            ImGui::SameLine();

            if (ImGui::Button("Stop")){
                CEO::Instance().GetManager<ResourceManager>()->StopAllAudio();
                CEO::Instance().GetManager<ResourceManager>()->ClearBGMQueue();

                state = EditorState::Edit;
                selectedEntityId = 0;

                SceneManager::QueueSceneAction(sceneName, SceneManager::CHANGE);

                CEO::Instance().GetManager<CameraManager>()->IsEditorCam(true);
                CEO::Instance().GetManager<UpdateStackManager>()->SetStack(0);

                std::cout << "[Editor] Simulation stopped, scene restored to initial snapshot.\n";
            }
            break;

        // PAUSE MODE - Show Resume, Stop, Step buttons
        case EditorState::Pause:

            if (ImGui::Button("Resume")){
                CEO::Instance().GetManager<ResourceManager>()->PauseAllAudio(false);
                state = EditorState::Play;
            }
            ImGui::SameLine();

            if (ImGui::Button("Stop")){
                CEO::Instance().GetManager<ResourceManager>()->StopAllAudio();
                CEO::Instance().GetManager<ResourceManager>()->ClearBGMQueue();

                state = EditorState::Edit;
                selectedEntityId = 0;

                SceneManager::QueueSceneAction(sceneName, SceneManager::CHANGE);
                CEO::Instance().GetManager<CameraManager>()->IsEditorCam(true);
                
                std::cout << "[Editor] Simulation stopped, scene restored to initial snapshot.\n";
            }
            ImGui::SameLine();

            if (ImGui::Button("Step Forward")){
                stepForward = true;
                //ss.StepForward();
            }
            ImGui::SameLine();

            if (ImGui::Button("Step Backward")){
                stepBackward = true;
                //ss.StepBackward();
            }
            break;
    }

#ifdef EditorFlag
    // show editor/game camera state
    ImGui::Text(CEO::Instance().GetManager<CameraManager>()->IsEditorCam() ? "Camera: Editor" : "Camera: Game");
    if (ImGui::Button("Toggle Camera Mode"))
    {
        CEO::Instance().GetManager<CameraManager>()->ToggleEditorCam();
    }
#endif

    ImGui::Separator();
    ImGui::End();
}


bool Editor::IsDescendantOf(EntityRegistry::Entity possibleChild, EntityRegistry::Entity possibleParent)
{
    Registry* r = CEO::Instance().GetManager<Registry>();
    EntityRegistry::Entity cur = r->GetComponent<HierarchyComponnent>(possibleParent)->firstChild;
    EntityRegistry::Entity curChild = r->GetComponent<HierarchyComponnent>(possibleParent)->parent;


    while (cur != 0 || curChild != 0)
    {
        if (cur == possibleChild)
            return true;
        if (possibleChild == curChild)
            return true;

        if(cur != 0)
            cur = r->GetComponent<HierarchyComponnent>(cur)->nextSibling;
        if (curChild != 0)
            curChild = r->GetComponent<HierarchyComponnent>(curChild)->parent;

    }

    return false;
}

bool Editor::DrawNode(EntityRegistry::Entity e,int depth)
{
    Registry* registry = CEO::Instance().GetManager<Registry>();
    HierarchyComponnent* h = registry->GetComponent<HierarchyComponnent>(e);

    ImGui::Indent(depth * 15.0f);     // indent based on depth (15px each level)

    bool hasChildren = (h->firstChild != 0);

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow;

    // Optional: highlight if selected
    if (selectedEntityId == e)
        flags |= ImGuiTreeNodeFlags_Selected;

    bool expanded = false;

    auto active = registry->GetComponent<ActiveComponent>(e)->isActiveSelf && registry->GetComponent<ActiveComponent>(e)->isActiveInHierarchy;

    if (!active)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5, 0.5, 0.5, 1));

    if (hasChildren)
        expanded = ImGui::TreeNodeEx((registry->GetComponent<NameComponent>(e)->name + "##" + std::to_string(e)).c_str()  , flags);
    else
    {
        flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        ImGui::TreeNodeEx((registry->GetComponent<NameComponent>(e)->name + "##" + std::to_string(e)).c_str(), flags);
    }
    if (!hasChildren)
        expanded = false;

    bool clicked =
        ImGui::IsItemHovered() &&
        ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
        !ImGui::IsMouseDragging(ImGuiMouseButton_Left);

    if (clicked)
    {
        selectedEntityId = e;
        if (registry->HasComponent<TilemapComponent>(selectedEntityId))
        {
            tilesetEditor.SetSelectedTileset(registry->GetComponent<TilemapComponent>(selectedEntityId)->tileset);
        }
    }

    
    ImGui::Unindent(depth * 15.0f);

    if(expanded)
        ImGui::TreePop();

    if (!active)
        ImGui::PopStyleColor();

    if (!(registry->HasComponent<PrefabComponent>(e) && registry->GetComponent<PrefabComponent>(e)->name.empty())) {
        if (ImGui::BeginDragDropSource())
        {
            dragEntity = e;
            ImGui::SetDragDropPayload("HIERARCHY_ENTITY", &e, sizeof(EntityRegistry::Entity));
            ImGui::Text("Move %s", registry->GetComponent<NameComponent>(e)->name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
            {
                EntityRegistry::Entity incoming = *(EntityRegistry::Entity*)payload->Data;

                // Prevent dropping entity onto itself or its descendants
                if (!IsDescendantOf(incoming, e))
                    dropEntity = e;  // e is the new parent
            }
            ImGui::EndDragDropTarget();
        }
    }
    if (registry->HasComponent<PrefabComponent>(e)) {
        ImGui::SameLine();
        if (registry->GetComponent<PrefabComponent>(e)->name.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.f));
            ImGui::Text("(prefab child)");
        }
        else {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.f));
            ImGui::Text("(prefab)");
        }
        ImGui::PopStyleColor();

    }
    return expanded;
}

void Editor::DrawHierarchyIterative(EntityRegistry::Entity root)
{
    Registry* registry = CEO::Instance().GetManager<Registry>();

    EntityRegistry::Entity cur = root;
    int depth = 1;

    while (cur != 0)
    {
        auto* h = registry->GetComponent<HierarchyComponnent>(cur);

        bool expanded = DrawNode(cur, depth);

        if (expanded && h->firstChild != 0)
        {
            // go DOWN the hierarchy
            depth++;
            cur = h->firstChild;
            

        }
        else
        {
            if (h->nextSibling != 0)
            {
                cur = h->nextSibling;
  
            }
            else
            {
                // go UP until we find a sibling
                while (cur != 0)
                {
                    auto* ph = registry->GetComponent<HierarchyComponnent>(cur);
                    if (ph->parent != 0)
                    {
                        // climb up
                        cur = ph->parent;
                        depth--;

                        auto* parentH = registry->GetComponent<HierarchyComponnent>(cur);
                        if (parentH->nextSibling != 0)
                        {
                            cur = parentH->nextSibling; // go to parent's sibling
                            break;
                        }
                    }
                    else
                    {
                        cur = 0;
                        break;
                    }
                }
            }
        }
    }
}


void Editor::NewHiearchyLayout()
{
    ImGui::Begin("Hierarchy");
    isHierarchyFocused = ImGui::IsWindowFocused();
    ImGui::Text(("Opened Scene: " + sceneName).c_str()); ImGui::SameLine();
    if(ImGui::Button("Reload Scene")){
		CEO::Get<SceneManager>()->QueueSceneAction(sceneName, SceneManager::CHANGE);
	}
    ImGui::Separator();
    if (ImGui::BeginPopupContextWindow())
    {
        if (ImGui::MenuItem("Create Empty Entity"))
        {
            GameObject newEnt = CreateGameobject();
            if (selectedPrefab.second) {
                newEnt.AddComponent<PrefabDummyMetatag>();
                GameObject{ selectedPrefab.second }.SetChild(newEnt.GetEntityID());
            }
        }
        if (ImGui::MenuItem("Create Empty UI Entity"))
        {
            GameObject newEnt = CreateUIGameobject();
            if (selectedPrefab.second) {
                newEnt.AddComponent<PrefabDummyMetatag>();
                GameObject{ selectedPrefab.second }.SetChild(newEnt.GetEntityID());
            }
        }

        ImGui::EndPopup();
    }
    Registry* registry = CEO::Instance().GetManager<Registry>();

    std::vector<Registry::Entity> entitiesToDraw{};
    
    if (displayPrefabHierarchy) {
        if (ImGui::Button("<")) {
			displayPrefabHierarchy = false;
            selectedEntityId = selectedPrefab.second;
        }
        entitiesToDraw = registry->GetDummyEntities();
    }
    else {
        entitiesToDraw = registry->GetEntitiesWithComponents<HierarchyComponnent>();
    }

    // Find all root-level entities
    for (auto ent : entitiesToDraw)
    {
        auto* h = registry->GetComponent<HierarchyComponnent>(ent);
        if (h->parent == 0)
        {
            DrawHierarchyIterative(ent);
            ImGui::InvisibleButton((std::string("##RootDropZone") + std::to_string(ent)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 2));
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
                {
                    EntityRegistry::Entity dragged = *(EntityRegistry::Entity*)payload->Data;

                    CEO::Instance().GetManager<HierarchyManager>()->ReparentToRoot(dragged);
                }
                ImGui::EndDragDropTarget();
            }
        }
            
    }

    if (dragEntity != 0 && dropEntity != 0)
    {
        CEO::Instance().GetManager<HierarchyManager>()->Reparent(dragEntity, dropEntity);
        dragEntity = 0;
        dropEntity = 0;

    }


 

    
    ImGui::End();
}

void Editor::InspectorMemberName(PrefabComponent* prefabComp, std::string const& component, std::string const& memberName) {
    if (prefabComp &&
        prefabComp->overriddenComponents.find(component) != prefabComp->overriddenComponents.end() &&
        prefabComp->overriddenComponents[component] & rtr::TypeInfo::GetByName(component).GetMemberFlag(memberName))
    {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(66.f / 255.f, 150.f / 255.f, 250.f / 255.f, 1.0f)); // Blue separator lines
        ImGui::SeparatorText(memberName.c_str());
        ImGui::PopStyleColor();
        OverridePopup(prefabComp, component, memberName);
    }
    else {
        ImGui::SeparatorText(memberName.c_str());
    }
}

#endif
#endif
