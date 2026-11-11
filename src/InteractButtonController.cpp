#include "InteractButtonController.h"
#include "GameObjectManager.h"
#include "InputManager.h"
#include "PlayerManager.h"
#include "ButtonInput.h"
#include "AGameObject.h"
#include <vector>
#include <string>
#include <iostream>

#include "EventBroadcaster.h"
#include "Player.h"

static PlayerInput* findPlayerInputSafe()
{
    PlayerManager* pm = PlayerManager::getInstance();
    if (!pm || !pm->getPlayer()) return nullptr;

    AComponent* comp = pm->getPlayer()->findComponentByName("PlayerInput");
    return dynamic_cast<PlayerInput*>(comp);
}

InteractButtonController::InteractButtonController() :AComponent("InteractButtonController", ComponentType::SCRIPT)
{
}

void InteractButtonController::onAttach()
{
    // Try to resolve player input on attach (fast path)
    PlayerInput* p = findPlayerInputSafe();
    if (p)
    {
        // store in owner component's userdata (or a static) — keep simple: store on this instance if header has member
        this->playerInput = p;
    }

    this->pRenderer = (SpriteRenderer*)this->pOwner->findComponentByName("SpriteRenderer");
}

void InteractButtonController::perform()
{
    this->playerInput = findPlayerInputSafe();   //the player can be rebuilt (reload, undo), so look it up each frame
    
    bool bBlocked = EventBroadcaster::getInstance()->isBlocking();
    if (this->pRenderer) this->pRenderer->setColor({ 255, 255, 255, (Uint8)(bBlocked ? 0 : 255) });
    if (bBlocked) return;

	if (!this->playerInput || !this->pRenderer) return;
        if (!this->pOwner->getEnabled()) return;  

        if (this->bJustEnabled)
        {
            this->bJustEnabled = false;
            this->playerInput->setInteracted(false);
            return;
        }

        Uint64 fingerId = 0;
        float x, y = 0.0f;
        bool isAnyFingerInRect = InputManager::getInstance()->isAnyFingerInRect(this->pRenderer->getRect(), fingerId);

        if (isAnyFingerInRect && !this->playerInput->getInteracted())
        {
            this->playerInput->setInteracted(true);
            SDL_Log("Virtual Interacted");
        }
        else if (!isAnyFingerInRect && this->playerInput->getVirtualInteract()) this->playerInput->setVirtualInteract(false);
    
}
