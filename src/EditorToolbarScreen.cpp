#include "EditorToolbarScreen.h"
#include "EditorModule.h"
#include "LevelDataManager.h"
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
        const bool noScene = (sceneIndex == (int)SceneTag::NONE);
        int newIndex = sceneIndex;

        ImGui::BeginDisabled(noScene);
        if (ImGui::ArrowButton("##prevScene", ImGuiDir_Left)) newIndex--;
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        if (noScene) ImGui::TextUnformatted("(no level)");
        else         ImGui::Text("Scene %d", sceneIndex);
        ImGui::SameLine();
        if (ImGui::ArrowButton("##nextScene", ImGuiDir_Right)) newIndex++;
        ImGui::EndDisabled();

        if (!noScene)
        {
            newIndex = SDL_clamp(newIndex,
                (int)SceneTag::PLATFORMER_LEVEL_1_SCENE,
                (int)SceneTag::PLATFORMER_LOADED_SCENE);   // last tag in your enum

            if (newIndex != sceneIndex)
            {
                editor->setSelectedObject(nullptr);
                sceneManager->loadScene((SceneTag)newIndex);
            }
        }

        // ---- file switch (add here) ----
        LevelDataManager* ldm = LevelDataManager::getInstance();
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted(ldm->hasCurrentFile() ? ldm->getCurrentFile().c_str() : "(no file)");
        // --------------------------------

        // 3. Play button, centered horizontally in the bar
        ImGui::SameLine((ImGui::GetWindowWidth() - button_width) * 0.5f);
        if (ImGui::Button("##play", ImVec2(button_width, 0)))
            editor->togglePlayMode();
        drawPlayIcon();

        // 4. Save / Load, right-aligned (placeholders for now)
        ImGui::SameLine(ImGui::GetWindowWidth() - style.WindowPadding.x
            - button_width * 3 - style.ItemSpacing.x * 2);
        
        if (ImGui::Button("New", ImVec2(button_width, 0)))
            ImGui::OpenPopup("NewLevelPopup");
        ImGui::SameLine();

        ImGui::BeginDisabled(!ldm->hasActiveScene());
        if (ImGui::Button("Save", ImVec2(button_width, 0)))
            filePanel.openSave();
        ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button("Load", ImVec2(button_width, 0)))
            filePanel.openLoad();

        if (ImGui::BeginPopup("NewLevelPopup"))
        {
            if (ImGui::Selectable("Puzzle level"))
            {
                editor->setSelectedObject(nullptr);
                ldm->newLevel("puzzle");
            }

            // enable once PlatformerLoadedScene exists
            ImGui::BeginDisabled(!ldm->canCreate("platformer"));
            if (ImGui::Selectable("Platformer level"))
            {
                editor->setSelectedObject(nullptr);
                ldm->newLevel("platformer");
            }
            ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        
    }
    ImGui::End();

    filePanel.draw();
}