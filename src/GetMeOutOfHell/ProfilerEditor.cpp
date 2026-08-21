/**___________________________________________________________________________/
@file          ProfilerEditor.cpp
@author        j.junbo@digipen.edu
@date          9/29/2025

The file name is outdated.
This file is an extension of the editor.cpp file and it implements 3 additional functions
It draws the imgui widget to display the profiler info as a graph
Copyright (C) 2026 DigiPen Institute of Technology. All rights reserved.

/*____________________________________________________________________________*/
#include "Platform.h"
#ifdef PLATFORM_WINDOWS
#ifdef EditorFlag

#include "editor.h"
#include "Profiler.h"
#include "implot.h"
#include "implot_internal.h"
#include <deque>
#include <filesystem>
#include <set>
#include <numeric>
#include "ResourceManager.h"
#include "PrefabManager.h"
#include "CEO.h"
#include "Components.h"
#include "SceneManager.h"
#include "ScriptingAPI.h"
#include "FileManager.h"

namespace {
    // Functors. I made them before learning about lambda in hlp3.
    // Checks if a given string is equal to what it is initialized with
    class Equal {
    private:
        const std::string& s;
    public:
        Equal(const std::string& s) : s{ s } {}
        bool operator()(const std::string& s1) const { return s1 == s; }
    };

    // Custom comparator functor to pass into as the third template type in the map.
    // Orders the string by checking if it ends with value or graph. If graph put above values.
    // If both has the equal ending it uses the default > operator on the strings.
    class StringComp {
    public:
        bool operator()(const std::string& s1, const std::string& s2) const {
            std::string sub1 = s1.substr(s1.size() - 5, 5);
            std::string sub2 = s2.substr(s2.size() - 5, 5);

            if (sub1 == sub2) {
                return s1 > s2;
            }
            else {
                if (sub1 == "value") {
                    return false;
                }
                else {
                    return true;
                }
            }
        }
    };

    // Display names for the profiler widget
    const std::string value(" value");
    const std::string graph(" graph");

}

void Editor::ProfilerWindow() {
#ifdef PLATFORM_WINDOWS
    size_t size = 300;                                              // based on how many frames to store
    static std::vector<std::string> profileNames;                   // the names of what is being profiled
    static std::map<std::string, bool, StringComp> profileBool;     // a map to store if the imgui checkbox is ticked
    static std::vector<double> segment;                             // the x axis of the graph
    auto& profileData = Profiler::Instance().Profile();             // a reference to the profiler storage
    static std::vector<double> temp;                                // temp buffer to store the profiled values with a running average
    static std::map<std::string, std::tuple<double,int,std::deque<double>>> temp2; // The monstrosity
    static std::chrono::steady_clock::time_point prevTime{ std::chrono::high_resolution_clock::now() };
    temp.resize(size);

    if (segment.empty()) {
        segment.reserve(size);
        for (int i{}; i < size; i++) {
            segment.push_back(i / 5.);
        }
    }

    // Adding names to the vector if there is a difference
    if (profileNames.size() != profileData.size()) {
        for (auto& pair : profileData) {
            if (!std::any_of(profileNames.begin(), profileNames.end(), Equal(pair.first))) {
                profileNames.push_back(pair.first);
                profileBool[pair.first + value] = false;
                profileBool[pair.first + graph] = false;
                std::get<0>(temp2[pair.first]) = 0.;
                std::get<1>(temp2[pair.first]) = 0;
            }
        }
    }

    // Running the updates for the graph data outside of the imgui draw call
    // as if it is inside, the data wont update properly since the code wont
    // run if header is not open
    size_t timeDiff = static_cast<size_t>(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - prevTime).count() / 1000000.);
    for (const std::string& s : profileNames) {
        auto timeStore = profileData.at(s).ts;
        if (timeStore->storage.size() < 2) continue;
        std::deque<double>& data = std::get<2>(temp2[s]);
        double& time = std::get<0>(temp2[s]);
        int& count = std::get<1>(temp2[s]);
        time += timeStore->storage[timeStore->storage.size() - 2];
        count++;
        if (timeDiff >= 200) {
            if (data.size() == size) data.pop_front();
            data.push_back(time / static_cast<double>(count));
            time = 0;
            count = 0;
        }
    
    }
    if (timeDiff >= 200) prevTime = std::chrono::high_resolution_clock::now();

    if (ImGui::Begin("Profiler")) {
        // Checkboxes
        if (ImGui::CollapsingHeader("Profiling options")) {
            for (auto& pair : profileBool) {
                ImGui::Checkbox(pair.first.c_str(), &pair.second);

            }
        }

        
        // Graph window
        if (ImGui::CollapsingHeader("Graph")) {
            if (ImPlot::BeginPlot("Time Taken")) {
                ImPlot::SetupAxisLimits(ImAxis_X1, 0., size / 5., ImPlotCond_Always);
                
                for (const std::string& s : profileNames) {
                    if (profileBool[s + graph]) {
                        // drawing the raw values of draw timr
                        auto timeStore = profileData.at(s).ts;
                        if (timeStore) {
                            std::deque<double>& data = std::get<2>(temp2[s]);
                            ImPlot::PlotLine(s.c_str(), segment.data(), std::vector<double>(data.begin(), data.end()).data(), static_cast<int>(data.size()));
                        }

                        //// drawing the 1min running average
                        //auto averageStore = profileData.at(s).avgs;
                        //if (averageStore) {
                        //    size_t inc = averageStore->size() / size;
                        //    std::copy_if(averageStore->begin(), averageStore->end(), temp.begin(), [copyCount = 0, val = 0, inc](const auto& a) mutable {
                        //        (void)a; // not needed
                        //        val++;
                        //        if (copyCount == 300) return false;
                        //        if (val == inc) {
                        //            val = 0;
                        //            copyCount++;
                        //            return true;
                        //        }
                        //        else return false;
                        //        });
                        //    ImPlot::PlotLine((s + " average").c_str(), segment.data(), temp.data(), static_cast<int>(temp.size()));
                        //    std::fill_n(temp.begin(), size, 0.);
                        //}

                    }

                }
                ImPlot::EndPlot();
            }
        }
        // Text values
        if (ImGui::CollapsingHeader("Values")) {
            for (const std::string& s : profileNames) {
                if (profileBool[s + value]) {
                    ImGui::Text("%s stats:\nCurrent Runtime: %lfms, 1min Average: %lfms\n",s.c_str(),profileData.at(s).currentTime,profileData.at(s).minuteAverage);
                    ImGui::Text("99%% maximum time: %lfms, 95%% maximum time: %lfms\n", profileData.at(s)._99p, profileData.at(s)._95p);
                }
            }
        }
        
    }
    ImGui::End();
#endif
}

void Editor::AssetBrowserWindow(Registry& registry) {
#ifdef PLATFORM_WINDOWS
    
    if (ImGui::Begin("Assets")) {
        ImVec2 pos {0, 0};
#ifndef HAS_MFC
        // Guide for if mfc isnt installed
        ImGui::Text("MFC is not detected, so the drag and drop feature from file explorer would not work. \n"
                    "If you do have MFC installed, just recompile the project.\n"
                    "Drag and drop within imgui will still work normally.");
        if (ImGui::CollapsingHeader("Installation Guide")) {
            ImGui::Text("You will need to open the Visual Studio Installer application.\n"
                        "Then click on the \"modify\" button on the right side for the visual studio you are using\n"
                        "Under the \"Desktop development with C++\" dropdown, click the option that says \"C++ MFC for latest... \"\n"
                        "Click the modify button bottom right and recompile the program once it's done.\n");
        }
#endif

        // Flag for the header to be opened by default
        if (ImGui::CollapsingHeader("Folder", 32)) {
            ResourceManager& rm = *CEO::Instance().GetManager<ResourceManager>();
            static float buttonSize{128.f};                                 // Size of the icons. Default is medium
            int textfileId = rm.GetTexture("textfile.png").TexId();         // openGL texture ID for the folder icon
            int folderId = rm.GetTexture("folder.png").TexId();             // openGL texture ID for the default file icon
            static std::set<std::string> texNames;                          // Storing the names of all the pictures
            float w = ImGui::GetColumnWidth();                              // Width of the widget. Used for placing the icons
            constexpr float spacing{ 10.f };                                // Custom spacing between icons
            static filepath currPath{ mainPath };                           // Current directory we are at
            static ImVec2 deleteCursorPos{};                                // The position of the mouse cursor to be used to display the delete popup
            static std::string deleteFileName{};                            // Name of the file we may or may not want to delete

            // Filling out the texture name vector if resourcemanager has loaded in more textures
            if (texNames.size() != rm.TextureStorage().size()) {
                for (const std::pair<const std::string, TextureObj>& p : rm.TextureStorage()) {
                    filepath fp{ p.first };
                    if (texNames.find(fp.filename().string()) == texNames.end())
                        texNames.insert(fp.filename().string());
                }
            }
            

            ImGui::Text("Icon Size: ");
            ImGui::SameLine();
            if (ImGui::Button(" Small ")) {
                buttonSize = 64.f;
                listFormat = false;
            }
            ImGui::SameLine();
            if (ImGui::Button(" Medium ")) {
                buttonSize = 128.f;
                listFormat = false;
            }
            ImGui::SameLine();
            if (ImGui::Button(" Large ")) {
                buttonSize = 256.f;
                listFormat = false;
            }
            ImGui::SameLine();
            if (ImGui::Button(" List ")) {
                listFormat = true;
            }
            // Goes to parent directory. Cannot exit out of Assets folder
            if (ImGui::Button("  Back  ")) {
                if (currPath.parent_path().filename() != ENGINENAME) {
                    currPath = currPath.parent_path();
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("  Sync  ")) {
                SyncAssetsFolders();
            }

            static ImVec2 nextPos{};
            static int MaxyDisp{}; // To handle long text filenames
            nextPos = { 0,0 };
            MaxyDisp = -1;

            // Iterates through each currently available directory and draws out their icons.
            std::for_each(std::filesystem::directory_iterator(currPath), std::filesystem::directory_iterator(), [this, spacing, w, folderId, textfileId, &rm, &registry, listFormat = listFormat ](const directory& d) {
                std::string name = d.path().filename().string().c_str();
                if (listFormat) {
                    if (d.is_directory()) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.996f, 0.949f, 0.f, 1.f));
                        if (ImGui::Selectable(("\\" + name + "\\").c_str(), false, 0, { ImGui::GetWindowWidth(),16.f })) {
                            if (d.is_directory()) {
                                currPath = d.path();
                            }
                        }
                        ImGui::PopStyleColor();
                    }
                    else {
                        ImGui::Selectable((name).c_str(), false, 0, { ImGui::GetWindowWidth(),16.f });
                    }
                }
                else {
                    int texid;

                    if (d.is_directory()) texid = folderId;
                    else {
                        if (texNames.find(d.path().filename().string()) == texNames.end()) {
                            texid = textfileId;

                            // This checks if the file is an unloaded texture file, then tries to load it from resourcemanager.
                            if (std::any_of(imgExt.begin(), imgExt.end(), [&d, this](const std::string& s) { return d.path().extension().string() == s; })) {

                                std::string s = d.path().string();
                                rm.GetTexture(s.substr(s.find("Textures\\") + std::string("Textures\\").size()));
                            }
                        }
                        else {
                            std::string s = d.path().string();
                            texid = rm.GetTexture(s.substr(s.find("Textures\\") + std::string("Textures\\").size())).TexId();
                        }
                    }

                    // Setting icon position manually
                    if (nextPos.x == 0 && nextPos.y == 0) {
                        nextPos = ImGui::GetCursorPos();
                    }
                    else ImGui::SetCursorPos(nextPos);


                    if (ImGui::ImageButton(name.c_str(), texid, { buttonSize,buttonSize }, { 0.f,1.f }, { 1.f,0.f }, { 0.f,0.f,0.f,1.f })) {
                        if (d.is_directory()) {
                            currPath = d.path();
                        }
                    }
                }

                if (ImGui::GetIO().MouseClicked[1]) { /* right clicked */
                    ImVec2 mousePos = ImGui::GetMousePos();
                    ImVec2 windowPos = ImGui::GetWindowPos();

                    windowPos.y -= ImGui::GetScrollY();

                    if (!listFormat) {
                        windowPos.x += nextPos.x;
                        windowPos.y += nextPos.y;
                        if (mousePos.x > windowPos.x && mousePos.x < windowPos.x + buttonSize &&
                            mousePos.y > windowPos.y && mousePos.y < windowPos.y + buttonSize &&
                            !d.is_directory()) {
                            ImGui::OpenPopup("DELETE");
                            deleteCursorPos = ImGui::GetMousePos();
                            deleteFileName = d.path().string();
                        }
                    }
                    else {
                        windowPos.x += ImGui::GetCursorPos().x;
                        windowPos.y += ImGui::GetCursorPos().y;
                        windowPos.y -= ImGui::GetScrollY();

                        float width = ImGui::GetWindowWidth();
                        float height = 16.f;

                        if (mousePos.x > windowPos.x && mousePos.x < windowPos.x + width &&
                            mousePos.y < windowPos.y && mousePos.y > windowPos.y - height &&
                            !d.is_directory()) {
                            ImGui::OpenPopup("DELETE");
                            deleteCursorPos = ImGui::GetMousePos();
                            deleteFileName = d.path().string();
                        }
                    }
                }

                if (d.is_directory()) {
                    // If something is dropped in
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE")) {
                            filepath source = (const char*)payload->Data;
                            filepath relpath = std::filesystem::relative(d.path(), mainPath);
                            UpdateAssetsFolders(source.string(), relpath.string());
                            relpath = std::filesystem::relative(source, mainPath);
                            UpdateAssetsFolders("", relpath.string(), true);
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                else /*this is a file*/ {
                    if(d.path().extension() == ".scene"){
                        if(ImGui::IsItemClicked()) {
                            selectedEntityId = 0;    // reset selected entity
                            // Clear selection when changing scenes.
                            // Clear the command buffer, including the clipboard.
                            buffer.Clear(true);
                            CEO::Instance().GetManager<ResourceManager>()->StopAllAudio();

                            Editor::state = Editor::EditorState::Edit;
                            sceneName = d.path().stem().string();

                            selectedPrefab.first.clear();
                            selectedPrefab.second= 0;
                            displayPrefabHierarchy = false;
                            SceneManager::QueueSceneAction(d.path().stem().string(),SceneManager::CHANGE);
                        }
                    }
                    else if (d.path().extension() == ".prefab") {

                        bool clicked =
                            ImGui::IsItemHovered() &&
                            ImGui::IsMouseReleased(ImGuiMouseButton_Left) &&
                            !ImGui::IsMouseDragging(ImGuiMouseButton_Left);

 
                        if (clicked) {
                            if (selectedPrefab.second != 0) {
                                // Delete previous dummy entity
                                for (Registry::Entity ent : registry.GetDummyEntities()) {
                                    registry.DestroyEntity(ent);
                                }
                            }
                            selectedPrefab.first = d.path().stem().string();
                            selectedPrefab.second = CEO::Instance().GetManager<ResourceManager>()->LoadDummyPrefab(registry, d.path().stem().string());
                            selectedEntityId = selectedPrefab.second;    // set selected entity to dummy entity
                        }
                    }
                    else if (d.path().extension() == ".tileset") {
                        if (ImGui::IsItemClicked()) {
                            tilesetEditor.SetSelectedTileset(CEO::Instance().GetManager<ResourceManager>()->GetTileset(std::string{ d.path().filename().string() }));
                        }
                    }
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAutoExpirePayload)) {
                        ImGui::Text("Dragging %s", name.c_str());
                        ImGui::Text("%s", d.path().string().c_str());

                        if (d.path().extension() == ".prefab") {
                            ImGui::SetDragDropPayload("PREFAB", d.path().stem().string().c_str(), d.path().string().size() + 1);
                        }
                        else if (d.path().extension() == ".Script") {
                            ImGui::SetDragDropPayload("SCRIPT", d.path().stem().string().c_str(), d.path().string().size() + 1);
                        }
                        else {
                            ImGui::SetDragDropPayload("FILE", d.path().string().c_str(), d.path().string().size() + 1);
                        }
                        ImGui::EndDragDropSource();
                    }
                }
                if (!listFormat) {
                    // Logic for centering the text + dealing with text wrapping as ImGui flushes text to the left
                    ImVec2 size = ImGui::CalcTextSize(name.c_str());
                    static ImVec2 textSize = ImGui::CalcTextSize("x");
                    int textCount = static_cast<int>(buttonSize / textSize.x);
                    int yDisp{};

                    // If wrapping is needed
                    while (size.x > buttonSize) {
                        ImGui::SetCursorPos({ nextPos.x, nextPos.y + spacing * (yDisp + 1) + buttonSize });
                        ImGui::Text(name.substr(yDisp * textCount, textCount).c_str());

                        size.x -= buttonSize;
                        yDisp++;
                    }
                    // Centering the text
                    float disp = (buttonSize - size.x) / 2;
                    ImGui::SetCursorPos({ nextPos.x + disp, nextPos.y + spacing * (yDisp + 1) + buttonSize });
                    ImGui::Text(name.substr(yDisp * textCount, textCount).c_str());
                    if (yDisp > MaxyDisp) MaxyDisp = yDisp;

                    // Setting the position for the next icon.
                    nextPos.x += buttonSize + spacing * 2;
                    if (nextPos.x > w - buttonSize) {
                        nextPos.x = spacing;
                        nextPos.y += buttonSize + spacing * 3 + MaxyDisp * spacing;
                        MaxyDisp = -1;
                    }
                }

                });

            // Ghost folder that is meant to allow moving files out of directories
            // Also intentionally rendered upside down for the funsies (and to make it stand out)
            if (currPath != mainPath) {
                if (listFormat) {
                    if (ImGui::Selectable("\\..")) {
                        currPath = currPath.parent_path();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE")) {
                            filepath source = (const char*)payload->Data;
                            filepath relpath = std::filesystem::relative(currPath, mainPath);
                            UpdateAssetsFolders(source.string(), relpath.string() + "\\..");
                            relpath = std::filesystem::relative(source, mainPath);
                            UpdateAssetsFolders("", relpath.string(), true);
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
                else {
                    if (ImGui::ImageButton("wallahi", folderId, { buttonSize,buttonSize }, { 1.f,0.f }, { 0.f,1.f }, { 0.f,0.f,0.f,1.f })) {
                        currPath = currPath.parent_path();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE")) {
                            filepath source = (const char*)payload->Data;
                            filepath relpath = std::filesystem::relative(currPath, mainPath);
                            UpdateAssetsFolders(source.string(), relpath.string() + "\\..");
                            relpath = std::filesystem::relative(source, mainPath);
                            UpdateAssetsFolders("", relpath.string(), true);
                        }
                        ImGui::EndDragDropTarget();
                    }
                    static ImVec2 size2 = ImGui::CalcTextSize("\\..");
                    float disp2 = (buttonSize - size2.x) / 2;
                    ImGui::SetCursorPos({ spacing + disp2, nextPos.y + spacing + buttonSize + (MaxyDisp == -1 ? 0 : spacing * 3 + buttonSize) });
                    ImGui::Text("\\..");
                }
            }

            // small information tab to know which folders will get updated together
            ImGui::NewLine();
            if (ImGui::CollapsingHeader("Linked folders")) {
                ImGui::Text("Main folder: %s", currPath.string().c_str());
                for (filepath& p : assetPath) {
                    if (p != currPath) {
                        ImGui::Text("Other linked folder: %s", p.string().c_str());
                    }
                }
            }

            // Popup for the delete window
            ImGui::SetNextWindowPos(deleteCursorPos);
            if (ImGui::BeginPopup("DELETE")) {
                if (ImGui::Button("   delete    ")) {
                    ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                    ImGui::OpenPopup("CONFIRMATION");                    
                }
                else {
                    ImGui::EndPopup();
                }
            }

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImVec2 size(100, 50);
            center.x -= size.x / 2; center.y -= size.y / 2;
            ImGui::SetNextWindowSize(size);
            ImGui::SetNextWindowPos(center);
            if (ImGui::BeginPopup("CONFIRMATION")) {
                
                ImGui::GetIO().MouseClicked[0] = false; // Disabling clicking outisde

                ImGui::Text("ARE YOU SURE?");
                if (ImGui::Button("yes")) {
                    filepath relpath = std::filesystem::relative(deleteFileName, mainPath);



                    UpdateAssetsFolders("", relpath.string(), true);
                    deleteFileName = ""; // Clearing it to prevent accidents

                    if (relpath.extension().string() == ".Script")
                    {
                        //get copy of Project directory
                        filepath SrcCodeDirectory = ProjectDirectory;

                        //append to reach src files
                        SrcCodeDirectory /= "src";
                        SrcCodeDirectory /= "Scripts";
                        SrcCodeDirectory /= relpath.stem();

                        filepath headerFile = SrcCodeDirectory;
                        headerFile += ".h";
                        filepath sourceFile = SrcCodeDirectory;
                        sourceFile += ".cpp";
                        if (std::filesystem::exists(headerFile) && std::filesystem::exists(sourceFile))
                        {
                            RemoveScriptFile = true;
                        }
                        if (std::filesystem::exists(headerFile))
                        {
                            std::filesystem::remove(headerFile);
                        }
                        else
                        {
                            LOGE("file %s cannot be found", headerFile.string().c_str());
                        }
                        if (std::filesystem::exists(sourceFile))
                        {
                            std::filesystem::remove(sourceFile);
                        }
                        else
                        {
                            LOGE("file %s cannot be found", headerFile.string().c_str());
                        }
                    }
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();
                if (ImGui::Button("no")) {
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

        }
        ImVec2 window_content_region = ImGui::GetContentRegionMax();
        window_content_region.y = ImGui::GetCursorPosY();
        ImGui::SetCursorPos(pos);

        /*Drag drop region for entire asset folder*/ {
            ImGui::Dummy(window_content_region);
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHY_ENTITY"))
                {
                    Registry::Entity data = *(Registry::Entity*)payload->Data;
                    if (PrefabManager::CreatePrefab(CEO::Instance().GetManager<Registry>()->GetComponent<NameComponent>(data)->name, data)) {
#ifdef PLATFORM_WINDOWS
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
#endif
                    }

                }
                ImGui::EndDragDropTarget();
            }
        }


    }
    ImGui::End();
#endif
}

void Editor::AssetBuildSizeWindow() {
    // Setting a minimum size as file names can get preeetty long
    ImGui::SetNextWindowSizeConstraints({ 400.f,20.f }, { FLT_MAX,FLT_MAX });
    ResourceManager* rm = CEO::Get<ResourceManager>();
    static std::map<filepath, bool> sceneNames;                 // Map of all the scenes, for the Select from scene button
    static std::set<filepath> exportFiles;                      // Seperate set storing all selected assets.
    static std::set<filepath> dirSet;                           // Stores all the directories right below Assets. Used for scenefile parsing. (Kind of a self inflicted issue ig...)
    static std::set<std::string> prefabNames;                   // whatever them prefabs are called
    static char exportPath[1024]{ '\0' };                       // Where to export the files to
    static std::string exportPopup{ "Export window" };          // Used for the ImGui popup window
    static bool scenePopup{ false };                            // Pop up window for scene select option.
    int textfileId = rm->GetTexture("textfile.png").TexId();    // openGL texture ID for the folder icon
    static size_t totalFilesize{};                              // total selected filesize

    auto updateAssetNames = [mainPath = mainPath]() {
        std::for_each(std::filesystem::recursive_directory_iterator(mainPath), std::filesystem::recursive_directory_iterator(), [mainPath](directory dir) {
            if (dir.is_directory()) {
                // do nothing
            }
            else {
                filepath relPath = std::filesystem::relative(dir.path(), mainPath);
                filepath parentPath = relPath;
                if (parentPath.remove_filename() == filepath("Scenes\\") ||
                    parentPath.remove_filename() == filepath("Prefabs\\"))
                    sceneNames.insert({ relPath,false });
                if (parentPath == filepath("Prefabs\\"))
                    prefabNames.insert(relPath.stem().string());
            }
        });
    };

    auto updateDirFolders = [mainPath = mainPath]() {
        std::for_each(std::filesystem::directory_iterator(mainPath), std::filesystem::directory_iterator(), [mainPath](directory dir) {
            if (dir.is_directory()) {
                filepath relPath = std::filesystem::relative(dir.path(), mainPath);
                dirSet.insert(relPath);
            }
        });
    };

    if (sceneNames.empty()) {
        updateAssetNames();
        updateDirFolders();
        for (const auto& p : assetSelectionMap) {
            if (p.second) {
                exportFiles.insert(p.first);
                totalFilesize += std::filesystem::file_size("Assets\\" / p.first);
            }
        }
    }

    if (ImGui::Begin("Build Size Analyzer")) {

        if (ImGui::Button("    Select All   ")) {
            totalFilesize = 0;
            for (auto& p : assetSelectionMap) {
                p.second = true;
                exportFiles.insert(p.first);
                totalFilesize += std::filesystem::file_size("Assets\\" / p.first);
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("   Deselect All  ")) {
            for (auto& p : assetSelectionMap) p.second = false;
            exportFiles.clear();
            totalFilesize = 0;
        }
        ImGui::SameLine();
        if (ImGui::Button("Select from Scene")) {
            scenePopup = !scenePopup;
        }
        ImGui::SameLine(); 
        if (ImGui::Button("    Export To    ")) {
            ImGui::OpenPopup(exportPopup.c_str());
        }
        ImGui::SameLine();
        if (ImGui::Button("  Update Assets  ")) {
            updateAssetNames();
            updateDirFolders();
        }

        if (totalFilesize < 1024) {
            ImGui::Text("Total selected filesize: %dB", totalFilesize);
        }
        else if (totalFilesize < (size_t)1024 * 1024 * 2) {
            double dsize = totalFilesize / 1024.;
            ImGui::Text("Total selected filesize: %.2lfKB", dsize);
        }
        else {
            double dsize = totalFilesize / (1024. * 1024);
            ImGui::Text("Total selected filesize: %.2lfMB", dsize);
        }
        
        ImGui::Text("Assets:");
        int n{};
        if (ImGui::BeginTable("table", 4, ImGuiTableFlags_ScrollY))
        {

            // Setup for formatting
            ImGui::PushID(n++);
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);    // Image
            ImGui::PushID(n++); ImGui::PopID();
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);  // Filename
            ImGui::PushID(n++); ImGui::PopID();
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);    // Size
            ImGui::PushID(n++); ImGui::PopID();
            ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);    // Checkbox
            ImGui::PopID();
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();
            for (auto& p : assetSelectionMap)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                int texId;
                if (std::any_of(imgExt.begin(), imgExt.end(), [&p, rm](const std::string& s) { return p.first.extension().string() == s; })) {
                    std::string s = p.first.string();
                    texId = rm->GetTexture(s.substr(s.find("Textures\\") + std::string("Textures\\").size())).TexId();
                }
                else {
                    texId = textfileId;
                }

                ImGui::Image(texId, { 32,32 }, { 0.f,1.f }, { 1.f,0.f });
                ImGui::TableNextColumn();
                ImGui::Text(p.first.string().c_str());
                ImGui::TableNextColumn();
                size_t size = std::filesystem::file_size("Assets\\" / p.first);
                if (size < 1024) {
                    ImGui::Text("%dB", size);
                }
                else if (size < (size_t)1024 * 1024 * 2) {
                    double dsize = size / (1024.);
                    ImGui::Text("%.2lfKB", dsize);
                }
                else {
                    double dsize = size / (1024. * 1024);
                    ImGui::Text("%.2lfMB", dsize);
                }
                ImGui::TableNextColumn();
                ImGui::PushID(n);
                if (ImGui::Checkbox("", &(p.second))) {
                    // If selected
                    if (p.second) {
                        exportFiles.insert(p.first);
                        totalFilesize += size;
                    }
                    // If deselected
                    else {
                        exportFiles.erase(p.first);
                        totalFilesize -= size;
                    }
                }
                ImGui::PopID();
                n++;
            }
            ImGui::EndTable();
        }
    }

    if (ImGui::BeginPopup(exportPopup.c_str())) {
        ImGui::Text("Filepath: ");
        ImGui::SameLine();
        ImGui::PushID(0);
        ImGui::InputText("", exportPath, 1024);
        ImGui::PopID();
        filepath export1(exportPath);
        export1 = std::filesystem::absolute(export1);

        std::error_code ec;
        if (export1.empty()) {
            ImGui::Text("Empty directory not allowed.");
        }
        else if (std::any_of(assetPath.begin(), assetPath.end(), [&export1, &ec](filepath& asset) { return std::filesystem::equivalent(export1, asset, ec); })) {
            ImGui::Text("This is an active build directory. Please choose another one to export to.");
        }
        else {
            if (std::filesystem::exists(export1)) {
                ImGui::Text("Directory exists.\nWill be exporting to directory: \n%s", export1.string().c_str());
            }
            else {
                ImGui::Text("Directory does not exist.\nA new directory will be created at: \n%s", export1.string().c_str());
            }
            if (ImGui::Button("Export")) {
                if (!std::filesystem::exists(export1)) {
                    std::filesystem::create_directories(export1);
                }
                else {
                    // rmdir deletes the everything within the folder if it existed beforehand
                    system(("rmdir " + export1.string() + " /q /s").c_str());
                }

                for (const filepath& fp : exportFiles) {
                    filepath parent = fp.parent_path();
                    filepath finalExport = export1 / parent;
                    if (!std::filesystem::exists(finalExport)) {
                        std::filesystem::create_directories(finalExport);
                    }
                    // Will need to change this to some form of xcopy parse probably.
                    system(("copy \"Assets\\" + fp.string() + "\" \"" + finalExport.string() + "\" /y ").c_str());
                }

            }
        }
        
        ImGui::EndPopup();
    }

    if (scenePopup) {

        ImGui::SetNextWindowSizeConstraints({ 400.f,20.f }, { FLT_MAX,FLT_MAX });
        static float windowX{ 400.f }, windowY{ 400.f };
        ImGui::SetNextWindowSize({ windowX,windowY });
        if (ImGui::Begin("Scene/Prefab select:")) {
            if (ImGui::Button("  Close  ")) {
                scenePopup = false;
            }
            if (ImGui::BeginTable("table2", 3, ImGuiTableFlags_ScrollY)) {
                int n{};
                // Setup for formatting
                ImGui::PushID(n++);
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);    // Image
                ImGui::PushID(n++); ImGui::PopID();
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch);  // Filename
                ImGui::PushID(n++); ImGui::PopID();
                ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);    // Checkbox
                ImGui::PopID();
                ImGui::TableSetupScrollFreeze(0, 1);
                ImGui::TableHeadersRow();

                for (auto& p : sceneNames)
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Image(textfileId, { 32,32 }, { 0.f,1.f }, { 1.f,0.f });
                    ImGui::TableNextColumn();
                    ImGui::Text(p.first.string().c_str());
                    ImGui::TableNextColumn();
                    ImGui::PushID(n);
                    if (ImGui::Checkbox("", &(p.second))) {
                        // Select itself in the main map
                        assetSelectionMap.at(p.first.string()) = p.second;

                        std::stringstream buf = CEO::Get<FileManager>()->ReadFile("Assets\\" + p.first.string());
                        std::set<std::string> strings;
                        std::queue<std::string> prefabs;
                        std::string s;
                        while (std::getline(buf,s,'\"')) {
                            if (s[0] != ':') {
                                strings.insert(s);
                                // Checking if it is a prefab
                                // Kind of a stupid solution...
                                if (prefabNames.find(s) != prefabNames.end()) {
                                    prefabs.push(s + ".prefab");
                                }
                            }
                        }
                        
                        // Parsing the prefabs within the scene...
                        while (!prefabs.empty()) {
                            std::string name = prefabs.front();
                            prefabs.pop();
                            std::stringstream prefabBuf = CEO::Get<FileManager>()->ReadFile("Assets\\Prefabs\\" + name);
                            strings.insert(name);
                            sceneNames.at("Prefabs\\" + name) = p.second;

                            // Do the same parsing as above basically. copy pasted code YIPPIE
                            while (std::getline(prefabBuf, s, '\"')) {
                                if (s[0] != ':') {
                                    strings.insert(s);
                                    // For nested prefabs.
                                    if (prefabNames.find(s) != prefabNames.end()) {
                                        if (s + ".prefab" != name)
                                            prefabs.push(s + ".prefab");
                                    }
                                }
                            }
                        }

                        for (std::string s1 : strings) {
                            auto it = std::remove_if(s1.begin(), s1.end(), [pc = '\0'](char& c) mutable {
                                bool ret = pc == c;
                                pc = c;
                                return ret;
                            });
                            s1.erase(it, s1.end());

                            // Ok this is really terrible but I really can't find a better way to do this tbh...
                            // Basically appending the directory headers to the filenames to bruteforce
                            // which directory they are under. Technically can hardcode but I don't feel
                            // good about doing it.
                            std::vector<filepath> appendList;
                            appendList.push_back(s1);
                            for (const filepath& fp : dirSet) {
                                appendList.push_back(fp / s1);
                            }
                            for (const filepath& fp : appendList) {
                                if (assetSelectionMap.find(fp) != assetSelectionMap.end()) {
                                    assetSelectionMap.at(fp) = p.second;
                                    break;
                                }
                            }

                            
                        }
                    }
                    ImGui::PopID();
                    n++;
                }
                ImGui::EndTable();
            }

            windowX = ImGui::GetWindowWidth();
            windowY = ImGui::GetWindowHeight();
        }

        ImGui::End();
        
    }

    ImGui::End();

    
}
    
#ifdef HAS_MFC
BOOL Editor::DragDrop::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point) {
    if (dropEffect != DROPEFFECT_COPY) return false;
    (void)pWnd; (void)point; // warning prevention

    if (!pDataObject->IsDataAvailable(CF_HDROP)) {
        std::cout << "Current type not supported\n";
        return false;
    }

    std::cout << "file dropped\n";
    STGMEDIUM stgm; // A struct that encapsulates the data.

    if (!pDataObject->GetData(CF_HDROP, &stgm, NULL)) {
        std::cout << "Input Data failed\n";
        ReleaseStgMedium(&stgm);
        return false;
    }
    
    HGLOBAL hg = stgm.hGlobal;
    DROPFILES* data = static_cast<DROPFILES*>(GlobalLock(hg));
    if (!data) {
        std::cout << "Global data get failed\n";
        GlobalUnlock(stgm.hGlobal);
        GlobalFree(stgm.hGlobal);
        ReleaseStgMedium(&stgm);
        return false;
    }
    // the pfiles member of the dropfiles struct indicates the offset from the pointer to the dropfiles struct to the start of the character array.
    // yeah reinterpret_cast is actually needed here
    char* c = reinterpret_cast<char*>(data) + data->pFiles;
    do {
        std::stringstream fileName;
        while (*c || *(c + 1)) {
            if (*c) fileName << *c;
            c++;
        }
        std::cout << fileName.str() << '\n';
        Editor::Instance().UpdateAssetsFolders(fileName.str());
        c+=3;
    } while (*c);

    GlobalUnlock(stgm.hGlobal);
    GlobalFree(stgm.hGlobal);
    ReleaseStgMedium(&stgm);
    
    return true;
}
#endif

void Editor::UpdateAssetsFolders(const std::string& filename, const std::string& _relpath, bool remove) {
    std::string relpath = "\\" + _relpath;
    if (!remove) { // Copying over to the asset folders

        std::stringstream data{ CEO::Instance().GetManager<FileManager>()->ReadFile(filename, false, std::ios_base::binary | std::ios_base::in) };

        for (const filepath& fp : assetPath) {
            std::string endFilename = filepath(filename).filename().string();
            std::string combined = fp.string() + relpath + "\\" + endFilename;
            CEO::Instance().GetManager<FileManager>()->EditorWriteFile(combined, data, std::ios_base::binary | std::ios_base::out);
        }
    }

    else {
        if (relpath == "\\") return;
        for (const filepath& fp : assetPath) {
            std::filesystem::remove(fp.string() + relpath);
        }
    }
}

void Editor::SyncAssetsFolders() {
    std::string name = CEO::Instance().GetManager<SceneManager>()->BaseScene();

    CEO::Instance().GetManager<ResourceManager>()->Free();
    for (const filepath& p : assetPath) {
        if (p != mainPath) {
            // rmdir deletes the Asset directory within the other folders
            system(("rmdir " + p.string() + " /q /s").c_str());
            // Then xcopy copies from the main assets folder back to the removed directory
            system(("xcopy \"" + mainPath.string() + "\" \"" + p.string() + "\\\" /s /e /y /q").c_str());
        }
    }
    CEO::Instance().GetManager<ResourceManager>()->Init();
    
    CEO::Instance().GetManager<SceneManager>()->QueueSceneAction(name, SceneManager::CHANGE);
}
#endif
#endif