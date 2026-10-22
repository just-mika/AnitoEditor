#include "ObjectPalette.h"
#include "GameObjectManager.h"
#include "LevelDataManager.h"
#include "Camera.h"
#include "Settings.h"
#include "imgui.h"

#include <vector>
#include <string>
#include <algorithm>
#include <variant>

#include "CameraManager.h"
#include "EditorModule.h"
#include "ObjectFactory.h"

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

ObjectPalette::ObjectPalette() : AUIScreen("OBJECT_PALETTE")
{
	this->selectedObject = NULL;
    this->wasCollapsed = false;
}

ObjectPalette::~ObjectPalette()
{

}

void ObjectPalette::spawnObject(const std::string& typeKey, bool isScreenObject, const std::string& variant)
{
    if (!LevelDataManager::getInstance()->hasActiveScene()) return;
    AGameObject* newObj = ObjectFactory::createObject(typeKey, variant);
    if (!newObj) return;

    // 2. Position object in the active viewport
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

    // 3. Register to scene
    GameObjectManager::getInstance()->addObject(newObj);

    // 4. Update editor selection so InspectorScreen opens immediately
    this->selectedObject = newObj;
    Editor::EditorModule::getInstance()->setSelectedObject(newObj);
}
void ObjectPalette::DrawUI()
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
                if (!ObjectFactory::getMissingDependencies(item.typeKey).empty())
                    continue;

                if (item.variants.empty())
                {
                    ImGui::PushID(i);
                    if (shown % columns != 0) ImGui::SameLine();
                    if (ImGui::Button(item.displayName.c_str(), button_size))
                        spawnObject(item.typeKey, item.isScreenObject, "");
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
                        if (ImGui::Button(v.c_str(), button_size))
                            spawnObject(item.typeKey, item.isScreenObject, v);
                        ImGui::PopID();
                        ++shown;
                    }
                }
            }
        }

        ImGui::PopStyleColor(5);
        ImGui::PopStyleVar(3);
    }
    ImGui::End();
}