#pragma once

#include "AComponent.h"
#include "Vector2D.h"
#include <vector>

class AGameObject
{
protected:
    bool bEnabled;
    std::string strName;

    AGameObject* pParent;
    std::vector<AGameObject*> vecChildren;
    std::vector<AComponent*> vecComponent;

    // X and Y since 2d
    bool bIsScreenObject;
    Vector2D fVecTranslate;
    Vector2D fVecScale;
    float fRot;

    bool m_isScreenObject = false;
    bool bIsSystemObject = false;

public:
    AGameObject(std::string strName);
    virtual ~AGameObject();

public:
    virtual void initialize() = 0;
    virtual void onEnable() {};
    virtual void onDisable() {};
    virtual void processInput(SDL_Event* eEvent);
    virtual void update(float fDeltaTime);
    virtual void draw(SDL_Renderer* pRenderer);

public:
    void attachChild(AGameObject* pChild);
    void detachChild(AGameObject* pChild);
    AGameObject* findChildByName(std::string strName);
    int  getChildIndex(AGameObject* pChild) const;          // -1 if not a child
    void insertChildAt(AGameObject* pChild, int nIndex);    // no initialize() call (see .cpp)
    bool isDescendantOf(const AGameObject* pAncestor) const; // strict: false for itself
    std::vector<AGameObject*> getChildren();
    std::vector<AGameObject*> getChildrenRecursively();
    void adoptChild(AGameObject* pChild); // adopt child w/o reinitializing
    void toJson();
    void fromJson();

    void attachComponent(AComponent* pComponent);
    void detachComponent(AComponent* pComponent);
    AComponent* findComponentByName(std::string strName);
    std::vector<AComponent*> getComponents(ComponentType EType);
    std::vector<AComponent*> getComponentsRecursively(ComponentType EType, bool bInclusive = true);
    bool componentExists(std::string strName);
    const std::vector<AComponent*>& getAllComponents() const { return vecComponent; }

public:
    bool getEnabled() const;
    void setEnabled(bool bEnabled);
    std::string getName() const;
    void setName(std::string strName);
    AGameObject* getParent() const;
    void setParent(AGameObject* pParent);
    void setPos(Vector2D fVecTranslate);
    void setLocalPos(Vector2D fVecTranslate);
    void setScale(Vector2D fVecScale);
    void setLocalScale(Vector2D fVecScale);
    Vector2D getPos();
    Vector2D getLocalPos();
    Vector2D getScale();
    Vector2D getLocalScale();
    void setRot(float fRot);
    void setLocalRot(float fRot);
    float getRot();
    float getLocalRot();

    bool getIsScreenObject() const;
    void setIsScreenObject(bool bIsScreenObject);
    bool getFollowParent();
    void setFollowParent(bool bFollowParent);
    bool isGloballyEnabled() const;

    void setScreenObject(bool isScreen) { m_isScreenObject = isScreen; }
    bool isScreenObject() const { return m_isScreenObject; }

    void setIsSystemObject(bool b) { bIsSystemObject = b; }
    bool isSystemObject() const { return bIsSystemObject; }
};
