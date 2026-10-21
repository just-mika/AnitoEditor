#pragma once
#include <string>
#include <vector>
#include "json.hpp"
#include "SpriteRenderer.h"
#include "Vector2D.h"

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

private:

    static constexpr const char* kOrg = "AnitorEditor";
    static constexpr const char* kApp = "BabaylanTales";
    static constexpr const char* kBundledDir = "Assets/levels/";
    std::string pendingLoad;

    

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
