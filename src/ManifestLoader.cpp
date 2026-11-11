#include "ManifestLoader.h"
#include <SDL3/SDL.h>
#include <sstream>
#include <string>
#include <vector>

#include "Settings.h"

std::vector<std::string> readAssetManifest(const std::string& manifestFilename)
{
    std::vector<std::string> out;

    // Desktop/fallback implementation
    std::string path = ASSET_ROOT + manifestFilename;
    size_t size = 0;
    void* data = SDL_LoadFile(path.c_str(), &size);
    if (!data) {
        SDL_Log("readAssetManifest: can't open manifest '%s' : %s", path.c_str(), SDL_GetError());
        return out;
    }

    std::string buf(static_cast<char*>(data), size);
    SDL_free(data);

    //one asset path per line; strip Windows line endings
    std::istringstream iss(buf);
    std::string line;
    while (std::getline(iss, line)) {
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        if (!line.empty())
            out.push_back(line);
    }

    return out;
}
