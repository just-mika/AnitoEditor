#include "TokenController.h"
#include "PuzzleToken.h"

TokenController::TokenController(ButtonInput* pInput) : AComponent("TokenController", ComponentType::SCRIPT)
{
	this->pInput = pInput;
	this->bPickedup = false;
	this->holdingScale = Vector2D(0.05f);
}

void TokenController::onAttach()
{
	this->initialScale = this->pOwner->getScale();
}

void TokenController::perform()
{
	if (this->pInput->getDragging())
	{
		Vector2D mouse = this->pInput->getMouseWorldPos();

		// the press just started: remember where
		if (!this->bPickedup)
		{
			this->bPickedup = true;
			this->bMoved = false;
			this->pressPos = mouse;
			this->bIgnorePress = EventBroadcaster::getInstance()->isBlocking();
		}

		if (this->bIgnorePress)
			return;

		// only count it as a drag once the pointer has moved far enough,
		// so a tap doesn't move or shrink the token
		if (!this->bMoved && (mouse - this->pressPos).SqrMagnitude() > kTapRadius * kTapRadius)
			this->bMoved = true;

		if (this->bMoved)
		{
			this->pOwner->setPos(mouse);
			this->pOwner->setScale(this->holdingScale);
		}
	}
	// the press ended: either we saw it held (bPickedup), or it was so quick
// that down and up happened in the same frame (only the click flag is left)
	else if (this->bPickedup || this->pInput->getClicked())
	{
		bool bIgnore = this->bPickedup ? this->bIgnorePress : EventBroadcaster::getInstance()->isBlocking();
		bool bDrag = this->bPickedup && this->bMoved;
		this->bPickedup = false;

		// a press that started under an open screen (e.g. Dismiss over the token) does nothing
		if (!bIgnore && bDrag)
		{
			// a drag: let SequenceGameManager snap it to a slot
			((PuzzleToken*)this->pOwner)->setDropped(true);
			this->pOwner->setScale(this->initialScale);
		}
		else if (!bIgnore)
		{
			// a tap: show this token's hint
			std::unordered_map<std::string, void*> mapParam;
			mapParam["Token"] = static_cast<void*>(this->pOwner);
			EventBroadcaster::getInstance()->broadcast(EventKey::RIGHT_CLICK, mapParam);
		}
	}

	// clicks are decided above, so clear the input's flags so they don't carry over
	this->pInput->setClicked(false);
	this->pInput->setLeftClick(false);
	this->pInput->setRightClick(false);
}