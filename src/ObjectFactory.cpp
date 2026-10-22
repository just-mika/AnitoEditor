#include "ObjectFactory.h"

#include "PhysicsSystem.h"
#include "GemInputManager.h"
#include "DataAssetManager.h"

// Primitives & General Containers
#include "EmptyObject.h"
#include "Sprite.h"
#include "AnimatedSprite.h"
#include "Text.h"
#include "Background.h"

// Gameplay & Platformer Entities
#include "Player.h"
#include "Gem.h"
#include "Statue.h"
#include "StatuePickup.h"
#include "Gate.h"
#include "Platform.h"

// Puzzle Entities
#include "PuzzleToken.h"
#include "PuzzleHint.h"
#include "PuzzleEnd.h"

// UI Screens & Composites
#include "StoryWindow.h"
#include "GUIButton.h"
#include "GemInputScreen.h"
#include "GemInputToken.h"
#include "GemInputPreview.h"
#include "InventoryGUI.h"
#include "LevelEndGUI.h"
#include "PauseScreen.h"
#include "ObjectiveIntro.h"

static const std::vector<ObjectTypeInfo> S_REGISTERED_TYPES = {
    // --- Primitives ---
    { "empty",             "Empty Object",      ObjectCategory::Primitive, false },
    { "sprite",            "Sprite",            ObjectCategory::Primitive, false },
    { "animated_sprite",   "Animated Sprite",   ObjectCategory::Primitive, false },
    { "text",              "Text",              ObjectCategory::Primitive, false },
    { "background",        "Background",        ObjectCategory::Primitive, false },

    // --- Gameplay Entities ---
    { "player",           "Player",           ObjectCategory::Gameplay, false,{ Dependency::Physics } },
    { "gem",          "Gem",          ObjectCategory::Gameplay, false,{ Dependency::Physics, Dependency::GemDataAsset }, 
    	{ "Gem_Cyan", "Gem_Green", "Gem_Orange", "Gem_Purple", "Gem_Red" } },
	{ "statue",       "Statue",       ObjectCategory::Gameplay, false,{ Dependency::Physics },{ "Statue_Carabao" } },
	{ "gate",             "Gate",             ObjectCategory::Gameplay, false,{ Dependency::Physics } },
	{ "platform",         "Platform",         ObjectCategory::Gameplay, false,{ Dependency::Physics } },
    { "puzzle_token", "Puzzle Token", ObjectCategory::Gameplay, false,{},
  { "Token_Horns", "Token_Claws", "Token_Eyes", "Token_Heart", "Token_Mouth",
			"Token_Scales", "Token_Fire", "Token_Claw", "Token_Wing" } },

    // --- UI & Narrative Composites ---
    { "story_window",      "Story Window",      ObjectCategory::UI,        true  },
    { "puzzle_hint",       "Puzzle Hint",       ObjectCategory::UI,        true  },
    { "puzzle_end",        "Puzzle End",        ObjectCategory::UI,        true  },
    { "gui_button",        "GUI Button",        ObjectCategory::UI,        true  },
	{ "gem_input_screen", "Gem Input Screen", ObjectCategory::UI, true,{ Dependency::GemInputManager, Dependency::GemDataAsset } },
    { "gem_input_token",   "Gem Input Token",   ObjectCategory::UI,        true  },
	{ "gem_input_preview","Gem Input Preview",ObjectCategory::UI, true,{ Dependency::GemDataAsset } },
    { "inventory_gui",    "Inventory GUI",    ObjectCategory::UI, true,{ Dependency::GemDataAsset } },
	{ "level_end_gui",     "Level End GUI",     ObjectCategory::UI,        true  },
    { "pause_screen",      "Pause Screen",      ObjectCategory::UI,        true  },
    { "objective_intro",   "Objective Intro",   ObjectCategory::UI,        true  }
};

const std::vector<ObjectTypeInfo>& ObjectFactory::getRegisteredTypes() {
    return S_REGISTERED_TYPES;
}

std::string ObjectFactory::generateUniqueName(const std::string& typeKey) {
    return typeKey + "_" + std::to_string(++s_idCounter);
}

bool ObjectFactory::isScreenObjectType(const std::string& typeKey) {
    for (const auto& info : S_REGISTERED_TYPES) {
        if (info.typeKey == typeKey) {
            return info.isScreenObject;
        }
    }
    return false;
}

std::string ObjectFactory::getDisplayName(const std::string& typeKey) {
    for (const auto& info : S_REGISTERED_TYPES) {
        if (info.typeKey == typeKey) {
            return info.displayName;
        }
    }
    return typeKey;
}

AGameObject* ObjectFactory::createObject(const std::string& typeKey, const std::string& customName) {
    const std::vector<std::string> missing = getMissingDependencies(typeKey);
    if (!missing.empty())
    {
        std::string list;
        for (const std::string& m : missing) list += (list.empty() ? "" : ", ") + m;
        SDL_Log("[ObjectFactory] cannot create '%s', missing: %s", typeKey.c_str(), list.c_str());
        return nullptr;
    }
	
	// Most objects just need a unique display name.
    const std::string uniqueName = customName.empty() ? generateUniqueName(typeKey) : customName;

    // These types use their *name* as the texture key (SpriteRenderer(strName) in initialize()),
    // so their name must be a loaded texture key unless the caller supplies one.
    auto textureName = [&customName](const char* defaultKey) {
        return customName.empty() ? std::string(defaultKey) : customName;
    };

    AGameObject* newObj = nullptr;

    // 1. Primitives
    if (typeKey == "empty") {
        newObj = new EmptyObject(uniqueName);
    }
    else if (typeKey == "sprite") {
        newObj = new Sprite(uniqueName);
    }
    else if (typeKey == "animated_sprite") {
        newObj = new AnimatedSprite(uniqueName);
    }
    else if (typeKey == "text") {
        newObj = new Text(uniqueName);
    }
    else if (typeKey == "background") {
        newObj = new Background(uniqueName);
    }

    // 2. Gameplay Entities
    else if (typeKey == "player") {
        newObj = new Player(uniqueName);
    }
    else if (typeKey == "gem") {
        newObj = new Gem(textureName("Gem_Cyan"));
    }
    else if (typeKey == "statue") {
        newObj = new Statue(textureName("Statue_Carabao"));
    }
    else if (typeKey == "gate") {
        newObj = new Gate(uniqueName);
    }
    else if (typeKey == "platform") {
        newObj = new Platform(uniqueName);
    }
    else if (typeKey == "puzzle_token") {
        newObj = new PuzzleToken(textureName("Token_Horns"));
    }

    // 3. UI & Narrative Composites
    else if (typeKey == "story_window") {
        newObj = new StoryWindow(uniqueName);
    }
    else if (typeKey == "puzzle_hint") {
        newObj = new PuzzleHint(uniqueName);
    }
    else if (typeKey == "puzzle_end") {
        newObj = new PuzzleEnd(uniqueName);
    }
    else if (typeKey == "gui_button") {
        newObj = new GUIButton(uniqueName);
    }
    else if (typeKey == "gem_input_screen") {
        newObj = new GemInputScreen(uniqueName);
    }
    else if (typeKey == "gem_input_token") {
        newObj = new GemInputToken(textureName("Gem_Cyan"));
    }
    else if (typeKey == "gem_input_preview") {
        newObj = new GemInputPreview(uniqueName);
    }
    else if (typeKey == "inventory_gui") {
        newObj = new InventoryGUI(uniqueName);
    }
    else if (typeKey == "level_end_gui") {
        newObj = new LevelEndGUI(uniqueName);
    }
    else if (typeKey == "pause_screen") {
        newObj = new PauseScreen(uniqueName);
    }
    else if (typeKey == "objective_intro") {
        newObj = new ObjectiveIntro(uniqueName);
    }

    if (newObj != nullptr) {
        // NOTE: do NOT call initialize() here. GameObjectManager::addObject() already
        // initializes the object, so calling it here would attach every component/child twice.

        // Screen vs world space. Uses the real flag the renderers/colliders read (bIsScreenObject).
        newObj->setIsScreenObject(isScreenObjectType(typeKey));
    }

    return newObj;
}

template <class T>
static bool isType(AGameObject* o) { return dynamic_cast<T*>(o) != nullptr; }

const char* ObjectFactory::typeKeyOf(AGameObject* o)
{
    struct Probe { const char* key; bool (*is)(AGameObject*); };
    static const Probe kProbes[] = {
        { "empty",             isType<EmptyObject> },
        { "sprite",            isType<Sprite> },
        { "animated_sprite",   isType<AnimatedSprite> },
        { "text",              isType<Text> },
        { "background",        isType<Background> },
        { "player",            isType<Player> },
        { "gem",               isType<Gem> },
        { "statue",            isType<Statue> },
        { "gate",              isType<Gate> },
        { "platform",          isType<Platform> },
        { "puzzle_token",      isType<PuzzleToken> },
        { "story_window",      isType<StoryWindow> },
        { "puzzle_hint",       isType<PuzzleHint> },
        { "puzzle_end",        isType<PuzzleEnd> },
        { "gui_button",        isType<GUIButton> },
        { "gem_input_screen",  isType<GemInputScreen> },
        { "gem_input_token",   isType<GemInputToken> },
        { "gem_input_preview", isType<GemInputPreview> },
        { "inventory_gui",     isType<InventoryGUI> },
        { "level_end_gui",     isType<LevelEndGUI> },
        { "pause_screen",      isType<PauseScreen> },
        { "objective_intro",   isType<ObjectiveIntro> },
    };
    for (const Probe& p : kProbes)
        if (p.is(o)) return p.key;
    return nullptr;
}

static const char* missingName(Dependency d)
{
    switch (d)
    {
    case Dependency::Physics:
        return PhysicsSystem::getInstance() ? nullptr : "PhysicsSystem";
    case Dependency::GemInputManager:
        return GemInputManager::getInstance() ? nullptr : "GemInputManager";
    case Dependency::GemDataAsset:
        return DataAssetManager::getInstance()->getDataAsset("CollectableGemDataAsset")
            ? nullptr : "CollectableGemDataAsset";
    }
    return nullptr;
}

std::vector<std::string> ObjectFactory::getMissingDependencies(const std::string& typeKey)
{
    std::vector<std::string> missing;
    for (const auto& info : S_REGISTERED_TYPES)
        if (info.typeKey == typeKey)
            for (Dependency d : info.dependencies)
                if (const char* n = missingName(d)) missing.push_back(n);
    return missing;
}