#include "LevelFilePanel.h"
#include "LevelDataManager.h"
#include "imgui.h"
#include <algorithm>
#include <cstring>

namespace
{
    constexpr float kButtonH = 64.0f;   // big touch targets for the mobile device
    constexpr const char* kSaveId  = "Save Level";
    constexpr const char* kDraftId = "Save Draft";
    constexpr const char* kLoadId  = "Load JSON";

    void centerNextPopup(float widthFraction)
    {
        const ImVec2 ds = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(ds.x * 0.5f, ds.y * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(ds.x * widthFraction, 0.0f), ImGuiCond_Appearing);
    }
}

void LevelFilePanel::draw()
{
    // Popups must be opened from the same ID stack that later calls BeginPopupModal,
    // so the buttons only set a request and the actual OpenPopup happens here.
    switch (pending)
    {
    case Request::Save:
        ImGui::OpenPopup(kSaveId);
        break;
    case Request::SaveAs:
        refreshFiles();
        strncpy_s(draftBuf, suggestName().c_str(), sizeof(draftBuf) - 1);
        draftBuf[sizeof(draftBuf) - 1] = '\0';
        ImGui::OpenPopup(kDraftId);
        break;
    case Request::Load:
        refreshFiles();
        ImGui::OpenPopup(kLoadId);
        break;
    default:
        break;
    }
    pending = Request::None;

    drawSavePopup();
    drawSaveAsPopup();
    drawLoadPopup();
    drawStatusToast();
}

// ------------------------------------------------------------------- popups


void LevelFilePanel::drawSavePopup()
{
    centerNextPopup(0.35f);
    bool open = true;
    if (!ImGui::BeginPopupModal(kSaveId, &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        return;

    const std::string& cur = LevelDataManager::getInstance()->getCurrentFile();
    ImGui::TextDisabled("Open file: %s", cur.empty() ? "(none)" : cur.c_str());
    ImGui::Spacing();

    ImGui::BeginDisabled(cur.empty());
    if (ImGui::Button("Save", ImVec2(-FLT_MIN, kButtonH)))
    {
        saveAs(stemOf(cur));            // overwrite the open file
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();

    if (ImGui::Button("Save As...", ImVec2(-FLT_MIN, kButtonH)))
    {
        pending = Request::SaveAs;      // opens next frame
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

void LevelFilePanel::drawSaveAsPopup()
{
    centerNextPopup(0.40f);
    bool open = true;
    if (!ImGui::BeginPopupModal(kDraftId, &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        return;

    ImGui::TextUnformatted("File name");
    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::InputText("##draftname", draftBuf, sizeof(draftBuf));

    const std::string stem = sanitizeStem(draftBuf);
    const bool valid = !stem.empty();
    const bool exists = valid && hasFile(stem + ".json");

    if (!valid)
        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Use letters, numbers, _ or -");
    else if (exists)
        ImGui::TextColored(ImVec4(1, 0.8f, 0.3f, 1), "%s.json exists and will be overwritten", stem.c_str());
    else
        ImGui::TextDisabled("Will save as %s.json", stem.c_str());

    ImGui::Spacing();
    ImGui::BeginDisabled(!valid);
    if (ImGui::Button("Save", ImVec2(-FLT_MIN, kButtonH)))
    {
        saveAs(stem);
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();

    ImGui::EndPopup();
}

void LevelFilePanel::drawLoadPopup()
{
    centerNextPopup(0.40f);
    bool open = true;
    if (!ImGui::BeginPopupModal(kLoadId, &open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings))
        return;

    ImGui::TextDisabled("Level files");

    if (files.empty())
    {
        ImGui::TextUnformatted("No saved files for this level yet.");
    }
    else if (ImGui::BeginChild("##filelist", ImVec2(0, 320.0f), ImGuiChildFlags_Borders))
    {
        for (const std::string& f : files)
        {
            // Canonical file gets a tag so it is easy to tell apart from drafts.
			const bool isOpen = (f == LevelDataManager::getInstance()->getCurrentFile());
            const std::string label = isOpen ? f + "   (open)" : f;

            if (ImGui::Selectable(label.c_str(), false, 0, ImVec2(0, 48.0f)))
            {
                LevelDataManager::getInstance()->requestLoad(f);   // applied by applyPending()
                setStatus("Loading " + f);
                ImGui::CloseCurrentPopup();
                break;   // files list is unchanged, but we just closed the popup
            }
        }
    }
    ImGui::EndChild();

    ImGui::EndPopup();
}

void LevelFilePanel::drawStatusToast()
{
    if (statusMsg.empty() || ImGui::GetTime() > statusUntil)
        return;

    const ImVec2 ds = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(ds.x * 0.5f, 80.0f), ImGuiCond_Always, ImVec2(0.5f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.85f);
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                                   ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings |
                                   ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (ImGui::Begin("##levelfile_toast", nullptr, flags))
        ImGui::TextUnformatted(statusMsg.c_str());
    ImGui::End();
}

// ------------------------------------------------------------------ helpers

bool LevelFilePanel::saveAs(const std::string& stem)
{
    const std::string fileName = stem + ".json";
    const bool ok = LevelDataManager::getInstance()->saveLevel(fileName);
    setStatus(ok ? "Saved " + fileName : "Save failed: " + fileName);
    return ok;
}

void LevelFilePanel::setStatus(const std::string& msg)
{
    statusMsg = msg;
    statusUntil = ImGui::GetTime() + 2.5;
}


void LevelFilePanel::refreshFiles()
{
    files = LevelDataManager::getInstance()->listLevelFiles();   // no prefix filter
}

bool LevelFilePanel::hasFile(const std::string& fileName) const
{
    return std::find(files.begin(), files.end(), fileName) != files.end();
}


std::string LevelFilePanel::suggestName() const
{
    const std::string& cur = LevelDataManager::getInstance()->getCurrentFile();
    const std::string base = cur.empty() ? "level" : stemOf(cur) + "_copy";
    std::string name = base;
    for (int n = 2; hasFile(name + ".json"); ++n)
        name = base + std::to_string(n);
    return name;
}

std::string LevelFilePanel::stemOf(const std::string& fileName)
{
    const size_t slash = fileName.find_last_of("/\\");
    std::string base = (slash == std::string::npos) ? fileName : fileName.substr(slash + 1);
    const size_t dot = base.rfind(".json");
    if (dot != std::string::npos && dot + 5 == base.size())
        base.erase(dot);
    return base;
}

std::string LevelFilePanel::sanitizeStem(const std::string& raw)
{
    // Strip a typed ".json", then keep only [A-Za-z0-9_-]. No paths, no spaces.
    std::string s = stemOf(raw);
    std::string out;
    for (char c : s)
    {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-')
            out.push_back(c);
    }
    return out;
}
