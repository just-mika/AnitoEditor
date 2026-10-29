#pragma once
#include <string>
#include <vector>
#include "json.hpp"
#include "LoadedScene.h"
#include "SpriteRenderer.h"
#include "Vector2D.h"

class TileMap;
using json = nlohmann::json;

class LevelDataManager
{
public:
    bool saveLevel(const std::string& fileName);
    void requestLoad(const std::string& fileName);
    void applyPending();
    std::vector<std::string> listLevelFiles() const;

    bool loadLevel(const std::string& fileName);
    json serializeScene() const;
    bool buildFromJson(const json& level);
    void newLevel(const std::string& genre);

    const std::string& getCurrentFile() const { return currentFile; }
    bool hasCurrentFile() const { return !currentFile.empty(); }

    void enterScene(LoadedScene* scene);
    void leaveScene(LoadedScene* scene);
    LoadedScene* getActiveScene() const { return activeScene; }
    bool hasActiveScene() const { return activeScene != nullptr; }
    bool canCreate(const std::string& genre) const;

    void loadTilemap(const std::string& mapPath);

    void beginPlay();
    void endPlay();

    void importTilemap(const std::string& mapPath);
    const std::string& getTilemapPath() const { return tilemapPath; }
private:
    TileMap* pTileMap = nullptr;
    std::string tilemapPath;

    json playSnapshot;
    bool restorePending = false;
    void restoreSnapshot();

    std::string pendingTilemap;

    static constexpr const char* kOrg = "AnitoEditor";
    static constexpr const char* kApp = "BabaylanTales";
    static constexpr const char* kBundledDir = "Assets/levels/";
    std::string pendingLoad;
    std::string currentFile;
    LoadedScene* activeScene = nullptr;
    std::string nextFile;                                // file to open after a genre switch
    std::string currentFileGenre;
    static SceneTag sceneTagFor(const std::string& genre);
    
    bool loadingTemplate = false;
    std::string templateGenre;
    void newBlankLevel(const std::string& genre);

    static std::string writableDir();
    static bool isSafeFileName(const std::string& name);
    static bool readFile(const std::string& path, std::string& out);

    static json vecToJson(const Vector2D& v);
    static Vector2D jsonToVec(const json& a);
    static SpriteRenderer* spriteRendererOf(AGameObject* o);

    //save
    static json serializeObject(AGameObject* o);
    
    //load
    static AGameObject* createFromJson(const json& j);
    static void applyFields(AGameObject* o, const json& j);
    static void applyChildren(AGameObject* parent, const json& j);
    void clearScene();

// singleton
public:
    static void initialize();
    static void destroy();

    static LevelDataManager* getInstance();



private:
    static LevelDataManager* P_SHARED_INSTANCE;
    LevelDataManager();
    LevelDataManager(const LevelDataManager&) = delete;
    LevelDataManager& operator=(const LevelDataManager&) = delete;
	
};
