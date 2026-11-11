#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>
#include <vector>
#include <string>

class TextureManager
{
private:
    SDL_Renderer* pRenderer = NULL;

    std::unordered_map<std::string, std::vector<SDL_Texture*>> mapTexture;
    std::vector<SDL_Texture*> vecTexture;
    
    std::unordered_map<std::string, std::string> mapKeyToPath;   //key -> the file (or folder) it was loaded from
    std::unordered_map<std::string, std::string> mapPathToKey;   //lowercase path -> key
    std::unordered_map<std::string, float> mapTexScale;   //path -> how much the file was shrunk (from texture_scales.txt)
	
	static std::string lowerPath(const std::string& path);

public:
    void load(std::string assetPath, std::string strName);
    void loadFromFolder(std::string strPath, std::string strName);
    void loadFromText(std::string strName, std::string fontType, int nFontSize, std::string textureText, SDL_Color textColor);

    SDL_Texture* get(const std::string& strName);

    void unload(std::string strName);
    std::vector<SDL_Texture*> getTexture(std::string strName, int nStart = -1, int nEnd = -1);
    std::vector<std::string> getTextureNames();

    std::string getKey(SDL_Texture* pTexture);
    std::string getPathOfKey(const std::string& key) const;
    std::string getKeyOfPath(const std::string& path) const;

    /* * * * * * * * * * * * * * * * * * * * *
     *       SINGLETON-RELATED CONTENT       *
     * * * * * * * * * * * * * * * * * * * * */
private:
    static TextureManager* P_SHARED_INSTANCE;

private:
    TextureManager() {};
    TextureManager(const TextureManager&) {};
    TextureManager& operator=(const TextureManager&) {};

private:
    int extractFrameNumber(const std::string& filename);
    bool tryLoadFile(const std::string& path, const std::string& name = "");

public:
    static void initialize(SDL_Renderer* pRenderer);
    static void destroy();

    static TextureManager* getInstance();
    /* * * * * * * * * * * * * * * * * * * * */
};
