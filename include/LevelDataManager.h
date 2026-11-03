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
    void newBlankLevel(const std::string& genre);

	//undo/redo
    void recordEdit();
    void resetHistory();
    void requestUndo();
    void requestRedo();
    bool canUndo() const;
    bool canRedo() const;

    //turns a texture reference from a level file or the editor (a path, or an old-style key)
	//into a loaded texture key. Loads the file on demand. Returns "" if it can't be found.
    static std::string resolveTexture(const std::string& ref);

    //puts a texture (path, folder or old key) on an object's sprite.
//For animated sprites it also swaps the animation's frames.
    static bool setObjectTexture(AGameObject* o, const std::string& ref);

    AGameObject* duplicateObject(AGameObject* pObject);

    void rewireScene() { if (activeScene) activeScene->rewire(); }
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
    std::vector<std::string> pathTextures;   //textures loaded by ensureTexture

    std::string pendingLoad;
    std::string currentFile;
    LoadedScene* activeScene = nullptr;
    std::string nextFile;                                // file to open after a genre switch
    std::string currentFileGenre;
    static SceneTag sceneTagFor(const std::string& genre);
    
    bool loadingTemplate = false;
    std::string templateGenre;

    std::vector<json> undoStack;
    std::vector<json> redoStack;
    json lastState;
    int nPendingHistory = 0;
    static constexpr size_t kMaxHistory = 50;

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
