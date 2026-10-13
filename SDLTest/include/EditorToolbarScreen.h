#pragma once
#include "AUIScreen.h"

class EditorToolbarScreen : public AUIScreen
{
private:

protected:
    EditorToolbarScreen();
    ~EditorToolbarScreen();
    void DrawUI() override;

    friend class UIManager;
};