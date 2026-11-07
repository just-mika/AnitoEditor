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

enum class Dependency { Physics, GemInputManager, GemDataAsset, SequenceGame };

struct ObjectTypeInfo {
    std::string typeKey;
    std::string displayName;
    ObjectCategory category;
    bool isScreenObject;
    std::vector<Dependency> dependencies; 
    std::vector<std::string> variants;
};

class ObjectFactory {
public:
    static AGameObject* createObject(const std::string& typeKey, const std::string& customName = "");

    static const std::vector<ObjectTypeInfo>& getRegisteredTypes();
    static bool isScreenObjectType(const std::string& typeKey);
    static std::string getDisplayName(const std::string& typeKey);

    static const char* typeKeyOf(AGameObject* obj);
    static std::vector<std::string> getMissingDependencies(const std::string& typeKey);
    static bool isInsideComposite(AGameObject* obj);
    static bool isComposite(AGameObject* pObject);   //a screen whose children are built in code
private:
    static std::string generateUniqueName(const std::string& typeKey);
    static inline uint32_t s_idCounter = 0;
    
};