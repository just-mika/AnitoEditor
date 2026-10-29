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

//draw one row of object
void HierarchyScreen::drawObjectNode(AGameObject *obj) {
    ImGui::PushID(obj);
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGuiTreeNodeFlags flags;
    bool hasChildren = !obj->getChildren().empty();

    //expand parent when auto scrolling to selection
    if (scrollToSelection && hasChildren && isOnSelectedPath(obj))
        ImGui::SetNextItemOpen(true, ImGuiCond_Always);

    //collapses to reveal children if arrow is clicked
    if (hasChildren)
        flags = ImGuiTreeNodeFlags_OpenOnArrow;
    //no children = leaf node and prevents TreePush() when opened
    else
        flags = ImGuiTreeNodeFlags_Leaf |
        ImGuiTreeNodeFlags_NoTreePushOnOpen;

    // allows select object when node is clicked
    flags |= ImGuiTreeNodeFlags_SpanFullWidth;

    //set selected if an object is selected
    if(obj == selectedObject){
        flags |= ImGuiTreeNodeFlags_Selected;
    }

    std::string name = obj->getName();

    //auto scrolls to selected object
    if (scrollToSelection && obj == selectedObject)
    {
        ImGui::SetScrollHereY(0.5f);
        scrollToSelection = false;
    }

    //create new node
    bool nodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);

    //set selected if node is clicked
    if (ImGui::IsItemClicked()) {
        Editor::EditorModule::getInstance()->setSelectedObject(obj);
        selectedObject = obj;
    }

    // Allow dragging (from source)
    if (ImGui::BeginDragDropSource())
    {
        //creates payload to encapsulate selected object in a payload labeled HIERARCHY_OBJECT
        ImGui::SetDragDropPayload("HIERARCHY_OBJECT", &obj, sizeof(AGameObject*));
        ImGui::TextUnformatted(name.c_str());   // preview shown next to the cursor
        ImGui::EndDragDropSource();
    }
    //set drag and drop target
    if (ImGui::BeginDragDropTarget())
    {
        this->handleDropOnNode(obj, hasChildren && nodeOpen);
        ImGui::EndDragDropTarget();
    }

    bool active = obj->getEnabled();

    ImGui::TableNextColumn();
    //enable checkbox if game object is active
    if (ImGui::Checkbox("##enabled", &active))
        obj->setEnabled(active);

    //recursively draw object node for children
    if(hasChildren && nodeOpen) {
        for (AGameObject *child: obj->getChildren())
            drawObjectNode(child);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

// handles node dropping (from dragging) and reparenting
void HierarchyScreen::handleDropOnNode(AGameObject* target, bool targetIsOpenParent)
{
    //creates payload to allow Hierarchy Screen to accept dragged object (labeled HIERARCHY_OBJECT)
    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
            "HIERARCHY_OBJECT",
            ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect);

    //no payload/payload size doesnt fit gameobj size
    if (!payload || payload->DataSize != sizeof(AGameObject*)) 
        return;

    //extracts the gameobject (pointer) from the payload
    AGameObject* dragged = *static_cast<AGameObject**>(payload->Data);
    GameObjectManager* mgr = GameObjectManager::getInstance();

    //calculates hover position in respect to a node
    ImVec2 rMin = ImGui::GetItemRectMin();
    ImVec2 rMax = ImGui::GetItemRectMax();
    float t = (ImGui::GetMousePos().y - rMin.y) / (rMax.y - rMin.y);

    AGameObject* newParent = nullptr;
    int index = 0;
    //drop position depends the dragged obj's new position in the hierarchy 
    // Mouse cursor is at the top of the row or close
	if (t < 0.25f) { 
        //Top = dragged object is now a sibling of the target object
        newParent = target->getParent();
        index = mgr->getSiblingIndex(target);
    }
    //mouse cursor is at the middle of the row or close
    else if (t > 0.75f) { 
        //Center = dragged object will either be a sibling or the first child of the target obj
        if (targetIsOpenParent) {
            newParent = target;
            index = 0;
        }
        else {
            newParent = target->getParent();
            index = mgr->getSiblingIndex(target) + 1;
        }
    }
    //Mouse cursor is at the bottom of the row or close
    else { 
    	//Bottom = the dragged obj will be set as target node's new child
        newParent = target;
        index = (int)target->getChildren().size();
    }

    //check if the dragged object 
    if (!mgr->canMoveObject(dragged, newParent))
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
        return;
    }

    // Drop indicator
    ImDrawList* dl = ImGui::GetWindowDrawList();
    const ImU32 col = IM_COL32(0, 230, 204, 255);

    //add line depending on drag position
    if (t < 0.25f)
        dl->AddLine(ImVec2(rMin.x, rMin.y), ImVec2(rMax.x, rMin.y), col, 2.0f);
    else if (t > 0.75f)
        dl->AddLine(ImVec2(rMin.x, rMax.y), ImVec2(rMax.x, rMax.y), col, 2.0f);
    else
    	dl->AddRect(rMin, rMax, col, 0.0f, 0, 2.0f);

    if (payload->IsDelivery())   // mouse released over this row
    {
        this->pendingMove.obj       = dragged;
        this->pendingMove.newParent = newParent;
        this->pendingMove.index     = index;
        this->hasPendingMove        = true;
    }
}

// Checks if current object is an ancestor of selected object
bool HierarchyScreen::isOnSelectedPath(AGameObject* obj)
{
    for (AGameObject* p = selectedObject; p != nullptr; p = p->getParent())
    {
        if (p == obj)
            return true;
    }
    return false;
}

//Draw HierarchyScreen
void HierarchyScreen::draw()
{
    this->selectedObject = Editor::EditorModule::getInstance()->getSelectedObject();
    if (selectedObject != lastSelectedObject)
    {
        lastSelectedObject = selectedObject;
        scrollToSelection = (selectedObject != nullptr);
    }

    ImGuiViewport* viewport = ImGui::GetMainViewport();

    const float bottom_bar_height = kBottomBarH;
    const float hierarchy_width = kHierarchyW; 
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

            //draw all gameobjects
            for(AGameObject* root: GameObjectManager::getInstance()->getAllObjects()){
                if(root->getParent() != nullptr) 
                	continue;
                this->drawObjectNode(root);
            }
            ImGui::EndTable();
        }

        //reflect dragging to the GameObjectManager
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