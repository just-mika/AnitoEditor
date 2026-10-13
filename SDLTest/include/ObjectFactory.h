#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "AGameObject.h"

enum class ObjectCategory {
    Primitive,
    Gameplay,
    UI
};

struct ObjectTypeInfo {
    std::string typeKey;
    std::string displayName;
    ObjectCategory category;
    bool isScreenObject;
};

class ObjectFactory {
public:
    // Instantiates an AGameObject by its serialized JSON type key. Does NOT call initialize();
    // GameObjectManager::addObject() does that.
    static AGameObject* createObject(const std::string& typeKey, const std::string& customName = "");

    // Metadata access for ObjectPalette and UI rendering
    static const std::vector<ObjectTypeInfo>& getRegisteredTypes();
    static bool isScreenObjectType(const std::string& typeKey);
    static std::string getDisplayName(const std::string& typeKey);

private:
    static std::string generateUniqueName(const std::string& typeKey);
    static inline uint32_t s_idCounter = 0;
};