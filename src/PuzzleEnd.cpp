#include "PuzzleEnd.h"
#include "Background.h"
#include "SpriteRenderer.h"
#include "AnimatedSprite.h"
#include "EmptyObject.h"
#include "SpriteAnimator.h"
#include "GUIButton.h"
#include "SceneSwitcher.h"
#include "Sprite.h"
#include "EventBroadcaster.h"
#include "Text.h"

PuzzleEnd::PuzzleEnd(std::string strName, AGameObject* incDragon, AGameObject* compDragon) :AGameObject(strName)
{
	this->EKey = EventKey::END_LEVEL;
	this->bListenerEnabled = true;
	this->bIsScreenObject = true;
	this->incDragon = incDragon;
	this->compDragon = compDragon;
}

PuzzleEnd::~PuzzleEnd()
{
	this->incDragon = NULL;
	this->compDragon = NULL;
}

void PuzzleEnd::initialize()
{
	// Menu: everything that appears after the end animation finishes.
	// Children draw in order, so the dragon comes before the text and the button.
	EmptyObject* pMenu = new EmptyObject("Menu");
	this->attachChild(pMenu);
	pMenu->setIsScreenObject(true);
	pMenu->setLocalPos(Vector2D(-1020, -542));

	Background* pTransBack = new Background("TransparentBG", "Square", Vector2D(1.0f));
	pMenu->attachChild(pTransBack);
	pTransBack->setLocalPos(Vector2D(-8, -33));
	pTransBack->setLocalScale(Vector2D(3.78f, 2.12f));
	SpriteRenderer* pTransBGR = (SpriteRenderer*)pTransBack->findComponentByName("SpriteRenderer");
	pTransBGR->setColor({ 0, 0, 0, 127 });

	// shown by onAnimationFinished (through setDragons)
	Sprite* pCompleteDragon = new Sprite("Complete_Dragon", "Full_Dragon");
	pMenu->attachChild(pCompleteDragon);
	pCompleteDragon->setIsScreenObject(true);
	pCompleteDragon->setLocalPos(Vector2D(934, 542));
	pCompleteDragon->setLocalScale(Vector2D(0.66f, 0.66f));
	pCompleteDragon->setEnabled(false);

	Sprite* pTextHolder = new Sprite("pTextHolder", "Tablet");
	pMenu->attachChild(pTextHolder);
	pTextHolder->setIsScreenObject(true);
	pTextHolder->setLocalPos(Vector2D(944, 202));
	pTextHolder->setLocalScale(Vector2D(0.32f, 0.26f));

	Text* pCompleteText = new Text("Level Complete", "Maragsa.otf", 64);
	pTextHolder->attachChild(pCompleteText);
	pCompleteText->setIsScreenObject(true);
	pCompleteText->setMessage("Level Complete");
	pCompleteText->setLocalScale(Vector2D(1 / 0.32f, 1 / 0.26f));
	pCompleteText->setColor(SDL_Color(0, 0, 0, 255));

	GUIButton* pExit = new GUIButton("Button_Exit", "Close_Button");
	pMenu->attachChild(pExit);
	pExit->setIsScreenObject(true);
	pExit->setLocalPos(Vector2D(944, 952));
	pExit->setLocalScale(Vector2D(0.19f, 0.19f));
	pExit->attachComponent(new SceneSwitcher(SceneTag::LOBBY_SCENE));

	Text* pExitText = new Text("Exit Text", "Maragsa.otf", 349);
	pExit->attachChild(pExitText);
	pExitText->setIsScreenObject(true);
	pExitText->setMessage("Return");
	pExitText->setColor(SDL_Color(139, 32, 32, 255));

	// the animation plays first, then shows the Menu (onAnimationFinished)
	AnimatedSprite* pAnimSprite = new AnimatedSprite("EndAnim", "Puzzle_End", Vector2D(0.0f), Vector2D(1.0f), 0.0f, 12);
	this->attachChild(pAnimSprite);
	pAnimSprite->setIsScreenObject(true);
	pAnimSprite->setLocalPos(Vector2D(-85, 7));
	pAnimSprite->setLocalScale(Vector2D(0.96f, 0.9f));
	SpriteRenderer* pSpriteRenderer = (SpriteRenderer*)pAnimSprite->findComponentByName("SpriteRenderer");
	pSpriteRenderer->setPivot(Vector2D(0.5f, 0.5f));
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimSprite->findComponentByName("SpriteAnimator");
	pAnimator->getCurrentAnimation()->setType(AnimationType::ONCE);
	pAnimator->getCurrentAnimation()->setOnAnimFinished(OnAnimFinished::FUNC);
	pAnimator->addListener(this);

	pMenu->setEnabled(false);
	pAnimSprite->setEnabled(false);
	EventBroadcaster::getInstance()->registerListener(this);
}

void PuzzleEnd::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	// switched off in the level (or inside a disabled group): this screen isn't in play
	if (!this->isGloballyEnabled()) return;

	AGameObject* pMenu = this->findChildByName("Menu");
	AnimatedSprite* pAnimSprite = (AnimatedSprite*)this->findChildByName("EndAnim");
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimSprite->findComponentByName("SpriteAnimator");
	if (!pMenu->getEnabled())
	{
		pAnimSprite->setEnabled(true);
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		pAnimator->getCurrentAnimation()->stop();
		pAnimator->getCurrentAnimation()->play();
	}
}

EventKey PuzzleEnd::getKey()
{
	return this->EKey;
}

bool PuzzleEnd::isListenerEnabled()
{
	return this->bListenerEnabled;
}

void PuzzleEnd::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string PuzzleEnd::getListenerOwnerName()
{
	return this->getName();
}

void PuzzleEnd::onAnimationFinished()
{
	if (this->incDragon) this->incDragon->setEnabled(false);
	if (this->compDragon) this->compDragon->setEnabled(true);
	this->findChildByName("EndAnim")->setEnabled(false);
	this->findChildByName("Menu")->setEnabled(true);
}

