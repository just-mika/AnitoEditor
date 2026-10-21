#include "LoadedScene.h"
#include "TextureManager.h"
#include "CameraManager.h"
#include "LevelDataManager.h"

LoadedScene::LoadedScene() : AScene(SceneTag::LOADED_SCENE) {}
LoadedScene::~LoadedScene() {}
void LoadedScene::onLoadResources()
{
	TextureManager::getInstance()->load("Square.png", "Square");
}

void LoadedScene::onLoadObjects()
{
    CameraManager::getInstance()->getCurrentCamera()->setPos(Vector2D(0.0f));

    LevelDataManager* ldm = LevelDataManager::getInstance();
    if (ldm->hasCurrentFile()) ldm->requestLoad(ldm->getCurrentFile());   // re-enter the open file
    else                       ldm->stepFile(+1);                          // otherwise the first file
}

void LoadedScene::onUnloadResources()
{
	TextureManager::getInstance()->unload("Square");
}