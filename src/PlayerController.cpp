#include "PlayerController.h"
#include "AGameObject.h"
#include "AInteractable.h"
#include "AudioManager.h"
#include "EventBroadcaster.h"
#include "InputManager.h"
#include "GameObjectManager.h"
#include "imgui.h"
#include "InteractButtonController.h"

PlayerController::PlayerController(PlayerInput* pInput, SpriteRenderer* pSprite, SpriteAnimator* pAnimator, RigidBody* pRigidBody)
	: AComponent("PlayerController", ComponentType::SCRIPT)
{
	this->pInput = pInput;
	this->pSprite = pSprite;
	this->pAnimator = pAnimator;
	this->pRigidBody = pRigidBody;
	this->fMoveSpeed = 100.0f;
	this->fJumpForce = 150.f;
	this->pQMark = NULL;

	this->pRigidBody->setListener(this);
}

PlayerController::~PlayerController()
{

}

void PlayerController::onAttach()
{
	this->pQMark = this->pOwner->findChildByName("Q_Mark");
}

void PlayerController::perform()
{
    if (this->pInput == NULL || this->pSprite == NULL) return;

    // a screen (objective intro, pause, story...) is open: drop this frame's
	// movement, jump and interact, so the player stands still behind it
    if (EventBroadcaster::getInstance()->isBlocking())
    {
        this->bMoving = false;
        this->bJumped = false;
        this->pInput->setInteracted(false);
    }

    bool bHasInput = this->bMoving && SDL_fabsf(this->fMoveDirection) > 0.01f;
    this->bMoving = false;

    if (bHasInput)
    {
        // Preserve existing Y velocity (jump/gravity), only override X
        Vector2D currentVel = this->pRigidBody->getVelocity();
        this->pRigidBody->setVelocity(Vector2D(fMoveDirection * this->fMoveSpeed * this->fDeltaTime, currentVel.y));

        this->pSprite->setFlipX(fMoveDirection < 0.0f);
        if (this->pRigidBody->getGrounded()) this->pAnimator->play("run");
    }
    else
    {
        // no movement input this frame: stop sideways, keep falling/jumping speed
        Vector2D currentVel = this->pRigidBody->getVelocity();
        this->pRigidBody->setVelocity(Vector2D(0.0f, currentVel.y));

        if (this->pRigidBody->getGrounded()) this->pAnimator->play("idle");
    }

    if (bJumped)
    {
        this->bJumped = false;
        if (this->pRigidBody->getGrounded())
        {
            Vector2D currentVel = this->pRigidBody->getVelocity();
            this->pRigidBody->addForce(Vector2D(currentVel.x, this->fJumpForce), true);
            this->pAnimator->play("jump");

            AudioManager::getInstance()->play(new AudioPlayer("Jump", AudioGroupTag::SFX));
            AudioManager::getInstance()->play(new AudioPlayer("Land", AudioGroupTag::SFX));
        }
    }

    if (!this->pRigidBody->getGrounded())
    {
        Animation* pCurrent = this->pAnimator->getCurrentAnimation();
        if (pCurrent != nullptr && pCurrent->getName() != "idle")
        {
            this->pAnimator->play("fall");
        }
    }
}

void PlayerController::onCollisionEnter(ACollider* pCollider)
{
    if (AInteractable* pCollectable = dynamic_cast<AInteractable*>(pCollider))
    {
        std::cout << "collectable detected." << std::endl;
        if (this->pQMark) this->pQMark->setEnabled(true);
        this->pInput->setInteracted(false);

        AGameObject* pBtn = GameObjectManager::getInstance()->findObjectByName("InteractBtn");
        if (pBtn)
        {
            pBtn->setEnabled(true);
            InteractButtonController* pCtrl = (InteractButtonController*)pBtn->findComponentByName("InteractButtonController");
            if (pCtrl) pCtrl->setJustEnabled(true);
        }
    }
    else if (pCollider)
    {
        std::cout << pCollider->getOwner()->getName() << std::endl;
    }
}

void PlayerController::onCollisionExit(ACollider* pCollider)
{
    if (AInteractable* pCollectable = dynamic_cast<AInteractable*>(pCollider))
    {
        if (this->pQMark) this->pQMark->setEnabled(false);
        AGameObject* pBtn = GameObjectManager::getInstance()->findObjectByName("InteractBtn");
        if (pBtn) pBtn->setEnabled(false);
    }
}

void PlayerController::onCollisionContinue(ACollider* pCollider)
{
    if (AInteractable* pCollectable = dynamic_cast<AInteractable*>(pCollider))
    {
        if (this->pInput->getInteracted())
        {
            pCollectable->onInteract();
            this->pInput->setInteracted(false); 
        }
    }
}

void PlayerController::Move(float direction)
{
	this->bMoving = true;
    this->fMoveDirection = direction;
}

void PlayerController::Jump()
{
	this->bJumped = true;
}

void PlayerController::setMoveSpeed(float fMoveSpeed)
{
	this->fMoveSpeed = fMoveSpeed;
}

void PlayerController::setJumpForce(float fJumpForce)
{
	this->fJumpForce = fJumpForce;
}

// shown under the PlayerController component in the editor's Inspector
void PlayerController::drawInspector()
{
    ImGui::DragFloat("Move Speed", &this->fMoveSpeed, 5.0f, 0.0f, 2000.0f);
    ImGui::DragFloat("Jump Force", &this->fJumpForce, 1.0f, 0.0f, 1000.0f);
}