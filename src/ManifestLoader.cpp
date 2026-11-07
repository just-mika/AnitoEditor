#include "ManifestLoader.h"
#include <SDL3/SDL.h>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> readAssetManifest(const std::string& manifestFilename)
{
    std::vector<std::string> out;

    // Desktop/fallback implementation
    std::string path = "Assets/" + manifestFilename;
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data) {
        SDL_Log("readAssetManifest: can't open manifest '%s' : %s", path.c_str(), SDL_GetError());
        return out;
    }
}