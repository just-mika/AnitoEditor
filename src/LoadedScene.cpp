// LoadedScene.cpp  (replaces the old contents; onLoadResources/onUnloadResources move to the subclasses)
#include "LoadedScene.h"
#include "CameraManager.h"
#include "LevelDataManager.h"
#include "EditorMode.h"
#if EDITOR_MODE
#include "EditorModule.h"
#endif

LoadedScene::LoadedScene(SceneTag tag) : AScene(tag) {}
LoadedScene::~LoadedScene() {}

void LoadedScene::onLoadObjects()
{
    CameraManager::getInstance()->getCurrentCamera()->setPos(Vector2D(0.0f));
    setUpSubsystems();                                   // managers first
    LevelDataManager::getInstance()->enterScene(this);   // opens the pending / open file, if any
}

void LoadedScene::onUnloadObjects()
{
#if EDITOR_MODE
    Editor::EditorModule::getInstance()->setSelectedObject(nullptr);   // objects are about to die
#endif
    if (LevelDataManager* ldm = LevelDataManager::getInstance())
        ldm->leaveScene(this);
    AScene::onUnloadObjects();                           // deletes objects, managers included
}