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
#include "Gem.h"
#include "GUIToggle.h"
#include "EmptyObject.h"
#include "Sprite.h"
#include "Text.h"
#include "GUIButton.h"
#include "ButtonInput.h"
#include "SpriteRenderer.h"
#include "ObjectiveManager.h"
#include "ObjectiveButton_Controller.h"
#include "JumpButtonController.h"
#include "InteractButtonController.h"
#include "VirtualJoystick.h"
#include "AudioPlayer.h"
#include "ObjectiveIntro.h"

PlatformerLoadedScene::PlatformerLoadedScene() : LoadedScene(SceneTag::PLATFORMER_LOADED_SCENE) {}
PlatformerLoadedScene::~PlatformerLoadedScene() {}

void PlatformerLoadedScene::onLoadResources()
{
    TextureManager* tm = TextureManager::getInstance();

    tm->loadFromFolder("animations/level_intro", "Intro");
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
    tm->load("gems/green_grab_gem.png", "Gem_Green_Grab");
    tm->load("gems/orange_grab_gem.png", "Gem_Orange_Grab");
    tm->load("gems/purple_grab_gem.png", "Gem_Purple_Grab");
    tm->load("gems/red_grab_gem.png", "Gem_Red_Grab");
    tm->load("gems/colorless_grab_gem.png", "Gem_Colorless_Grab");
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
    for (const char* key : { "Panel", "Intro","player_idle", "player_run", "player_jump", "player_fall", "Pause", "Pickup",
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

    GameObjectManager* gom = GameObjectManager::getInstance();
    this->vecHUD.clear();   //last load's objects were deleted with that scene

    // ---- objective: manager, world arrow, HUD button (scene-made, not level data) ----
    EmptyObject* pObjectiveObj = new EmptyObject("ObjectiveManager_Object");
    this->pObjectiveManager = new ObjectiveManager();
    pObjectiveObj->attachComponent(this->pObjectiveManager);
    pObjectiveObj->setIsSystemObject(true);
    gom->addObject(pObjectiveObj);
    this->pObjectiveManager->initialize();   //registers for OBJECTIVE_BUTTON

    Sprite* pArrow = new Sprite("Arrow", "Arrow", Vector2D(1077.777f, 463.686f), 1.0f);
    pArrow->setScale(Vector2D(.5f));
    pArrow->setIsSystemObject(true);
    gom->addObject(pArrow);
    pArrow->setEnabled(false);   //shown while it points somewhere
    this->vecHUD.push_back(pArrow);

    // ---- mobile controls (touch only) ----
#if defined(SDL_PLATFORM_ANDROID)
    GUIButton* pJumpBtn = new GUIButton("JumpBtn", "jump_button");
    pJumpBtn->setIsScreenObject(true);
    pJumpBtn->setIsSystemObject(true);
    gom->addObject(pJumpBtn);
    pJumpBtn->setPos(Vector2D(1750, 900));
    pJumpBtn->setScale(Vector2D(.15f));
    pJumpBtn->attachComponent(new JumpButtonController());
    pJumpBtn->setEnabled(false);
    this->vecHUD.push_back(pJumpBtn);

    // PlayerController finds this by name ("InteractBtn") and shows it near interactables
    GUIButton* pInteractBtn = new GUIButton("InteractBtn", "interact_button");
    pInteractBtn->setIsScreenObject(true);
    pInteractBtn->setIsSystemObject(true);
    gom->addObject(pInteractBtn);
    pInteractBtn->setPos(Vector2D(1550, 900));
    pInteractBtn->setScale(Vector2D(.15f));
    pInteractBtn->attachComponent(new InteractButtonController());
    pInteractBtn->setEnabled(false);
    this->vecHUD.push_back(pInteractBtn);

    Sprite* pJoyBase = new Sprite("VirtualJoystickBase", "joystick_base", Vector2D(200.0f, 900.0f), Vector2D(0.25f));
    pJoyBase->setIsScreenObject(true);
    pJoyBase->setIsSystemObject(true);
    Sprite* pJoyThumb = new Sprite("VirtualJoystickThumb", "joystick", Vector2D(0.0f, 0.0f), Vector2D(0.5f));
    pJoyThumb->setIsScreenObject(true);
    pJoyBase->attachChild(pJoyThumb);
    gom->addObject(pJoyBase);
    pJoyBase->attachComponent(new VirtualJoystick(250.0f));
    pJoyBase->setEnabled(false);
    this->vecHUD.push_back(pJoyBase);
#endif
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
        da->initialize();
}

void PlatformerLoadedScene::rewire()
{
    GameObjectManager* gom = GameObjectManager::getInstance();

    std::vector<AGameObject*> all = gom->getAllObjectsRecursively();

    CollectableGemDataAsset* da = (CollectableGemDataAsset*)
        DataAssetManager::getInstance()->getDataAsset("CollectableGemDataAsset");

    bool havePlayer = false, haveGate = false;
    for (AGameObject* o : all)
    {
        if (!havePlayer)
            if (Player* p = dynamic_cast<Player*>(o))
            {
                PlayerManager::getInstance()->setPlayer(p);
                havePlayer = true;
            }
        if (!haveGate)
            if (Gate* g = dynamic_cast<Gate*>(o))
            {
                GemInputManager::getInstance()->addGate(g);
                haveGate = true;
            }

        if (Gem* gem = dynamic_cast<Gem*>(o))
            if (da && (!gem->getPickupTitle().empty() || !gem->getPickupText().empty()))
                da->setGemTexts(gem->getName(), gem->getPickupTitle(), gem->getPickupText());
    }

    // the level was rebuilt: the objective manager must forget the old Player
    if (this->pObjectiveManager)
        this->pObjectiveManager->resetSceneRefs();

    if (this->pObjectiveManager)
    {
        EventBroadcaster::getInstance()->unregisterListener(this->pObjectiveManager);
        EventBroadcaster::getInstance()->registerListener(this->pObjectiveManager);
        
        if (EventBroadcaster::getInstance()->isBlocking())
            this->pObjectiveManager->setListenerEnabled(false);
    	
    	this->pObjectiveManager->resetSceneRefs();
    }

    // an open objective intro blocks everything else; it blocked when it was created,
// so re-apply now that every listener (screens, ObjectiveManager) is registered
    for (AGameObject* o : gom->getAllObjectsRecursively())
        if (ObjectiveIntro* pIntro = dynamic_cast<ObjectiveIntro*>(o))
            if (pIntro->isGloballyEnabled())
                pIntro->blockOthers();

    // scene-made HUD (the arrow, mobile controls) draws on top of the level:
    // system objects sit at the front of the list, so move them to the end
    for (AGameObject* pHUD : this->vecHUD)
        gom->moveObject(pHUD, nullptr, (int)gom->getAllObjects().size());

    //music starts once the level is built; rewire also runs on undo/restart, so don't restart it
    if (!AudioManager::getInstance()->isPlaying("BGM"))
        AudioManager::getInstance()->play(new AudioPlayer("Jungle", "BGM", AudioGroupTag::MUSIC, OnAudioFinished::LOOP));

#if defined(SDL_PLATFORM_ANDROID)
    //the level is built: show the touch controls (the interact button stays hidden until the player reaches something)
    if (AGameObject* pJump = gom->findObjectByName("JumpBtn")) pJump->setEnabled(true);
    if (AGameObject* pJoy = gom->findObjectByName("VirtualJoystickBase")) pJoy->setEnabled(true);
#endif
}