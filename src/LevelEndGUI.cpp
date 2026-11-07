#include "LevelEndGUI.h"
#include "Background.h"
#include "AnimatedSprite.h"
#include "SpriteRenderer.h"
#include "SpriteAnimator.h"
#include "EventBroadcaster.h"
#include "SceneSwitcher.h"
#include "GUIButton.h"
#include "Sprite.h"
#include "Text.h"
#include "Settings.h"
LevelEndGUI::LevelEndGUI(std::string strName):AGameObject(strName)
{
	this->EKey = EventKey::END_LEVEL;
	this->bListenerEnabled = true;
	this->bIsScreenObject = true;
}

LevelEndGUI::~LevelEndGUI()
{

}

void LevelEndGUI::initialize()
{
	Background* pTransBack = new Background("TransparentBG", "Square", Vector2D(1.0f));
	this->attachChild(pTransBack);
	pTransBack->setLocalPos(Vector2D(0, 0));
	pTransBack->setLocalScale(Vector2D(5.0f, 1.0f));
	SpriteRenderer* pTransBGR = (SpriteRenderer*)pTransBack->findComponentByName("SpriteRenderer");
	pTransBGR->setColor({ 255, 255, 255, 0 });

	// plays first, stays on its last frame behind the panel
	AnimatedSprite* pAnimSprite = new AnimatedSprite("EndAnim", "End_Level", Vector2D(0.0f), Vector2D(1.0f), 0.0f, 12);
	this->attachChild(pAnimSprite);
	pAnimSprite->setIsScreenObject(true);
	pAnimSprite->setLocalPos(Vector2D(-47, -9));
	pAnimSprite->setLocalRot(4.66f);
	SpriteRenderer* pSpriteRenderer = (SpriteRenderer*)pAnimSprite->findComponentByName("SpriteRenderer");
	pSpriteRenderer->setPivot(Vector2D(0, 0));
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimSprite->findComponentByName("SpriteAnimator");
	pAnimator->getCurrentAnimation()->setType(AnimationType::ONCE);
	pAnimator->getCurrentAnimation()->setOnAnimFinished(OnAnimFinished::FUNC);
	pAnimator->addListener(this);

	GUIButton* pExit = new GUIButton("Button_Exit", "Exit");
	this->attachChild(pExit);
	pExit->setIsScreenObject(true);
	pExit->setLocalPos(Vector2D(1450, 801));
	pExit->setLocalScale(Vector2D(0.2f, 0.24f));
	pExit->attachComponent(new SceneSwitcher(SceneTag::LOBBY_SCENE));

	Text* pExitText = new Text("ExitText", "JainiPurva-Regular.ttf", 40, 0.f, false);
	pExit->attachChild(pExitText);
	pExitText->setIsScreenObject(true);
	pExitText->setMessage("Return to Hub");
	pExitText->setLocalPos(Vector2D(0, -16.67f));
	pExitText->setLocalScale(Vector2D(8.0f, 6.958f));
	pExitText->setColor(SDL_Color(174, 109, 109, 255));

	Sprite* pTextHolder = new Sprite("pTextHolder", "Tablet");
	this->attachChild(pTextHolder);
	pTextHolder->setIsScreenObject(true);
	pTextHolder->setLocalPos(Vector2D(1456, 351));
	pTextHolder->setLocalScale(Vector2D(0.42f, 0.42f));

	Text* pClearText = new Text("ClearText", "JainiPurva-Regular.ttf", 120, 0.f, false);
	pTextHolder->attachChild(pClearText);
	pClearText->setIsScreenObject(true);
	pClearText->setMessage("Cleared!");
	pClearText->setLocalPos(Vector2D(0, -28.57f));
	pClearText->setLocalScale(Vector2D(2.381f, 2.5f));
	pClearText->setColor(colorWhite);

	pTransBack->setEnabled(false);
	pAnimSprite->setEnabled(false);
	pExit->setEnabled(false);
	pTextHolder->setEnabled(false);
	EventBroadcaster::getInstance()->registerListener(this);
}

void LevelEndGUI::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	// switched off in the level (or inside a disabled group): this screen isn't in play
	if (!this->isGloballyEnabled()) return;

	AnimatedSprite* pAnimSprite = (AnimatedSprite*)this->findChildByName("EndAnim");
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimSprite->findComponentByName("SpriteAnimator");

	// open = the animation is showing (it stays on its last frame)
	if (!pAnimSprite->getEnabled())
	{
		// start clean whatever the level saved: only the animation shows until it ends
		this->findChildByName("TransparentBG")->setEnabled(false);
		this->findChildByName("Button_Exit")->setEnabled(false);
		this->findChildByName("pTextHolder")->setEnabled(false);

		pAnimSprite->setEnabled(true);
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		pAnimator->getCurrentAnimation()->stop();
		pAnimator->getCurrentAnimation()->play();
	}
}

EventKey LevelEndGUI::getKey()
{
	return this->EKey;
}

bool LevelEndGUI::isListenerEnabled()
{
	return this->bListenerEnabled;
}

void LevelEndGUI::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string LevelEndGUI::getListenerOwnerName()
{
	return this->strName;
}

void LevelEndGUI::onAnimationFinished()
{
	// show the panel on top of the last frame
	this->findChildByName("TransparentBG")->setEnabled(true);
	this->findChildByName("Button_Exit")->setEnabled(true);
	this->findChildByName("pTextHolder")->setEnabled(true);
}
