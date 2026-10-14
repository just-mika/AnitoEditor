#pragma once
#include "AUIScreen.h"
#include "AGameObject.h"

class HierarchyScreen : public AUIScreen
{
private:
    AGameObject* selectedObject;
    Vector2D mousePos;
    bool wasCollapsed = false;
    AGameObject* lastSelectedObject = nullptr;
    bool scrollToSelection = false;

    // Drag & drop reorder/reparent. The move is queued while the tree is being drawn and
    // applied after the table ends, so we never mutate the lists we're iterating.
    struct PendingMove
    {
        AGameObject* obj       = nullptr;
        AGameObject* newParent = nullptr;   // nullptr = root
        int          index     = 0;
    };
    PendingMove pendingMove;
    bool hasPendingMove = false;

    // Layout configuration
    ImVec2 dockPos  = ImVec2(0.0f, 0.0f);
    ImVec2 dockSize = ImVec2(0.0f, 0.0f);

public:
    void setMousePos(Vector2D mousePos);

protected:
    HierarchyScreen();
    ~HierarchyScreen();
    void DrawUI() override;

    friend class UIManager;

private:
    void drawObjectNode(AGameObject* obj);
    bool isOnSelectedPath(AGameObject* obj);
    void handleDropOnNode(AGameObject* target, bool targetIsOpenParent);
};