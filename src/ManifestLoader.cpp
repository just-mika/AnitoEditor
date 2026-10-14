#include "ManifestLoader.h"
#include <SDL3/SDL.h>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> readAssetManifest(const std::string& manifestFilename)
{
    std::vector<std::string> out;

    // Desktop/fallback implementation
    size_t size = 0;
    void* data = SDL_LoadFile(manifestFilename.c_str(), &size);
    if (!data) {
        SDL_Log("readAssetManifest: can't open manifest '%s' : %s", manifestFilename.c_str(), SDL_GetError());
        return out;
    }

    std::string buf(static_cast<char*>(data), (size_t)size);
    SDL_free(data);

    std::istringstream iss(buf);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty()) {
            out.push_back(line);
        }
    }

    return out;
}