#include "PlatformerLoadedScene.h"
#include "TextureManager.h"
#include "AudioManager.h"
#include "FontManager.h"
#include "DataAssetManager.h"
#include "CollectableGemDataAsset.h"
#include "GameObjectManager.h"
#include "PhysicsSystem.h"
#include "PlayerManager.h"
#include "GemInputManager.h"
#include "Player.h"
#include "Gate.h"
#include "GUIToggle.h"

PlatformerLoadedScene::PlatformerLoadedScene() : LoadedScene(SceneTag::PLATFORMER_LOADED_SCENE) {}
PlatformerLoadedScene::~PlatformerLoadedScene() {}

void PlatformerLoadedScene::onLoadResources()
{
    TextureManager* tm = TextureManager::getInstance();

    tm->loadFromFolder("animations/player_idle", "player_idle");
    tm->loadFromFolder("animations/player_run", "player_run");
    tm->loadFromFolder("animations/player_jump", "player_jump");
    tm->loadFromFolder("animations/player_falling", "player_fall");
    tm->loadFromFolder("animations/pause_animation", "Pause");
    tm->loadFromFolder("animations/player_grab_gem", "Pickup");
    tm->loadFromFolder("animations/inventory_animation", "Inventory");
    tm->loadFromFolder("animations/high_order_success", "End_Level");

    tm->load("platformer/trees.png", "Trees_BG");
    tm->load("platformer/gate.png", "Gate");
    tm->load("left_room/statue_carabao.png", "Statue_Carabao");

    tm->load("tilemaps/bottom.png", "Grass_Tile_BC");
    tm->load("tilemaps/bottomleft.png", "Grass_Tile_BL");
    tm->load("tilemaps/bottomright.png", "Grass_Tile_BR");
    tm->load("tilemaps/dirt.png", "Grass_Tile_MC");
    tm->load("tilemaps/left.png", "Grass_Tile_ML");
    tm->load("tilemaps/right.png", "Grass_Tile_MR");
    tm->load("tilemaps/top.png", "Grass_Tile_TC");
    tm->load("tilemaps/topleft.png", "Grass_Tile_TL");
    tm->load("tilemaps/topright.png", "Grass_Tile_TR");
    tm->load("tilemaps/cornergrass1.png", "Grass_Tile_C1");
    tm->load("tilemaps/cornergrass2.png", "Grass_Tile_C2");
    tm->load("tilemaps/cornergrass3.png", "Grass_Tile_C3");
    tm->load("tilemaps/cornergrass4.png", "Grass_Tile_C4");
    tm->load("tilemaps/grass_platform.png", "Grass_Platform");

    tm->load("gems/gem_cyan.png", "Gem_Cyan");
    tm->load("gems/gem_green.png", "Gem_Green");
    tm->load("gems/gem_orange.png", "Gem_Orange");
    tm->load("gems/gem_purple.png", "Gem_Purple");
    tm->load("gems/gem_red.png", "Gem_Red");
    tm->load("gems/blue_grab_gem.png", "Gem_Cyan_Grab");
    tm->load("gems/green_grab_gem.png", "Gem_Green_grab");
    tm->load("gems/orange_grab_gem.png", "Gem_Orange_Grab");
    tm->load("gems/purple_grab_gem.png", "Gem_Purple_grab");
    tm->load("gems/red_grab_gem.png", "Gem_Red_Grab");
    tm->load("gems/blue_inventory_gem.png", "Gem_Cyan_Inventory");
    tm->load("gems/green_inventory_gem.png", "Gem_Green_Inventory");
    tm->load("gems/orange_inventory_gem.png", "Gem_Orange_Inventory");
    tm->load("gems/purple_inventory_gem.png", "Gem_Purple_Inventory");
    tm->load("gems/red_inventory_gem.png", "Gem_Red_Inventory");
    tm->load("gems/colorless_inventory_gem.png", "Gem_Colorless_Inventory");

    tm->load("GUI/pause.png", "Pause_Button");
    tm->load("GUI/story.png", "Story_Button");
    tm->load("GUI/inventory.png", "Items_Button");
    tm->load("GUI/button6.png", "Close_Button");
    tm->load("Square.png", "Square");
    tm->load("GUI/arrow.png", "Arrow");
    tm->load("GUI/question_mark.png", "Q_Mark");
    tm->load("GUI/back.png", "Return");
    tm->load("GUI/ArrowVector.png", "Arrow_Vector");
    tm->load("GUI/tablet.png", "Tablet");
    tm->load("GUI/typing_screen.png", "typing_screen");
    tm->load("GUI/typing_screen_top.png", "typing_top");
    tm->load("GUI/typing_screen_tablet.png", "typing_tablet");
    tm->load("GUI/scrollview.png", "scrollview");
    tm->load("GUI/tablet_bottom.png", "tablet_bottom");
    tm->load("GUI/Circle.png", "Circle");
    tm->load("GUI/title_button.png", "Exit");
    tm->load("GUI/interact button.png", "interact_button");
    tm->load("GUI/jump button.png", "jump_button");
    tm->load("GUI/joystick_outer.png", "joystick_base");
    tm->load("GUI/joystick_inner.png", "joystick");

    AudioManager* am = AudioManager::getInstance();
    am->load("sounds/Music/Jungle.wav", "Jungle");
    am->load("sounds/SFX/arcade-game-jump.wav", "Jump");
    am->load("sounds/SFX/feet-landing-jump.wav", "Land");
    am->load("sounds/SFX/Pickup_Gem.wav", "Pickup");

    // GemInputManager reads this when it is created, so it must exist first
    DataAssetManager::getInstance()->addDataAsset("CollectableGemDataAsset", new CollectableGemDataAsset());
}

void PlatformerLoadedScene::onUnloadResources()
{
    TextureManager* tm = TextureManager::getInstance();
    for (const char* key : { "player_idle", "player_run", "player_jump", "player_fall", "Pause", "Pickup",
                             "Inventory", "End_Level", "Trees_BG", "Gate", "Statue_Carabao",
                             "Grass_Tile_BC", "Grass_Tile_BL", "Grass_Tile_BR", "Grass_Tile_MC", "Grass_Tile_ML",
                             "Grass_Tile_MR", "Grass_Tile_TC", "Grass_Tile_TL", "Grass_Tile_TR",
                             "Grass_Tile_C1", "Grass_Tile_C2", "Grass_Tile_C3", "Grass_Tile_C4", "Grass_Platform",
                             "Gem_Cyan", "Gem_Green", "Gem_Orange", "Gem_Purple", "Gem_Red",
                             "Gem_Cyan_Grab", "Gem_Green_grab", "Gem_Orange_Grab", "Gem_Purple_grab", "Gem_Red_Grab",
                             "Gem_Cyan_Inventory", "Gem_Green_Inventory", "Gem_Orange_Inventory",
                             "Gem_Purple_Inventory", "Gem_Red_Inventory", "Gem_Colorless_Inventory",
                             "Pause_Button", "Story_Button", "Items_Button", "Close_Button", "Square", "Arrow",
                             "Q_Mark", "Return", "Arrow_Vector", "Tablet", "typing_screen", "typing_top",
                             "typing_tablet", "scrollview", "tablet_bottom", "Circle", "Exit",
                             "interact_button", "jump_button", "joystick_base", "joystick" })
        tm->unload(key);

    AudioManager* am = AudioManager::getInstance();
    for (const char* key : { "Jungle", "Jump", "Land", "Pickup" })
        am->unload(key);

    FontManager::getInstance()->unloadAllFonts();
    DataAssetManager::getInstance()->removeDataAsset("CollectableGemDataAsset");
}

void PlatformerLoadedScene::onUnloadObjects()
{
    AudioManager::getInstance()->stopAll();
    LoadedScene::onUnloadObjects();
}

void PlatformerLoadedScene::setUpSubsystems()
{
    PhysicsSystem::initialize();
    PlayerManager::initialize();
    GemInputManager::initialize();
}

// Called after clearScene() and BEFORE the level's objects are built.
// GemInputScreen registers its tokens/boxes with the manager while it is built,
// so stale entries from the previous level must be gone first.
void PlatformerLoadedScene::resetSubsystems()
{
    PlayerManager::getInstance()->reset();
    GemInputManager::getInstance()->reset();
    PhysicsSystem::getInstance()->reset();

    CollectableGemDataAsset* da = (CollectableGemDataAsset*)
        DataAssetManager::getInstance()->getDataAsset("CollectableGemDataAsset");
    if (da)
        for (const char* g : { "Gem_Cyan", "Gem_Green", "Gem_Orange", "Gem_Purple", "Gem_Red" })
            da->setGemPickedUp(g, false);
}

void PlatformerLoadedScene::attachToggle(const char* name, EventKey key)
{
    AGameObject* b = GameObjectManager::getInstance()->findObjectByName(name);
    if (b && !b->findComponentByName("GUIToggle"))
        b->attachComponent(new GUIToggle(key));
}

void PlatformerLoadedScene::rewire()
{
    GameObjectManager* gom = GameObjectManager::getInstance();

    bool havePlayer = false, haveGate = false;
    for (AGameObject* o : gom->getAllObjects())
    {
        if (!havePlayer)
            if (Player* p = dynamic_cast<Player*>(o))
            {
                PlayerManager::getInstance()->setPlayer(p);
                havePlayer = true;
                continue;
            }
        if (!haveGate)
            if (Gate* g = dynamic_cast<Gate*>(o))
            {
                GemInputManager::getInstance()->addGate(g);
                haveGate = true;
            }
    }

    // components can't be saved in JSON, so re-attach them by name
    attachToggle("Pause_Button", EventKey::PAUSE_SCREEN);
    attachToggle("Story_Button", EventKey::STORY_SCREEN);
    attachToggle("Items_Button", EventKey::INVENTORY_SCREEN);
}