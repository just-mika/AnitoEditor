#include "PlatformerPickupGUI.h"
#include "SpriteAnimator.h" 
#include "EventBroadcaster.h" 
#include "Background.h"
#include "TextureManager.h"
#include "GUIToggle.h"
#include "ButtonInput.h"
#include "Text.h"
#include "DataAssetManager.h"

PlatformerPickupGUI::PlatformerPickupGUI(std::string strName):AGameObject(strName)
{
	this->pDataAsset = (CollectableGemDataAsset*)DataAssetManager::getInstance()->getDataAsset("CollectableGemDataAsset");
	this->EKey = EventKey::ITEM_PICKUP;
	this->bIsListenerEnabled = true;
	this->bIsScreenObject = true;
}

PlatformerPickupGUI::~PlatformerPickupGUI()
{
}

void PlatformerPickupGUI::initialize()
{
	// dims the screen; clicking anywhere on it closes the pickup screen
	Background* pTransBack = new Background("Transparent_BG", "Square", Vector2D(1.0f));
	this->attachChild(pTransBack);
	pTransBack->setLocalPos(Vector2D(-11, -24));
	pTransBack->setLocalScale(Vector2D(3.95f, 2.18f));
	SpriteRenderer* pTransBGR = (SpriteRenderer*)pTransBack->findComponentByName("SpriteRenderer");
	pTransBGR->setColor({ 0, 0, 0, 127 });
	pTransBack->attachComponent(new ButtonInput(pTransBGR));
	pTransBack->attachComponent(new GUIToggle(EventKey::ITEM_PICKUP));

	// the player grabbing the gem (plays first, stays on its last frame)
	AnimatedSprite* pAnimatedCharacter = new AnimatedSprite("PickupAnim", "Pickup", Vector2D(0.0f), Vector2D(1.0f), 0.0f, 8);
	this->attachChild(pAnimatedCharacter);
	pAnimatedCharacter->setIsScreenObject(true);
	pAnimatedCharacter->setLocalPos(Vector2D(909, 527));
	pAnimatedCharacter->setLocalScale(Vector2D(0.92f, 0.92f));
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimatedCharacter->findComponentByName("SpriteAnimator");
	pAnimator->getCurrentAnimation()->setType(AnimationType::ONCE);
	pAnimator->getCurrentAnimation()->setOnAnimFinished(OnAnimFinished::FUNC);
	pAnimator->addListener(this);

	// full-frame overlay of the gem in the player's hands; swapped to the picked gem's color
	Sprite* pGem = new Sprite("Gem", "Gem_Colorless_Grab");
	this->attachChild(pGem);
	pGem->setIsScreenObject(true);
	pGem->setLocalPos(Vector2D(909, 527));
	pGem->setLocalScale(Vector2D(0.92f, 0.92f));

	// texts on the gem (scale undoes the gem's 0.92)
	Text* pTitle = new Text("Title", "JainiPurva-Regular.ttf", 75, 0, false);
	pGem->attachChild(pTitle);
	pTitle->setIsScreenObject(true);
	pTitle->setMessage("Character");
	pTitle->setLocalPos(Vector2D(381.52f, -315.22f));
	pTitle->setLocalScale(Vector2D(1 / 0.92f, 1 / 0.92f));
	pTitle->setLocalRot(-14);
	pTitle->setColor(SDL_Color(0, 0, 0, 255));

	Text* pText = new Text("Text", "JainiPurva-Regular.ttf", 45, 0, false);
	pGem->attachChild(pText);
	pText->setIsScreenObject(true);
	pText->setMessage("Gem text");   //replaced by the picked gem's text
	pText->setLocalPos(Vector2D(325, 11.96f));
	pText->setLocalScale(Vector2D(1 / 0.92f, 1 / 0.92f));
	pText->setLocalRot(-14);
	pText->setColor(SDL_Color(0, 0, 0, 255));

	pTransBack->setEnabled(false);
	pAnimatedCharacter->setEnabled(false);
	pGem->setEnabled(false);
	EventBroadcaster::getInstance()->registerListener(this);
}

// shows the picked gem's color (keys like "Gem_Cyan_Grab")
void PlatformerPickupGUI::changeGem(std::string strGemName)
{
	SpriteRenderer* pRenderer = (SpriteRenderer*)this->findChildByName("Gem")->findComponentByName("SpriteRenderer");
	std::vector<SDL_Texture*> vecTex = TextureManager::getInstance()->getTexture(strGemName);
	if (!vecTex.empty())   //a missing key used to crash here
		pRenderer->setTexture(vecTex[0]);
}

void PlatformerPickupGUI::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	// switched off in the level (or inside a disabled group): this screen isn't in play
	if (!this->isGloballyEnabled()) return;

	AGameObject* pTransBack = this->findChildByName("Transparent_BG");
	AGameObject* pGem = this->findChildByName("Gem");
	AnimatedSprite* pAnimSprite = (AnimatedSprite*)this->findChildByName("PickupAnim");
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimSprite->findComponentByName("SpriteAnimator");

	bool bFromToggle = false;
	if (mapParameter.find("Sender") != mapParameter.end())
	{
		std::string senderType = *static_cast<std::string*>(mapParameter["Sender"]);
		bFromToggle = (senderType == "Transparent_BG");
	}

	if (!pAnimSprite->getEnabled())
	{
		// closed: a gem was picked up, play the grab animation (the gem panel appears when it finishes)
		pTransBack->setEnabled(false);
		pGem->setEnabled(false);

		pAnimSprite->setEnabled(true);
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		pAnimator->getCurrentAnimation()->stop();
		pAnimator->getCurrentAnimation()->play();

		if (mapParameter.find("GemName") != mapParameter.end())
		{
			std::string strGemName = *static_cast<std::string*>(mapParameter["GemName"]);
			Text* pTitle = (Text*)pGem->findChildByName("Title");
			Text* pText = (Text*)pGem->findChildByName("Text");
			pTitle->setMessage(this->pDataAsset->getGemTitle(strGemName));
			pText->setMessage(this->pDataAsset->getGemTextBreak(strGemName));
			this->changeGem(strGemName + "_Grab");
		}
	}
	else if (pTransBack->getEnabled() && bFromToggle)
	{
		// open: a click on the dim background closes everything
		pTransBack->setEnabled(false);
		pGem->setEnabled(false);
		pAnimSprite->setEnabled(false);
		EventBroadcaster::getInstance()->enableAllListeners();
	}
}

EventKey PlatformerPickupGUI::getKey()
{
	return this->EKey;
}

bool PlatformerPickupGUI::isListenerEnabled()
{
	return this->bIsListenerEnabled;
}

void PlatformerPickupGUI::setListenerEnabled(bool bListenerEnabled)
{
	this->bIsListenerEnabled = bListenerEnabled;
}

std::string PlatformerPickupGUI::getListenerOwnerName()
{
	return this->strName;
}

void PlatformerPickupGUI::onAnimationFinished()
{
	// the animation stays on its last frame; show the dim background and the gem on top
	this->findChildByName("Transparent_BG")->setEnabled(true);
	this->findChildByName("Gem")->setEnabled(true);
}
