#pragma once
#include "AUIScreen.h"
#include "AGameObject.h"

class InspectorScreen : public AUIScreen
{
private:
    AGameObject* selectedObject;
    Vector2D mousePos;
    AGameObject* lastSelected = nullptr;
    char nameBuf[128] = {};
    char textBuf[512] = {};
    int fontSizeBuf = 45;
    bool pendingDelete = false;

public:
    void setMousePos(Vector2D mousePos);

protected:
    InspectorScreen();
    ~InspectorScreen();
    void DrawUI() override;
    void showComponents();

    friend class UIManager;

private:
    void showTransform(ImGuiChildFlags childFlags);
    void syncBuffers();
    void applyName();
    void showTexture();
    void showParent();
    void showTypeSpecific();
    void showObjectType();
    void showChildrenList();
    void showDelete();
    void performDelete();
};