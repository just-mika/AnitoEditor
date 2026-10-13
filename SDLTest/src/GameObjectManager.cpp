// ---------------------------------------------------------------------------
// Responsibilities: manage and iterate over game objects for input, update, draw.
// ---------------------------------------------------------------------------

#include "GameObjectManager.h"
#include <algorithm>
#include <cassert>
#include <iostream>

// ---------------------------------------------------------------------------
// Per-frame lifecycle
// ---------------------------------------------------------------------------

// processInput: forwards a single SDL_Event to every enabled object.
// Complexity: O(G) per call.
void GameObjectManager::processInput(SDL_Event* pEvent)
{
    for (AGameObject* pObject : vecGameObject)
    {
        if (pObject->getEnabled())
            pObject->processInput(pEvent);
    }
}

// update: advances every enabled object by fDeltaTime.
// Complexity: O(G) when each object's update is O(1).
void GameObjectManager::update(float fDeltaTime)
{
    for (AGameObject* pObject : vecGameObject)
    {
        if (pObject->getEnabled())
            pObject->update(fDeltaTime);
    }
}

// draw: submits draw calls for every enabled object.
// Complexity: O(G).
void GameObjectManager::draw(SDL_Renderer* pRenderer)
{
    for (AGameObject* pObject : vecGameObject)
    {
        if (pObject->getEnabled())
            pObject->draw(pRenderer);
    }
}

// cleanUpDeletedObjects: flush the deferred-deletion queue.
// Call once per frame after processInput / update / draw so that no active
// traversal is in flight when we mutate vecGameObject.
// Complexity: O(D * G) where D = objects to delete.
// For large D (e.g. scene teardown) prefer deleteAllObjects() instead.
void GameObjectManager::cleanUpDeletedObjects()
{
    // Swap-clear so that deletions triggered inside a destructor that call
    // deleteObject() again queue into vecPendingDeletion without disturbing
    // the list we are currently iterating.
    std::vector<AGameObject*> objectsToDelete;
    objectsToDelete.swap(vecPendingDeletion);

    for (AGameObject* pObject : objectsToDelete)
    {
        if (!pObject) continue;

        // Remove from the ordered vector (O(G) erase).
        auto vecIt = std::find(vecGameObject.begin(), vecGameObject.end(), pObject);
        if (vecIt != vecGameObject.end())
            vecGameObject.erase(vecIt);

        // Remove from the name map (O(1) average).
        mapGameObject.erase(pObject->getName());

        delete pObject;
        // Note: pObject is a local copy; setting it to nullptr here would be
        // a no-op on the original pointer, so we omit the misleading line.
    }
}

// ---------------------------------------------------------------------------
// Object management
// ---------------------------------------------------------------------------

// addObject: take ownership of pGameObject, register it, call initialize().
// Complexity: amortized O(1) for vector push + O(1) average for map insert.
void GameObjectManager::addObject(AGameObject* pGameObject)
{
    assert(pGameObject != nullptr && "addObject: received a null pointer");

    vecGameObject.push_back(pGameObject);
    mapGameObject[pGameObject->getName()] = pGameObject;
    pGameObject->initialize();
}

// deleteObject: schedule pGameObject for deferred deletion.
// Safe to call during traversal; the object is not freed until
// cleanUpDeletedObjects() runs. Duplicate schedules are ignored.
// Complexity: O(D) for the duplicate check where D = pending queue size.
void GameObjectManager::deleteObject(AGameObject* pGameObject)
{
    if (!pGameObject) return;

    bool alreadyPending = std::find(vecPendingDeletion.begin(),
                                    vecPendingDeletion.end(),
                                    pGameObject) != vecPendingDeletion.end();
    if (!alreadyPending)
        vecPendingDeletion.push_back(pGameObject);
}

// deleteObjectByName: look up then schedule for deletion.
// Complexity: O(1) average map lookup + O(D) pending-queue check.
void GameObjectManager::deleteObjectByName(const std::string& strName)
{
    AGameObject* pObject = findObjectByName(strName);
    if (pObject)
        deleteObject(pObject);
}

// deleteAllObjects: immediately free every object and clear all containers.
// Bypasses the deferred queue for efficiency � do not call during traversal.
// Complexity: O(G).
void GameObjectManager::deleteAllObjects()
{
    // Drain any pending deletions first to avoid double-deletes.
    vecPendingDeletion.clear();

    for (AGameObject* pObject : vecGameObject)
        deleteObject(pObject);

    vecGameObject.clear();
    mapGameObject.clear();
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

// findObjectByName: safe map lookup that never inserts phantom entries.
// Complexity: O(1) average (unordered_map).
AGameObject* GameObjectManager::findObjectByName(const std::string& strName) const
{
    auto it = mapGameObject.find(strName);
    if (it != mapGameObject.end())
        return it->second;

    std::cout << "[ERROR] GameObjectManager::findObjectByName � \""
              << strName << "\" not found.\n";
    return nullptr;
}

// setObjectName: rename an existing object in both containers.
// Complexity: O(1) average.
void GameObjectManager::setObjectName(const std::string& strName,
                                      const std::string& strNewName)
{
    auto it = mapGameObject.find(strName);
    if (it == mapGameObject.end())
    {
        std::cout << "[ERROR] GameObjectManager::setObjectName � \""
                  << strName << "\" not found.\n";
        return;
    }

    AGameObject* pObject = it->second;
    pObject->setName(strNewName);

    // Re-key in the map without a redundant lookup.
    mapGameObject.erase(it);
    mapGameObject[strNewName] = pObject;
}

// getAllObjects: O(1) reference return; callers bear the O(G) traversal cost.
std::vector<AGameObject*>& GameObjectManager::getAllObjects()
{
    return vecGameObject;
}

// ---------------------------------------------------------------------------
// Hierarchy editing
// ---------------------------------------------------------------------------

int GameObjectManager::getSiblingIndex(AGameObject* pObject) const
{
    if (!pObject) return -1;

    if (AGameObject* pParent = pObject->getParent())
        return pParent->getChildIndex(pObject);

    auto it = std::find(vecGameObject.begin(), vecGameObject.end(), pObject);
    return (it != vecGameObject.end()) ? (int)(it - vecGameObject.begin()) : -1;
}

bool GameObjectManager::canMoveObject(AGameObject* pObject, AGameObject* pNewParent) const
{
    if (!pObject) return false;

    if (pNewParent)
    {
        // Can't drop an object into itself or into one of its own descendants.
        if (pNewParent == pObject || pNewParent->isDescendantOf(pObject)) return false;

        // AGameObject::attachChild forces children to inherit the parent's screen flag,
        // so refuse to silently flip it here.
        if (pNewParent->getIsScreenObject() != pObject->getIsScreenObject()) return false;
    }
    return true;
}

void GameObjectManager::moveObject(AGameObject* pObject, AGameObject* pNewParent, int nIndex)
{
    if (!canMoveObject(pObject, pNewParent)) return;

    AGameObject* pOldParent = pObject->getParent();

    // Remember the world transform so the object doesn't visually jump when its parent changes.
    Vector2D worldPos   = pObject->getPos();
    Vector2D worldScale = pObject->getScale();
    float    worldRot   = pObject->getRot();

    // 1) Remove from the old container.
    int nOldIndex = this->getSiblingIndex(pObject);
    if (nOldIndex < 0) return;

    if (pOldParent)
        pOldParent->detachChild(pObject);
    else
        vecGameObject.erase(vecGameObject.begin() + nOldIndex);

    // Removing an earlier element shifts everything after it down by one.
    if (pOldParent == pNewParent && nOldIndex < nIndex)
        nIndex--;

    // 2) Insert into the new container.
    if (pNewParent)
    {
        pNewParent->insertChildAt(pObject, nIndex);
    }
    else
    {
        nIndex = std::max(0, std::min(nIndex, (int)vecGameObject.size()));
        pObject->setParent(nullptr);
        vecGameObject.insert(vecGameObject.begin() + nIndex, pObject);
    }

    // 3) Restore world transform relative to the (possibly new) parent.
    pObject->setPos(worldPos);
    pObject->setScale(worldScale);
    pObject->setRot(worldRot);

    // 4) Only roots live in the name map. A child is owned/deleted by its parent, so a
    //    lingering map entry would dangle when the parent is destroyed.
    if (pNewParent)
    {
        auto it = mapGameObject.find(pObject->getName());
        if (it != mapGameObject.end() && it->second == pObject)
            mapGameObject.erase(it);
    }
    else
    {
        mapGameObject.emplace(pObject->getName(), pObject);
    }
}

bool GameObjectManager::reparentObject(AGameObject* pObject, AGameObject* pNewParent)
{
    if (!pObject || pObject == pNewParent || pObject->getParent() == pNewParent)
        return false;

    if (pNewParent)
    {
        // Screen-space and world-space objects can't be mixed
        if (pNewParent->getIsScreenObject() != pObject->getIsScreenObject())
            return false;

        // Can't parent an object under its own descendant (cycle)
        for (AGameObject* pDesc : pObject->getChildrenRecursively())
            if (pDesc == pNewParent) return false;
    }

    // Keep the object visually in place
    Vector2D pos = pObject->getPos();
    Vector2D scale = pObject->getScale();
    float rot = pObject->getRot();

    // Remove from the old home
    if (AGameObject* pOld = pObject->getParent())
        pOld->detachChild(pObject);
    else
    {
        auto it = std::find(vecGameObject.begin(), vecGameObject.end(), pObject);
        if (it != vecGameObject.end()) vecGameObject.erase(it);
    }

    // Add to the new home
    if (pNewParent)
        pNewParent->adoptChild(pObject);
    else
    {
        pObject->setParent(nullptr);
        vecGameObject.push_back(pObject);
    }

    pObject->setPos(pos);
    pObject->setScale(scale);
    pObject->setRot(rot);
    return true;
}

// ---------------------------------------------------------------------------
// Destructor
// ---------------------------------------------------------------------------

GameObjectManager::~GameObjectManager()
{
    for (AGameObject* pObject : vecGameObject)
        delete pObject;
}

/* * * * * * * * * * * * * * * * * * * * *
 *       SINGLETON-RELATED CONTENT       *
 * * * * * * * * * * * * * * * * * * * * */
GameObjectManager* GameObjectManager::P_SHARED_INSTANCE = nullptr;

void GameObjectManager::initialize()
{
    P_SHARED_INSTANCE = new GameObjectManager();
}

void GameObjectManager::destroy()
{
    delete P_SHARED_INSTANCE;
    P_SHARED_INSTANCE = nullptr; // Guard against use-after-free.
}

GameObjectManager* GameObjectManager::getInstance()
{
    return P_SHARED_INSTANCE;
}
/* * * * * * * * * * * * * * * * * * * * */
