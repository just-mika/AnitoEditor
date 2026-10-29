// PuzzleLoadedScene.cpp
#include "PuzzleLoadedScene.h"
#include "TextureManager.h"
#include "FontManager.h"
#include "GameObjectManager.h"
#include "SequenceGameManager.h"
#include "PuzzleToken.h"
#include "GUIToggle.h"
#include "EnumEventKey.h"
#include "PuzzleEnd.h"

PuzzleLoadedScene::PuzzleLoadedScene() : LoadedScene(SceneTag::PUZZLE_LOADED_SCENE)
{

}
PuzzleLoadedScene::~PuzzleLoadedScene() {}

void PuzzleLoadedScene::onLoadResources()
{
    TextureManager* tm = TextureManager::getInstance();
    tm->load("Square.png", "Square");
    tm->load("GUI/button6.png", "Close_Button");
    tm->load("sequence_game/low_order_bg.png", "BG");
    tm->load("sequence_game/Dragon_RedHead.png", "Dragon_Head");
    tm->load("sequence_game/Dragon_RedTail.png", "Dragon_Tail");
    tm->load("sequence_game/Dragon_RedTile.png", "Dragon_Tile");
    tm->load("sequence_game/Dragon_RedFull.png", "Full_Dragon");
    tm->load("sequence_game/Token_Blank.png", "Token_Blank");
    tm->load("sequence_game/Token_Horns.png", "Token_Horns");
    tm->load("sequence_game/Token_Claws.png", "Token_Claws");
    tm->load("sequence_game/Token_Eyes.png", "Token_Eyes");
    tm->load("sequence_game/Token_Heart.png", "Token_Heart");
    tm->load("sequence_game/Token_Mouth.png", "Token_Mouth");
    tm->load("sequence_game/Token_Scales.png", "Token_Scales");
    tm->load("sequence_game/Token_Fire.png", "Token_Fire");
    tm->load("sequence_game/Token_Claw.png", "Token_Claw");
    tm->load("sequence_game/Token_Wing.png", "Token_Wing");
    tm->load("GUI/pause.png", "Pause_Button");
    tm->load("GUI/story.png", "Story_Button");
    tm->load("GUI/tablet.png", "Tablet");
    tm->loadFromFolder("animations/level_intro", "Intro");
    tm->loadFromFolder("animations/pause_animation", "Pause");
    tm->loadFromFolder("animations/low_order_success", "Puzzle_End");
}

void PuzzleLoadedScene::onUnloadResources()
{
    TextureManager* tm = TextureManager::getInstance();
    for (const char* key : { "Square", "Close_Button", "BG", "Dragon_Head", "Dragon_Tail", "Dragon_Tile",
                             "Full_Dragon", "Token_Blank", "Token_Horns", "Token_Claws", "Token_Eyes",
        "Token_Heart", "Token_Mouth", "Token_Scales", "Token_Fire", "Token_Claw", "Token_Wing",
                             "Pause_Button", "Story_Button", "Tablet", "Intro", "Pause", "Puzzle_End" })
        tm->unload(key);
    FontManager::getInstance()->unloadAllFonts();
}

void PuzzleLoadedScene::setUpSubsystems()
{
    SequenceGameManager::initialize();
}

void PuzzleLoadedScene::rewire()
{
    GameObjectManager* gom = GameObjectManager::getInstance();

    // 1. collect every token in the scene
    std::vector<PuzzleToken*> tokens;
    for (AGameObject* o : gom->getAllObjects())
        if (PuzzleToken* t = dynamic_cast<PuzzleToken*>(o)) tokens.push_back(t);

    // 2. build the layout, skipping tokens that have no target slot
    std::vector<Vector2D> starts, targets;
    std::vector<std::string> combo;
    for (PuzzleToken* t : tokens) {
        starts.push_back(t->getPos());                    // every token owns a start slot
        if (t->hasTargetPos()) {                          // only tokens with a dragon tile add a target
            targets.push_back(t->getTargetPos());
            combo.push_back(t->getName());
        }
    }

    SequenceGameManager* sgm = SequenceGameManager::getInstance();
    sgm->configure(starts, targets);
    for (PuzzleToken* t : tokens) sgm->addToken(t);       // no hasTargetPos check here any more
    sgm->setCombination(combo);

    if (AGameObject* b = gom->findObjectByName("Pause_Button"))
        if (!b->findComponentByName("GUIToggle"))
            b->attachComponent(new GUIToggle(EventKey::PAUSE_SCREEN));

    if (AGameObject* b = gom->findObjectByName("Story_Button"))
        if (!b->findComponentByName("GUIToggle"))
            b->attachComponent(new GUIToggle(EventKey::STORY_SCREEN));

    if (PuzzleEnd* end = dynamic_cast<PuzzleEnd*>(gom->findObjectByName("EndScreen")))
        end->setDragons(gom->findObjectByName("DragonContainer"),
            gom->findObjectByName("Complete_Dragon"));
}
