#include "ObjectPalette.h"
#include "GameObjectManager.h"
#include "LevelDataManager.h"
#include "Camera.h"
#include "Settings.h"
#include "imgui.h"

#include <vector>
#include <string>
#include <algorithm>

#include "CameraManager.h"
#include "EditorModule.h"
#include "ObjectFactory.h"
#include "PuzzleToken.h"
#include "Sprite.h"

ObjectPalette::ObjectPalette() : AUIScreen("OBJECT_PALETTE")
{
	this->selectedObject = NULL;
    this->wasCollapsed = false;
}

ObjectPalette::~ObjectPalette()
{

}

//handles spawn object
void ObjectPalette::spawnObject(const std::string& typeKey, bool isScreenObject, const std::string& variant)
{
    if (!LevelDataManager::getInstance()->hasActiveScene())
        return;

    //instantiate new object based on key and variant
    AGameObject* newObj = ObjectFactory::createObject(typeKey, variant);
    if (!newObj) 
        return;

    // Position object in the active viewport
    if (isScreenObject) {
        // Center UI elements in screen/canvas space using Settings.h globals
        newObj->setPos(Vector2D(uiWidth * 0.5f, uiHeight * 0.5f));
    }
    else {
        // Place world objects at current camera focus position
        Vector2D camPos(0.0f, 0.0f);
        Camera* cam = CameraManager::getInstance()->getCurrentCamera();
        if (cam != nullptr) {
            camPos = cam->getPos();
        }
        newObj->setPos(camPos);
    }
    // Register to scene
    GameObjectManager* gom = GameObjectManager::getInstance();

    if (typeKey == "puzzle_token")
    {
        // start-slot blank: its own object, named after the token
        Sprite* blank = new Sprite(newObj->getName() + "_Blank", "Token_Blank",
            newObj->getPos(), Vector2D(0.45f));
        gom->addObject(blank);

        // sit with the other blanks, just before the first existing token (behind the tokens)
        std::vector<AGameObject*>& roots = gom->getAllObjects();
        for (size_t i = 0; i < roots.size(); i++)
        {
            if (dynamic_cast<PuzzleToken*>(roots[i]))
            {
                gom->moveObject(blank, nullptr, (int)i);
                break;
            }
        }
    }
    gom->addObject(newObj);
    // 4. Update editor selection so InspectorScreen opens immediately
    this->selectedObject = newObj;
    Editor::EditorModule::getInstance()->setSelectedObject(newObj);
}


// collects .tmj file names from a folder (called by SDL_EnumerateDirectory for each file)
static SDL_EnumerationResult enumerateTilemapFile(void* userdata, const char* dirname, const char* fname)
{
    std::string n = fname;
    if (n.size() > 4 && n.compare(n.size() - 4, 4, ".tmj") == 0)
        static_cast<std::vector<std::string>*>(userdata)->push_back(n);
    return SDL_ENUM_CONTINUE;
}

void ObjectPalette::draw()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const float bottom_bar_height = kBottomBarH;
    const float hierarchy_width = kHierarchyW;
    const float palette_width = viewport->WorkSize.x - kHierarchyW;
    const float titlebar_height   = ImGui::GetFrameHeight();

    ImVec2 dock_pos;
    ImVec2 dock_size(palette_width, bottom_bar_height);

    if (wasCollapsed) {
        dock_pos = ImVec2(viewport->WorkPos.x + hierarchy_width,
            viewport->WorkPos.y + viewport->WorkSize.y - titlebar_height);
    }
    else {
        dock_pos = ImVec2(viewport->WorkPos.x + hierarchy_width,
            viewport->WorkPos.y + viewport->WorkSize.y - bottom_bar_height);
    }

    ImGui::SetNextWindowPos(dock_pos);
    ImGui::SetNextWindowSize(dock_size);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_HorizontalScrollbar;

    bool windowVisible = (ImGui::Begin("Object Palette", NULL, flags));
    wasCollapsed = ImGui::IsWindowCollapsed();

    if (windowVisible) {
        const auto& registeredTypes = ObjectFactory::getRegisteredTypes();
        const ImVec2 button_size(84.0f, 68.0f);

        float avail = ImGui::GetContentRegionAvail().x;
        int columns = std::max(1, static_cast<int>(avail / (button_size.x + 8.0f)));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.5f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.92f, 0.94f, 0.96f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.82f, 0.85f, 0.88f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.2f, 0.22f, 0.25f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));

        if (!LevelDataManager::getInstance()->hasActiveScene())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::TextUnformatted("Load or create a level to add objects.");
            ImGui::PopStyleColor();
        }
        else
        {
            int shown = 0;
            for (int i = 0; i < static_cast<int>(registeredTypes.size()); ++i) {
                const auto& item = registeredTypes[i];

                if (item.typeKey == "gem_input_token")
                    continue;
                const std::vector<std::string> missing = ObjectFactory::getMissingDependencies(item.typeKey);
                const bool blocked = !missing.empty();



                //build the "Needs: ..." tooltip once, shared by this type's button(s)
                std::string tip;
                if (blocked)
                {
                    tip = "Needs: ";
                    for (size_t k = 0; k < missing.size(); ++k)
                        tip += (k ? ", " : "") + missing[k];
                }

                if (item.variants.empty())
                {
                    ImGui::PushID(i);
                    if (shown % columns != 0) ImGui::SameLine();

                    if (blocked) ImGui::BeginDisabled();
                    if (ImGui::Button(item.displayName.c_str(), button_size))
                        spawnObject(item.typeKey, item.isScreenObject, "");
                    if (blocked) ImGui::EndDisabled();

                    if (blocked && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        ImGui::SetTooltip("%s", tip.c_str());
                        ImGui::PopStyleColor();
                    }

                    ImGui::PopID();
                    ++shown;
                }
                else
                {
                    for (const std::string& v : item.variants)
                    {
                        if (GameObjectManager::getInstance()->findObjectByName(v))
                            continue;                                   // already in the level

                        ImGui::PushID((i * 100) + (int)(&v - &item.variants[0]));
                        if (shown % columns != 0) ImGui::SameLine();

                        if (blocked) ImGui::BeginDisabled();
                        if (ImGui::Button(v.c_str(), button_size))
                            spawnObject(item.typeKey, item.isScreenObject, v);
                        if (blocked) ImGui::EndDisabled();

                        if (blocked && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                        {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                            ImGui::SetTooltip("%s", tip.c_str());
                            ImGui::PopStyleColor();
                        }

                        ImGui::PopID();
                        ++shown;
                    }
                }
            }

            // last tile in the grid: import a Tiled map
            if (shown % columns != 0) ImGui::SameLine();
            if (ImGui::Button("Tilemap", button_size))
            {
                tilemapFiles.clear();
                SDL_EnumerateDirectory("Assets/maps", enumerateTilemapFile, &tilemapFiles);
                std::sort(tilemapFiles.begin(), tilemapFiles.end());
                ImGui::OpenPopup("Load Tilemap");
            }
        }

        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(3);

        // same layout as the toolbar's Load panel (LevelFilePanel::drawLoadPopup)
        const ImVec2 ds = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(ds.x * 0.5f, ds.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(ds.x * 0.40f, 0.0f), ImGuiCond_Appearing);

        bool open = true;
        if (ImGui::BeginPopupModal("Load Tilemap", &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        {
            ImGui::TextDisabled("Tiled maps in Assets/maps");

            if (tilemapFiles.empty())
            {
                ImGui::TextUnformatted("No .tmj files found. Export one from Tiled (File > Export As > JSON).");
            }
            else
            {
                if (ImGui::BeginChild("##tilemaplist", ImVec2(0, 320.0f), ImGuiChildFlags_Borders))
                {
                    LevelDataManager* ldm = LevelDataManager::getInstance();
                    for (const std::string& f : tilemapFiles)
                    {
                        const bool isLoaded = (ldm->getTilemapPath() == "maps/" + f);
                        const std::string label = isLoaded ? f + "   (loaded)" : f;

                        if (ImGui::Selectable(label.c_str(), false, 0, ImVec2(0, 48.0f)))
                        {
                            ldm->importTilemap("maps/" + f);   // applied by applyPending()
                            ImGui::CloseCurrentPopup();
                            break;
                        }
                    }
                }
                ImGui::EndChild();   // always paired with BeginChild, even if it returned false
            }

            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

void ObjectPalette::setSelectedObject(AGameObject* selectedObject)
{
    this->selectedObject = selectedObject;
}

AGameObject* ObjectPalette::getSelectedObject()
{
    return this->selectedObject;
}

void ObjectPalette::setMousePos(Vector2D mousePos)
{
    this->mousePos = mousePos;
}