#pragma once
#include "AUIScreen.h"
#include "LevelFilePanel.h"

class EditorToolbarScreen : public AUIScreen
{
private:
    LevelFilePanel filePanel;
protected:
    EditorToolbarScreen();
    ~EditorToolbarScreen();
    void draw() override;

    friend class UIManager;
};