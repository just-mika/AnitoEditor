#include "SequenceGameManager.h"
#include "GameObjectManager.h"
#include "EmptyObject.h"
#include "PuzzleToken.h"
#include "EventBroadcaster.h"
void SequenceGameManager::perform()
{
    // Checks for dropped tokens (occurs when the token had been clicked and dragged).
    for (auto pToken : this->vecToken)
    {
        if (pToken->getDropped())
        {
            pToken->setDropped(false);
            int nearPosIndex = this->getSlotIndex(pToken);

            // Reassign the slot of the other token if the slot is occupied.
            if (this->vecTokenHolder[nearPosIndex] != NULL)
            {
                PuzzleToken* pOther = this->vecTokenHolder[nearPosIndex];
                this->assignTokenSlot(pOther, pToken->getSlot());
            }
            // Move the token to the slot.
            this->clearSlot(pToken->getSlot());
            this->assignTokenSlot(pToken, nearPosIndex);
        }
    }
}

void SequenceGameManager::addToken(PuzzleToken* pToken)
{
    if (this->vecPosition.empty())
    {
        SDL_Log("[SequenceGame] no slots configured, token '%s' ignored", pToken->getName().c_str());
        return;
    }

    this->vecToken.push_back(pToken);

    int nSlot = this->getSlotIndex(pToken);
    if (this->vecTokenHolder[nSlot] != nullptr)
        for (size_t i = 0; i < this->nStartSlots; i++)
            if (this->vecTokenHolder[i] == nullptr) { nSlot = (int)i; break; }

    this->assignTokenSlot(pToken, nSlot);
}

void SequenceGameManager::removeToken(PuzzleToken* pToken)
{
    int nIndex = -1;

    for (int i = 0; i < this->vecToken.size() && nIndex == -1; i++)
    {
        if (this->vecToken[i] == pToken)
            nIndex = i;
    }

    if (nIndex != -1)
    {
        this->vecToken.erase(this->vecToken.begin() + nIndex);
        delete pToken;
    }
}

void SequenceGameManager::setCombination(const std::vector<std::string>& combo)
{
    for (size_t i = 0; i < this->vecCombination.size(); i++)
        this->vecCombination[i] = (i < combo.size()) ? combo[i] : "";
}

// the old 3-argument version, so the hardcoded scene still compiles
void SequenceGameManager::setCombination(std::string strFirst, std::string strSecond, std::string strThird)
{
    this->setCombination(std::vector<std::string>{ strFirst, strSecond, strThird });
}

// Check the token's slot by getting the nearest slot position.
int SequenceGameManager::getSlotIndex(PuzzleToken* pToken)
{
    float minSqrMag = 10000000.0f;
    int nearPosIndex = 0;

    for (int i = 0; i < this->vecPosition.size(); i++)
    {
        float sqrMag = (pToken->getPos() - this->vecPosition[i]).SqrMagnitude();
        if (sqrMag < minSqrMag)
        {
            minSqrMag = sqrMag;
            nearPosIndex = i;
        }
    }

    return nearPosIndex;
}

void SequenceGameManager::assignTokenSlot(PuzzleToken* pToken, int nSlot)
{
    pToken->setSlot(nSlot);
    pToken->setPos(this->vecPosition[nSlot]);
    this->vecTokenHolder[nSlot] = pToken;
    this->checkToken(nSlot,pToken->getName());
    this->checkCombination();
}

void SequenceGameManager::checkToken(int nSlot, const std::string& strTokenName)
{
    if (nSlot < (int)this->nStartSlots) return;
    const size_t i = nSlot - this->nStartSlots;
    if (i < this->vecCombinationCheck.size())
        this->vecCombinationCheck[i] = (this->vecCombination[i] == strTokenName);
}

void SequenceGameManager::checkCombination()
{
    if (this->vecCombinationCheck.empty()) return;
    bool bEndCheck = true;
    for (bool bCheck : this->vecCombinationCheck)
    {
        if (!bCheck)
        {
            bEndCheck = false;
            break;
        }
    }
    if (bEndCheck)
    {
        for (PuzzleToken* pToken : this->vecToken)
        {
            pToken->setEnabled(false);
        }
        EventBroadcaster::getInstance()->broadcast(EventKey::END_LEVEL);
    }
}

SequenceGameManager* SequenceGameManager::P_SHARED_INSTANCE = NULL;

SequenceGameManager::SequenceGameManager() : AComponent("SequenceGameManager", ComponentType::SCRIPT)
{
    // default = the original Puzzle 1 layout
    this->configure(
        { Vector2D(-280.0f, -375.0f), Vector2D(0.0f, -375.0f), Vector2D(280.0f, -375.0f) },
        { Vector2D(-380.0f, 27.0f),   Vector2D(0.0f, 27.0f),   Vector2D(380.0f, 27.0f) });
}

void SequenceGameManager::initialize()
{
    EmptyObject* pManagerObject = new EmptyObject("SequenceGameManager");
    pManagerObject->setIsSystemObject(true);
    P_SHARED_INSTANCE = new SequenceGameManager();
    pManagerObject->attachComponent(P_SHARED_INSTANCE);
    GameObjectManager::getInstance()->addObject(pManagerObject);
}

void SequenceGameManager::destroy()
{
    delete P_SHARED_INSTANCE->pOwner;
}

SequenceGameManager* SequenceGameManager::getInstance()
{
    return P_SHARED_INSTANCE;
}

void SequenceGameManager::configure(const std::vector<Vector2D>& start, const std::vector<Vector2D>& target)
{
    this->vecPosition = start;
    this->vecPosition.insert(this->vecPosition.end(), target.begin(), target.end());
    this->nStartSlots = start.size();

    this->vecTokenHolder.assign(this->vecPosition.size(), nullptr);
    this->vecCombinationCheck.assign(target.size(), false);
    this->vecCombination.assign(target.size(), "");
    this->vecToken.clear();
}


void SequenceGameManager::clearSlot(int nSlot)
{
    this->vecTokenHolder[nSlot] = nullptr;
    if (nSlot >= (int)this->nStartSlots)
        this->vecCombinationCheck[nSlot - this->nStartSlots] = false;
}