#pragma once
#include "AUIScreen.h"
#include "AGameObject.h"
#include "AssetBrowserPanel.h"
#include "Text.h"

class InspectorScreen : public AUIScreen
{
private:
    AGameObject* selectedObject;
    Vector2D mousePos;
    AGameObject* lastSelected = nullptr;
    char nameBuf[128] = {};
    char textBuf[16384] = {};
    int fontSizeBuf = 45;
    bool pendingDelete = false;
    AssetBrowserPanel assetBrowser;
    Text* pEditingText = nullptr;

    char gemTitleBuf[128] = {};
    char gemTextBuf[1024] = {};

public:
    void setMousePos(Vector2D mousePos);

protected:
    InspectorScreen();
    ~InspectorScreen();
    void draw() override;
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
