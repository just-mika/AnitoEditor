#include "AssetBrowserPanel.h"
#include "LevelDataManager.h"
#include "EditorModule.h"
#include "SpriteRenderer.h"
#include "imgui.h"
#include <SDL3/SDL.h>
#include <algorithm>

namespace
{
    constexpr const char* kPopupId = "Choose Texture";

    std::string toLower(std::string s)
    {
        for (char& c : s) c = (char)SDL_tolower(c);
        return s;
    }

    bool isImage(const std::string& name)
    {
        std::string lower = toLower(name);
        return lower.size() > 4 &&
            (lower.compare(lower.size() - 4, 4, ".png") == 0 || lower.compare(lower.size() - 4, 4, ".jpg") == 0);
    }

    //where SDL_EnumerateDirectory puts the names it finds
    struct Listing
    {
        std::vector<std::string>* folders;
        std::vector<std::string>* images;
    };

    SDL_EnumerationResult collectEntry(void* userdata, const char* dirname, const char* fname)
    {
        Listing* listing = static_cast<Listing*>(userdata);
        std::string full = std::string(dirname) + fname;   //dirname ends with a slash

        SDL_PathInfo info;
        if (SDL_GetPathInfo(full.c_str(), &info) && info.type == SDL_PATHTYPE_DIRECTORY)
            listing->folders->push_back(fname);
        else if (isImage(fname))
            listing->images->push_back(fname);

        return SDL_ENUM_CONTINUE;
    }
}


// lists the subfolders and images in the current folder
void AssetBrowserPanel::refresh()
{
    folders.clear();
    images.clear();

    Listing listing = { &folders, &images };
    std::string path = "Assets/" + currentDir;
    SDL_EnumerateDirectory(path.c_str(), collectEntry, &listing);

    std::sort(folders.begin(), folders.end());
    std::sort(images.begin(), images.end());
}

void AssetBrowserPanel::draw()
{
    //popups must be opened from the same place they're drawn, so the button only sets a flag
    if (bPendingOpen)
    {
        bPendingOpen = false;
        searchBuf[0] = '\0';

        AGameObject* sel = Editor::EditorModule::getInstance()->getSelectedObject();
        bForAnimation = sel && sel->findComponentByName("SpriteAnimator");
        if (bForAnimation)
            currentDir = "animations/";   //start where the animations are

        refresh();
        ImGui::OpenPopup(kPopupId);
    }

    //same layout as the Load panel
    const ImVec2 ds = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(ds.x * 0.5f, ds.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(ds.x * 0.40f, 0.0f), ImGuiCond_Appearing);

    bool open = true;
    if (!ImGui::BeginPopupModal(kPopupId, &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        return;

    ImGui::TextDisabled("Assets/%s", currentDir.c_str());
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputTextWithHint("##search", "Search this folder", searchBuf, sizeof(searchBuf));
    std::string filter = toLower(searchBuf);

    //folder changes are applied after the list is drawn, so the lists don't change while looping over them
    std::string nextDir;
    bool bChangeDir = false;

    if (ImGui::BeginChild("##assetlist", ImVec2(0, 320.0f), ImGuiChildFlags_Borders))
    {
        if (!currentDir.empty() && ImGui::Selectable("..", false, 0, ImVec2(0, 32.0f)))
        {
            size_t slash = currentDir.find_last_of('/', currentDir.size() - 2);
            nextDir = (slash == std::string::npos) ? "" : currentDir.substr(0, slash + 1);
            bChangeDir = true;
        }

        for (const std::string& f : folders)
        {
            if (!filter.empty() && toLower(f).find(filter) == std::string::npos) continue;

            std::string label = "[" + f + "]";
            if (ImGui::Selectable(label.c_str(), false, 0, ImVec2(0, 32.0f)))
            {
                nextDir = currentDir + f + "/";
                bChangeDir = true;
            }
        }

        for (const std::string& img : images)
        {
            if (!filter.empty() && toLower(img).find(filter) == std::string::npos) continue;

            //animated sprites use the whole folder, so frames are only listed for reference
            if (bForAnimation)
            {
                ImGui::TextDisabled("%s", img.c_str());
                continue;
            }

            if (ImGui::Selectable(img.c_str(), false, 0, ImVec2(0, 32.0f)))
            {
                applyTexture(currentDir + img);
                ImGui::CloseCurrentPopup();
                break;
            }
        }

        if (folders.empty() && images.empty())
            ImGui::TextDisabled("No images here.");
    }
    ImGui::EndChild();   //always paired with BeginChild

    if (bChangeDir)
    {
        currentDir = nextDir;
        searchBuf[0] = '\0';
        refresh();
    }

    if (bForAnimation)
    {
        ImGui::BeginDisabled(images.empty());   //only folders that actually contain frames
        if (ImGui::Button("Use this folder", ImVec2(-FLT_MIN, 0)))
        {
            //currentDir ends with '/', the saved path doesn't ("animations/pause_animation")
            applyTexture(currentDir.substr(0, currentDir.size() - 1));
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();
    }

    ImGui::EndPopup();
}

// puts the chosen image on the selected object's sprite
// (reusing the scene's texture if it already loaded this file, otherwise loading it)
void AssetBrowserPanel::applyTexture(const std::string& path)
{
    AGameObject* selected = Editor::EditorModule::getInstance()->getSelectedObject();
    if (selected)
        LevelDataManager::setObjectTexture(selected, path);
}

