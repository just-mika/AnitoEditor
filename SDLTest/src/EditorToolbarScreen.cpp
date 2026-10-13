#include "EditorToolbarScreen.h"
#include "EditorModule.h"
#include "SceneManager.h"
#include "UIManager.h"
#include "Settings.h"

EditorToolbarScreen::EditorToolbarScreen() : AUIScreen("EDITOR_TOOLBAR_SCREEN")
{
}

EditorToolbarScreen::~EditorToolbarScreen()
{
}

// Draws a ▶ centered on the last submitted button.
static void drawPlayIcon()
{
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 c((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
    float s = (max.y - min.y) * 0.25f;

    ImGui::GetWindowDrawList()->AddTriangleFilled(
            ImVec2(c.x - s * 0.8f, c.y - s),
            ImVec2(c.x - s * 0.8f, c.y + s),
            ImVec2(c.x + s, c.y),
            ImGui::GetColorU32(ImGuiCol_Text));
}

// Draws a ■ centered on the last submitted button.
static void drawStopIcon()
{
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 c((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
    float s = (max.y - min.y) * 0.22f;

    ImGui::GetWindowDrawList()->AddRectFilled(
            ImVec2(c.x - s, c.y - s),
            ImVec2(c.x + s, c.y + s),
            ImGui::GetColorU32(ImGuiCol_Text));
}

void EditorToolbarScreen::DrawUI()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    Editor::EditorModule* editor = Editor::EditorModule::getInstance();

    // ---------------------------------------------------------------
    // PLAY MODE: the toolbar disappears entirely. Only a small stop
    // button remains, floating at the top-center of the screen.
    // ---------------------------------------------------------------
    if (editor->isPlaying())
    {
        const float top_margin = 24.0f;

        ImGui::SetNextWindowPos(
                ImVec2(viewport->WorkPos.x + viewport->WorkSize.x * 0.5f,
                       viewport->WorkPos.y + top_margin),
                ImGuiCond_Always,
                ImVec2(0.5f, 0.0f));   // pivot: top-center of the window

        ImGuiWindowFlags stopFlags = ImGuiWindowFlags_NoMove |
                                     ImGuiWindowFlags_NoResize |
                                     ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoTitleBar |
                                     ImGuiWindowFlags_NoScrollbar |
                                     ImGuiWindowFlags_NoBackground |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_AlwaysAutoResize;

        if (ImGui::Begin("EditorStopButton", NULL, stopFlags))
        {
            if (ImGui::Button("##stop", ImVec2(56.0f, 36.0f)))
                editor->togglePlayMode();
            drawStopIcon();
        }
        ImGui::End();
        return;
    }

    // ---------------------------------------------------------------
    // EDIT MODE: full-width toolbar pinned to the top of the screen.
    // Collapsing shrinks it down to just the arrow button's row.
    // ---------------------------------------------------------------
    const float toolbar_height = kToolbarH;
    const float button_width   = 60.0f;

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, toolbar_height));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse |
                             ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoScrollWithMouse;

    if (ImGui::Begin("EditorToolbar", NULL, flags))
    {
        UIManager* ui = UIManager::getInstance();
        const ImGuiStyle& style = ImGui::GetStyle();

        ImGui::SetCursorPosY((toolbar_height - ImGui::GetFrameHeight()) * 0.5f);

        // 2. Scene switch:  <  Scene N  >
        SceneManager* sceneManager = SceneManager::getInstance();
        const int sceneCount = sceneManager->getRegisteredSceneAmount();
        const int sceneIndex = (int)sceneManager->getLoadedSceneTag();
        int newIndex = sceneIndex;

        if (ImGui::ArrowButton("##prevScene", ImGuiDir_Left)) newIndex--;
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Scene %d", sceneIndex);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##nextScene", ImGuiDir_Right)) newIndex++;

        newIndex = SDL_clamp(newIndex,
            (int)SceneTag::PLATFORMER_LEVEL_1_SCENE,
            (int)SceneTag::PUZZLE_LEVEL_1_SCENE);

        if (newIndex != sceneIndex)
        {
            editor->setSelectedObject(nullptr);
            sceneManager->loadScene((SceneTag)newIndex);
        }

        // 3. Play button, centered horizontally in the bar
        ImGui::SameLine((ImGui::GetWindowWidth() - button_width) * 0.5f);
        if (ImGui::Button("##play", ImVec2(button_width, 0)))
            editor->togglePlayMode();
        drawPlayIcon();

        // 4. Save / Load, right-aligned (placeholders for now)
        ImGui::SameLine(ImGui::GetWindowWidth() - style.WindowPadding.x
                        - button_width * 2 - style.ItemSpacing.x);
        if (ImGui::Button("Save", ImVec2(button_width, 0)))
        {
            // TODO: save level
        }
        ImGui::SameLine();
        if (ImGui::Button("Load", ImVec2(button_width, 0)))
        {
            // TODO: load level
        }
    }
    ImGui::End();
}