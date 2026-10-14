#include "HierarchyScreen.h"
#include "SceneManager.h"
#include "GameObjectManager.h"
#include "CameraManager.h"
#include "EditorModule.h"
#include <cmath>
#include "Settings.h"

void HierarchyScreen::setMousePos(Vector2D mousePos)
{
    this->mousePos = mousePos;
}

HierarchyScreen::HierarchyScreen() : AUIScreen("HIERARCHY_SCREEN")
{
    this->selectedObject = NULL;
}

HierarchyScreen::~HierarchyScreen()
{

}

void HierarchyScreen::drawObjectNode(AGameObject *obj) {
    ImGui::PushID(obj);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGuiTreeNodeFlags flags;
    bool hasChildren = !obj->getChildren().empty();

    if (scrollToSelection && hasChildren && isOnSelectedPath(obj))
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);

    if(hasChildren)
        flags = ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanFullWidth;
    else
        flags  = ImGuiTreeNodeFlags_Leaf |
                 ImGuiTreeNodeFlags_NoTreePushOnOpen |
                 ImGuiTreeNodeFlags_SpanFullWidth;
    if(obj == selectedObject){
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    std::string name = obj->getName();

    if (scrollToSelection && obj == selectedObject)
    {
        ImGui::SetScrollHereY(0.5f);
        scrollToSelection = false;
    }

    bool nodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    if (ImGui::IsItemClicked()) {
        Editor::EditorModule::getInstance()->setSelectedObject(obj);
        selectedObject = obj;
    }

    // Drag source + drop target must come right after TreeNodeEx (they bind to the last item).
    if (ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("HIERARCHY_OBJECT", &obj, sizeof(AGameObject*));
        ImGui::TextUnformatted(name.c_str());   // preview shown next to the cursor
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget())
    {
        this->handleDropOnNode(obj, hasChildren && nodeOpen);
        ImGui::EndDragDropTarget();
    }

    bool active = obj->getEnabled();

    ImGui::TableNextColumn();

    if (ImGui::Checkbox("##enabled", &active))
        obj->setEnabled(active);

    if(hasChildren && nodeOpen) {
        for (AGameObject *child: obj->getChildren())
            drawObjectNode(child);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

// Unity-style drop zones on a row:
//   top 25%    -> insert BEFORE the row (same parent, drawn behind it)
//   middle 50% -> make the dragged object a CHILD of the row (appended = drawn on top)
//   bottom 25% -> insert AFTER the row (same parent, drawn in front of it);
//                 if the row is an expanded parent, this becomes "first child" instead,
//                 because that's where the line visually sits.
void HierarchyScreen::handleDropOnNode(AGameObject* target, bool targetIsOpenParent)
{
    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
            "HIERARCHY_OBJECT",
            ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);
    if (!payload || payload->DataSize != sizeof(AGameObject*)) return;

    AGameObject* dragged = *static_cast<AGameObject**>(payload->Data);
    GameObjectManager* mgr = GameObjectManager::getInstance();

    ImVec2 rMin = ImGui::GetItemRectMin();
    ImVec2 rMax = ImGui::GetItemRectMax();
    float t = (ImGui::GetMousePos().y - rMin.y) / (rMax.y - rMin.y);

    enum class Zone { Before, Into, After };
    Zone zone = (t < 0.25f) ? Zone::Before : (t > 0.75f) ? Zone::After : Zone::Into;

    AGameObject* newParent = nullptr;
    int index = 0;
    switch (zone)
    {
        case Zone::Before:
            newParent = target->getParent();
            index = mgr->getSiblingIndex(target);
            break;
        case Zone::After:
            if (targetIsOpenParent) { newParent = target; index = 0; }
            else { newParent = target->getParent(); index = mgr->getSiblingIndex(target) + 1; }
            break;
        case Zone::Into:
            newParent = target;
            index = (int)target->getChildren().size();
            break;
    }

    if (!mgr->canMoveObject(dragged, newParent))
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
        return;
    }

    // Drop indicator
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImU32 col = IM_COL32(0, 230, 204, 255);
    if (zone == Zone::Before)      dl->AddLine(ImVec2(rMin.x, rMin.y), ImVec2(rMax.x, rMin.y), col, 2.0f);
    else if (zone == Zone::After)  dl->AddLine(ImVec2(rMin.x, rMax.y), ImVec2(rMax.x, rMax.y), col, 2.0f);
    else                           dl->AddRect(rMin, rMax, col, 0.0f, 0, 2.0f);

    if (payload->IsDelivery())   // mouse released over this row
    {
        this->pendingMove.obj       = dragged;
        this->pendingMove.newParent = newParent;
        this->pendingMove.index     = index;
        this->hasPendingMove        = true;
    }
}

bool HierarchyScreen::isOnSelectedPath(AGameObject* obj)
{
    for (AGameObject* p = selectedObject; p != nullptr; p = p->getParent())
    {
        if (p == obj) return true;
    }
    return false;
}

void HierarchyScreen::DrawUI()
{
    this->selectedObject = Editor::EditorModule::getInstance()->getSelectedObject();
    if (selectedObject != lastSelectedObject)
    {
        lastSelectedObject = selectedObject;
        scrollToSelection = (selectedObject != nullptr);
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const float bottom_bar_height = kBottomBarH;   // 230.0f
    const float hierarchy_width = kHierarchyW;   // 600.0f
    const float titlebar_height = ImGui::GetFrameHeight();

    ImVec2 dock_pos;
    ImVec2 dock_size(hierarchy_width, bottom_bar_height);

    if (wasCollapsed)
        dock_pos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - titlebar_height);
    else
        dock_pos = ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - bottom_bar_height);

    ImGui::SetNextWindowPos(dock_pos);
    ImGui::SetNextWindowSize(dock_size);

    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.0f, 0.9f, 0.8f, 0.35f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.0f, 0.9f, 0.8f, 0.5f));
    ImGui::PushStyleColor(ImGuiCol_NavHighlight, ImVec4(0.0f, 0.9f, 0.8f, 1.0f));

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
                                    ImGuiWindowFlags_NoResize;

    bool windowVisible = ImGui::Begin("Hierarchy", NULL, window_flags);
    wasCollapsed = ImGui::IsWindowCollapsed();

    if (windowVisible) {
        static ImGuiTableFlags table_flags = ImGuiTableFlags_RowBg |
                                             ImGuiTableFlags_ScrollY |
                                             ImGuiTableFlags_SizingFixedFit;

        if (ImGui::BeginTable("HierarchyTable", 2, table_flags)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("Active", ImGuiTableColumnFlags_WidthFixed, 22.0f);

            for(AGameObject* root: GameObjectManager::getInstance()->getAllObjects()){
                if(root->getParent() != nullptr) continue;
                this->drawObjectNode(root);
            }
            ImGui::EndTable();
        }

        // Apply after the tree has been fully drawn so we don't mutate lists mid-iteration.
        if (this->hasPendingMove)
        {
            GameObjectManager::getInstance()->moveObject(
                    this->pendingMove.obj, this->pendingMove.newParent, this->pendingMove.index);
            this->hasPendingMove = false;
        }
    }
    ImGui::End();
    ImGui::PopStyleColor(3);
}