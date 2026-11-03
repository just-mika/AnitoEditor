#pragma once
#include <string>
#include <vector>

// "Choose Texture" popup: browse the Assets folder and apply an image to the selected sprite.
// Same layout as LevelFilePanel's Load popup. Owned by InspectorScreen.
class AssetBrowserPanel
{
public:
    void open() { bPendingOpen = true; }   //opened on the next draw()
    void draw();

private:
    bool bPendingOpen = false;
    std::string currentDir;               //relative to Assets/: "" = Assets itself, otherwise ends with '/'
    std::vector<std::string> folders;     //subfolders of currentDir
    std::vector<std::string> images;      //image files in currentDir
    char searchBuf[64] = {};
    bool bInLoaded = false;               //showing the [Loaded] list instead of a folder
    std::vector<std::string> loaded;      //texture keys that are already loaded
    bool bForAnimation = false;

    void refresh();
    void applyTexture(const std::string& key);
};