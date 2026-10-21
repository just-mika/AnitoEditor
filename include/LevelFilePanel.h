#pragma once
#include <string>
#include <vector>

// Save / Load popups for the editor toolbar (thesis Figure 4.9).
//
//   Save button -> "Save Level" popup -> [Save as Final]  writes <levelId>.json
//                                        [Save as Draft]  -> file name popup
//   Load button -> "Load JSON" popup  -> list of this level's JSON files
//
// Owned by EditorToolbarScreen. Call draw() once per frame inside DrawUI().
class LevelFilePanel
{
public:

    // Call from the toolbar's Save / Load buttons.
    void openSave() { pending = Request::Save; }
    void openLoad() { pending = Request::Load; }

    // Must be called every frame from inside an ImGui frame (e.g. EditorToolbarScreen::DrawUI).
    void draw();

private:
    enum class Request { None, Save, SaveAs, Load };

    Request pending = Request::None;

    char draftBuf[64] = {};
    std::vector<std::string> files;   // files shown in the Load popup

    std::string statusMsg;
    double statusUntil = 0.0;

    void refreshFiles();
    std::string suggestName() const;
    static std::string sanitizeStem(const std::string& raw);
    static std::string stemOf(const std::string& fileName);
    bool hasFile(const std::string& fileName) const;

    void drawSavePopup();
    void drawSaveAsPopup();
    void drawLoadPopup();
    void drawStatusToast();

    bool saveAs(const std::string& stem);   // stem without ".json"
    void setStatus(const std::string& msg);
};
