#include "GemController.h"
#include "AGameObject.h"
#include "GemInputToken.h"
#include "EventBroadcaster.h"

GemController::GemController(ButtonInput* pInput) :AComponent("GemController",ComponentType::SCRIPT)
{
	this->pInput = pInput;
	this->bPickedup = false;
	this->holdingScale = Vector2D(0.2f);
}

void GemController::onAttach()
{
	this->initialScale = this->pOwner->getScale();
}

void GemController::perform()
{
	if (this->pInput->getDragging())
	{
		Vector2D mouse = this->pInput->getMousePos();

		// the press just started: remember where
		if (!this->bPickedup)
		{
			this->bPickedup = true;
			this->bMoved = false;
			this->pressPos = mouse;
		}

		// only a drag once the pointer moved far enough, so a tap doesn't move or shrink the gem
		if (!this->bMoved && (mouse - this->pressPos).SqrMagnitude() > kTapRadius * kTapRadius)
			this->bMoved = true;

		if (this->bMoved)
		{
			this->pOwner->setPos(mouse);
			this->pOwner->setScale(this->holdingScale);
		}
	}
	// the press ended: either we saw it held, or down and up landed in the same frame
	else if (this->bPickedup || this->pInput->getClicked())
	{
		bool bDrag = this->bPickedup && this->bMoved;
		this->bPickedup = false;

		if (bDrag)
		{
			// a drag: GemInputManager checks which row it was dropped on
			((GemInputToken*)this->pOwner)->setDropped(true);
			this->pOwner->setScale(this->initialScale);
		}
		else
		{
			// a tap: show this gem's text (mouse and touch)
			std::unordered_map <std::string, void*> mapParam;
			std::string strName = pOwner->getName();
			mapParam["GemName"] = static_cast<void*>(&strName);
			EventBroadcaster::getInstance()->broadcast(EventKey::GEM_INPUT_PREVIEW, mapParam);
		}
	}

	// clicks are decided above; clear the flags so they don't carry over
	this->pInput->setClicked(false);
	this->pInput->setLeftClick(false);
	this->pInput->setRightClick(false);
}
