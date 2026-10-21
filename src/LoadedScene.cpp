#include "LoadedScene.h"
#include "TextureManager.h"
#include "CameraManager.h"

LoadedScene::LoadedScene() : AScene(SceneTag::LOADED_SCENE) {}
LoadedScene::~LoadedScene() {}
void LoadedScene::onLoadResources()
{
	TextureManager::getInstance()->load("Square.png", "Square");
}

void LoadedScene::onLoadObjects()
{
	CameraManager::getInstance()->getCurrentCamera()->setPos(Vector2D(0.0f));
}

void LoadedScene::onUnloadResources()
{
	TextureManager::getInstance()->unload("Square");
}