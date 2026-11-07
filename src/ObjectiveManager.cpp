#include "ObjectiveManager.h"
#include "GameObjectManager.h"
#include "AGameObject.h"
#include "SpriteRenderer.h"
#include "Gem.h"
#include "ButtonInput.h"
#include "EventBroadcaster.h"
#include "EnumEventKey.h"
#include <cmath>
#include <iostream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include "Gate.h"

// local PI constant
constexpr float PI_F = 3.14159265358979323846f;

ObjectiveManager::ObjectiveManager() : AComponent("ObjectiveManager", ComponentType::SCRIPT)
{
    duration = 1.0f;
    isPathFinding = false;
    elapsed = 0.0f;

    // default listener config
    this->bListenerEnabled = true;
    this->EKey = EventKey::OBJECTIVE_BUTTON;
}

ObjectiveManager::~ObjectiveManager()
{
    // EventBroadcaster unregistering
    // if (EventBroadcaster::getInstance()) EventBroadcaster::getInstance()->unregisterListener(this);
}

void ObjectiveManager::initialize()
{
    // attempt to resolve major objects
    pPlayer = GameObjectManager::getInstance()->findObjectInTree("Player");
    pArrow = GameObjectManager::getInstance()->findObjectInTree("Arrow");
    pObjectivePanel = GameObjectManager::getInstance()->findObjectInTree("ObjectivePanel");
    pOptionalPanel = GameObjectManager::getInstance()->findObjectInTree("OptionalPanel");

    // register for toggle events (GUIToggle broadcasts)
    if (EventBroadcaster::getInstance())
        EventBroadcaster::getInstance()->registerListener(this);

    // debug summary
    std::cout << "[ObjectiveManager] initialize summary:\n";
    std::cout << "  pPlayer: " << (pPlayer ? pPlayer->getName() : "NULL") << "\n";
    std::cout << "  pArrow: " << (pArrow ? pArrow->getName() : "NULL")
        << (pArrow ? (" pos=(" + std::to_string(pArrow->getPos().x) + "," + std::to_string(pArrow->getPos().y) + ")") : "") << "\n";
    std::cout << "  pObjectivePanel: " << (pObjectivePanel ? pObjectivePanel->getName() : "NULL") << "\n";
    std::cout << "  listener EKey: " << static_cast<int>(this->EKey) << "\n";
}

void ObjectiveManager::perform()
{
    // lazy-find scene objects in case initialization ran before they were created
    if (!pPlayer)           pPlayer = GameObjectManager::getInstance()->findObjectInTree("Player");
    if (!pArrow)            pArrow = GameObjectManager::getInstance()->findObjectInTree("Arrow");

    // Panels can be noisy if not present — only try to find them once (flag-based)
    if (!bTriedFindPanels)
    {
        pObjectivePanel = GameObjectManager::getInstance()->findObjectInTree("ObjectivePanel");
        pOptionalPanel = GameObjectManager::getInstance()->findObjectInTree("OptionalPanel");
        bTriedFindPanels = true;
    }

    //// debug heartbeat
    //static int dbgFrame = 0;

    //if (++dbgFrame % 60 == 0) {
    //    std::cout << "[ObjectiveManager] perform (isPathFinding=" << (isPathFinding ? "true" : "false")
    //        << ", elapsed=" << elapsed << ")\n";
    //    if (pArrow)
    //        std::cout << "  Arrow enabled=" << (pArrow->getEnabled() ? "true" : "false")
    //        << " pos=(" << pArrow->getPos().x << "," << pArrow->getPos().y << ")\n";
    //}

    // UI visibility: keep optional panel disabled when no gems
    auto gems = collectActiveGems();
    if (gems.empty())
    {
        if (pOptionalPanel) pOptionalPanel->setEnabled(false);
    }

    // Advance arrow tween if active
    if (isPathFinding && pArrow)
    {
        float fDelta = this->fDeltaTime;
        elapsed += fDelta;

        float totalDist = distance(startPos, targetPos);
        float speed = (totalDist / std::max(duration, 0.0001f));
        float maxMove = speed * fDelta;
        Vector2D curPos = pArrow->getPos();
        Vector2D newPos = moveTowards(curPos, targetPos, maxMove);
        pArrow->setPos(newPos);

        Vector2D dir = targetPos - newPos;
        float angleRad = std::atan2(dir.y, dir.x);
        float angleDeg = angleRad * 180.0f / PI_F;
        pArrow->setRot(angleDeg);

        if (elapsed >= duration || distance(newPos, targetPos) <= FINISH_EPS)
        {
            stopArrowPathfinding();
        }
    }
}

void ObjectiveManager::findDoor()
{
    if (isPathFinding) return;
    if (!pPlayer) return;

    AGameObject* doorObj = findDoorObject();
    if (!doorObj)
    {
        std::cout << "[ObjectiveManager] No door object found in scene." << std::endl;
        return;
    }

    Vector2D doorPos = doorObj->getPos();
    startPos = pPlayer->getPos();
    startArrowPathfinding(doorPos);
}

void ObjectiveManager::findGems()
{
    if (isPathFinding) return;
    if (!pPlayer) return;

    auto gems = collectActiveGems();
    if (gems.empty())
    {
        std::cout << "[ObjectiveManager] No gems to find\n";
        return;
    }

    Vector2D closest = getClosestGemToPlayer(gems);
    startPos = pPlayer->getPos();
    startArrowPathfinding(closest);
}

void ObjectiveManager::toggleObjectivePanel()
{
    if (!pObjectivePanel) return;
    bool active = pObjectivePanel->getEnabled();
    pObjectivePanel->setEnabled(!active);
}

void ObjectiveManager::toggleOptionalPanel()
{
    if (!pOptionalPanel) return;
    bool active = pOptionalPanel->getEnabled();
    pOptionalPanel->setEnabled(!active);
}

void ObjectiveManager::startArrowPathfinding(const Vector2D& target)
{
    if (!pArrow || !pPlayer) return;

    //std::cout << "[ObjectiveManager] startArrowPathfinding: target=("
    //    << target.x << "," << target.y << "), playerPos=("
    //    << (pPlayer ? std::to_string(pPlayer->getPos().x) + "," + std::to_string(pPlayer->getPos().y) : "NULL")
    //    << "), duration=" << duration << "\n";

    targetPos = target;
    startPos = pPlayer->getPos();
    elapsed = 0.0f;
    isPathFinding = true;

    pArrow->setPos(startPos);
    Vector2D dir = targetPos - startPos;
    float angleRad = std::atan2(dir.y, dir.x);
    float angleDeg = angleRad * 180.0f / PI_F;
    pArrow->setRot(angleDeg);
    pArrow->setEnabled(true);
}

void ObjectiveManager::stopArrowPathfinding()
{
    isPathFinding = false;
    elapsed = 0.0f;
    if (pArrow) pArrow->setEnabled(false);
}

std::vector<Vector2D> ObjectiveManager::collectActiveGems()
{
    std::vector<Vector2D> out;
    for (AGameObject* obj : GameObjectManager::getInstance()->getAllObjectsRecursively())
        if (Gem* g = dynamic_cast<Gem*>(obj))
            out.push_back(g->getPos());
    return out;
}


AGameObject* ObjectiveManager::findDoorObject()
{
    for (AGameObject* obj : GameObjectManager::getInstance()->getAllObjectsRecursively())
        if (dynamic_cast<Gate*>(obj))
            return obj;
    return nullptr;
}

Vector2D ObjectiveManager::getClosestGemToPlayer(const std::vector<Vector2D>& gems)
{
    Vector2D ret = gems[0];
    Vector2D pPos = pPlayer ? pPlayer->getPos() : Vector2D(0, 0);
    float best = distance(pPos, ret);
    for (const auto& g : gems)
    {
        float d = distance(pPos, g);
        if (d < best)
        {
            best = d;
            ret = g;
        }
    }
    return ret;
}

float ObjectiveManager::distance(const Vector2D& a, const Vector2D& b) const
{
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

Vector2D ObjectiveManager::moveTowards(const Vector2D& from, const Vector2D& to, float maxDist) const
{
    Vector2D delta = to - from;
    float dist = distance(from, to);
    if (dist <= 0.0001f) return to;
    if (maxDist >= dist) return to;
    float t = maxDist / dist;
    return Vector2D(from.x + delta.x * t, from.y + delta.y * t);
}

SpriteRenderer* ObjectiveManager::findSpriteRenderer(AGameObject* pObj)
{
    if (!pObj) return nullptr;
    auto vec = pObj->getComponentsRecursively(ComponentType::RENDERER, true);
    for (AComponent* c : vec)
    {
        SpriteRenderer* sr = dynamic_cast<SpriteRenderer*>(c);
        if (sr) return sr;
    }
    return nullptr;
}

ButtonInput* ObjectiveManager::findButtonInput(AGameObject* pObj)
{
    if (!pObj) return nullptr;
    auto vec = pObj->getComponentsRecursively(ComponentType::INPUT, true);
    for (AComponent* c : vec)
    {
        ButtonInput* b = dynamic_cast<ButtonInput*>(c);
        if (b) return b;
    }
    return nullptr;
}

/* -------------------
     Event listener
   ------------------- */

void ObjectiveManager::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
    if (!this->bListenerEnabled) return;

    // any button set to "Find Objective" (the button's name doesn't matter)
    if (isPathFinding) return;   //already pointing somewhere

    if (!collectActiveGems().empty())
        findGems();
    else
        findDoor();
}

EventKey ObjectiveManager::getKey()
{
    return this->EKey;
}

bool ObjectiveManager::isListenerEnabled()
{
    return this->bListenerEnabled;
}

void ObjectiveManager::setListenerEnabled(bool b)
{
    this->bListenerEnabled = b;
}

std::string ObjectiveManager::getListenerOwnerName()
{
    return this->strName;
}

// the level was rebuilt (load, undo, Play/Stop): the objects we pointed to are gone,
// perform() finds the new ones
void ObjectiveManager::resetSceneRefs()
{
    pPlayer = nullptr;
    pArrow = nullptr;
    pObjectivePanel = nullptr;
    pOptionalPanel = nullptr;
    bTriedFindPanels = false;
    isPathFinding = false;
    elapsed = 0.0f;
}