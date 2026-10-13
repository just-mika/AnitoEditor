#include "FontManager.h"
#include <fstream>
#include <cstdio>
#include "ManifestLoader.h"

TTF_Font* FontManager::getFont(const std::string fontKey, int fontSize)
{
    std::string fullKey = fontKey + "_" + std::to_string(fontSize);
    auto it = mapFonts.find(fullKey);
    if (it != mapFonts.end()) return it->second;

    SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "[ERROR] Font [%s] not found.",
            fullKey.c_str()
            );
    return nullptr;
}

void FontManager::loadFont(const std::string fileName, const std::string fontKey, int fontSize)
{
    const std::string assetPath = "Assets/Fonts/" + fileName;
    std::string fullKey = fontKey + "_" + std::to_string(fontSize);

    if (mapFonts.contains(fullKey)) return;

    TTF_Font* directFont = TTF_OpenFont(assetPath.c_str(), fontSize);
    if (!directFont)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "[ERROR] Failed to load font [%s] : %s",
            fullKey.c_str(), SDL_GetError());
        return;
    }
    mapFonts[fullKey] = directFont;
    SDL_Log("[DEBUG] %s has been loaded", fullKey.c_str());
    return;
}

void FontManager::unloadFontFamily(const std::string& fontKey)
{
    for (auto it = mapFonts.begin(); it != mapFonts.end(); ) {
        if (it->first.starts_with(fontKey + "_")) {
            TTF_CloseFont(it->second);
            it = mapFonts.erase(it);
        }
        else {
            ++it;
        }
    }
}

void FontManager::unloadAllFonts()
{
    if (mapFonts.empty())
    {
        std::cout << "[FontManager] No fonts to unload.\n";
        return;
    }
    for (auto& pair : mapFonts)
    {
        if (pair.second)
        {
            std::cout << "[DEBUG] " << pair.first << " has been removed" << std::endl;

            TTF_CloseFont(pair.second);
        }
    }

    mapFonts.clear();
    std::cout << "[FontManager] All fonts unloaded.\n";

}




/* * * * * * * * * * * * * * * * * * * * *
 *       SINGLETON-RELATED CONTENT       *
 * * * * * * * * * * * * * * * * * * * * */
FontManager* FontManager::P_SHARED_INSTANCE = NULL;

void FontManager::initialize()
{
    P_SHARED_INSTANCE = new FontManager();
    //Initialize font loading
    if (TTF_Init() == false)
    {
        SDL_Log("SDL_ttf could not initialize! SDL_ttf error: %s\n", SDL_GetError());
    }
}

void FontManager::destroy()
{
    delete P_SHARED_INSTANCE;
}

FontManager* FontManager::getInstance()
{
    return P_SHARED_INSTANCE;
}
/* * * * * * * * * * * * * * * * * * * * */
