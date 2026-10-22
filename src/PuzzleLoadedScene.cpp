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
    this->setDefaults();
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
                             "Pause_Button", "Story_Button", "Tablet", "Intro", "Pause", "Puzzle_End" })
        tm->unload(key);
    FontManager::getInstance()->unloadAllFonts();
}

void PuzzleLoadedScene::setUpSubsystems()
{
    SequenceGameManager::initialize();
}

static std::vector<Vector2D> readSlots(const nlohmann::json& a)
{
    std::vector<Vector2D> v;
    for (const auto& p : a)
        v.push_back(Vector2D(p.at(0).get<float>(), p.at(1).get<float>()));
    return v;
}

void PuzzleLoadedScene::readLevelData(const nlohmann::json& level)
{
    setDefaults();
    if (!level.contains("puzzle")) return;
    const nlohmann::json& p = level["puzzle"];

    if (p.contains("startSlots"))  startSlots = readSlots(p["startSlots"]);
    if (p.contains("targetSlots")) targetSlots = readSlots(p["targetSlots"]);
    if (p.contains("combination")) combination = p["combination"].get<std::vector<std::string>>();
    if (p.contains("hints"))       hints = p["hints"].get<std::unordered_map<std::string, std::string>>();
}

void PuzzleLoadedScene::writeLevelData(nlohmann::json& level) const
{
    nlohmann::json p;
    p["startSlots"] = nlohmann::json::array();
    p["targetSlots"] = nlohmann::json::array();
    for (const Vector2D& v : startSlots)  p["startSlots"].push_back({ v.x, v.y });
    for (const Vector2D& v : targetSlots) p["targetSlots"].push_back({ v.x, v.y });
    p["combination"] = combination;
    p["hints"] = hints;
    level["puzzle"] = std::move(p);
}
void PuzzleLoadedScene::rewire()
{
    GameObjectManager* gom = GameObjectManager::getInstance();

    // The old manager still points at tokens that were deleted, so replace it.
    if (SequenceGameManager* old = SequenceGameManager::getInstance())
    {
        SequenceGameManager* manager = SequenceGameManager::getInstance();
        manager->configure(startSlots, targetSlots);
    }

    SequenceGameManager* sgm = SequenceGameManager::getInstance();
    sgm->configure(startSlots, targetSlots);

    std::vector<std::string> names;
    for (AGameObject* o : gom->getAllObjects())
        if (PuzzleToken* t = dynamic_cast<PuzzleToken*>(o))
        {
            sgm->addToken(t);
            names.push_back(t->getName());
        }

    // No saved combination yet: fall back to token creation order.
    const std::vector<std::string>& combo = (combination.size() == 3) ? combination : names;
    if (combo.size() == 3)
        sgm->setCombination(combo[0], combo[1], combo[2]);
	
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

void PuzzleLoadedScene::setDefaults()
{
    startSlots.clear();
    targetSlots.clear();
    combination.clear();
    hints.clear();
}

