#include "ObjectFactory.h"

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
    { "player",            "Player",            ObjectCategory::Gameplay,  false },
    { "gem",               "Gem",               ObjectCategory::Gameplay,  false },
    { "statue",            "Statue",            ObjectCategory::Gameplay,  false },
    { "gate",              "Gate",              ObjectCategory::Gameplay,  false },
    { "platform",          "Platform",          ObjectCategory::Gameplay,  false },
    { "puzzle_token",      "Puzzle Token",      ObjectCategory::Gameplay,  false },

    // --- UI & Narrative Composites ---
    { "story_window",      "Story Window",      ObjectCategory::UI,        true  },
    { "puzzle_hint",       "Puzzle Hint",       ObjectCategory::UI,        true  },
    { "puzzle_end",        "Puzzle End",        ObjectCategory::UI,        true  },
    { "gui_button",        "GUI Button",        ObjectCategory::UI,        true  },
    { "gem_input_screen",  "Gem Input Screen",  ObjectCategory::UI,        true  },
    { "gem_input_token",   "Gem Input Token",   ObjectCategory::UI,        true  },
    { "gem_input_preview", "Gem Input Preview", ObjectCategory::UI,        true  },
    { "inventory_gui",     "Inventory GUI",     ObjectCategory::UI,        true  },
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
