#include "EditorToolbarScreen.h"
#include "EditorModule.h"
#include "LevelDataManager.h"
#include "SceneManager.h"
#include "UIManager.h"
#include "Settings.h"
#include "TiledImporter.h"
#include "TileMap.h"

EditorToolbarScreen::EditorToolbarScreen() : AUIScreen("EDITOR_TOOLBAR_SCREEN")
{
}

EditorToolbarScreen::~EditorToolbarScreen()
{
}

// Draws play mode button
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

// Draws stop play mode button
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

void EditorToolbarScreen::draw()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    Editor::EditorModule* editor = Editor::EditorModule::getInstance();

	//Play mode: only show Stop button to go back to editor
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

    //Edit mode: draw the toolbar
    const float toolbar_height = kToolbarH;
    const float button_width   = 60.0f;

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, toolbar_height));

    //flags keep the toolbar docked
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

        //Show filename on top if a file is loaded
        LevelDataManager* ldm = LevelDataManager::getInstance();
        ImGui::SameLine();
        ImGui::AlignTextToFramePadding();

        if (ldm->hasCurrentFile())
			ImGui::TextUnformatted(ldm->getCurrentFile().c_str());
        else
            ImGui::TextUnformatted("(no file)");

        // draw play button
        ImGui::SameLine((ImGui::GetWindowWidth() - button_width) * 0.5f);
        ImGui::BeginDisabled(!ldm->hasActiveScene());
        if (ImGui::Button("##play", ImVec2(button_width, 0)))
            editor->togglePlayMode();
        drawPlayIcon();
        ImGui::EndDisabled();

        ImGui::SameLine(ImGui::GetWindowWidth() - style.WindowPadding.x
            - button_width * 3 - style.ItemSpacing.x * 2);
        
        //draw "New" button (for new file)
        if (ImGui::Button("New", ImVec2(button_width, 0)))
            ImGui::OpenPopup("NewLevelPopup");
        ImGui::SameLine();

        //draw "Save" button (for save current level to a JSON)
        ImGui::BeginDisabled(!ldm->hasActiveScene());
        if (ImGui::Button("Save", ImVec2(button_width, 0)))
            filePanel.openSave();
        ImGui::EndDisabled();

        //draw "Load" button (for loading JSON file to level)
        ImGui::SameLine();
        if (ImGui::Button("Load", ImVec2(button_width, 0)))
            filePanel.openLoad();

        //pop-up to select new level template: puzzle or platformer
        if (ImGui::BeginPopup("NewLevelPopup"))
        {
            if (ImGui::Selectable("Puzzle level"))
            {
                editor->setSelectedObject(nullptr);
                ldm->newLevel("puzzle");
            }

            if (ImGui::Selectable("Platformer level"))
            {
                editor->setSelectedObject(nullptr);
                ldm->newLevel("platformer");
            }

            ImGui::EndPopup();
        }

        
    }
    ImGui::End();

    filePanel.draw(); //draw file selection panel (only visible if Save or Load is selected)
}