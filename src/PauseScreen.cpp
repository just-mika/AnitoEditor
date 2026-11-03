#include "PauseScreen.h"

#include "EmptyObject.h"
#include "Sprite.h"
#include "SpriteAnimator.h"

PauseScreen::PauseScreen(std::string strName): AGameObject(strName)
{
	this->EKey = EventKey::PAUSE_SCREEN;
	this->bListenerEnabled = true;
	this->bIsScreenObject = true;
}
PauseScreen::~PauseScreen()
{
	TextureManager::getInstance()->unload("BrownButton");
	TextureManager::getInstance()->unload("PausePanel");
}

void PauseScreen::initialize()
{
	TextureManager::getInstance()->load("GUI/button5.png", "BrownButton");
	TextureManager::getInstance()->load("GUI/Panel.png", "PausePanel");

	// Menu: everything that appears after the pause animation finishes
	EmptyObject* pMenu = new EmptyObject("Menu");
	this->attachChild(pMenu);
	pMenu->setIsScreenObject(true);

	Background* pTransBack = new Background("TransparentBG", "PausePanel", Vector2D(1.0f));
	pMenu->attachChild(pTransBack);
	pTransBack->setIsScreenObject(true);
	pTransBack->setLocalPos(Vector2D(-1059, -543));
	pTransBack->setLocalScale(Vector2D(0.39f, 0.36f));
	SpriteRenderer* pTransBGR = (SpriteRenderer*)pTransBack->findComponentByName("SpriteRenderer");
	pTransBGR->setPivot(Vector2D(0, 0));
	pTransBGR->setColor({ 255, 255, 255, 179 });

	Sprite* pBanner = new Sprite("MenuBanner", "Square");
	pMenu->attachChild(pBanner);
	pBanner->setIsScreenObject(true);
	pBanner->setLocalPos(Vector2D(-1010, 32));
	pBanner->setLocalScale(Vector2D(1.0f, 3.33f));
	pBanner->setLocalRot(20);
	SpriteRenderer* pBannerR = (SpriteRenderer*)pBanner->findComponentByName("SpriteRenderer");
	pBannerR->setColor({ 76, 0, 0, 255 });

	Text* pPauseText = new Text("PauseText", "JainiPurva-Regular.ttf", 200);
	pMenu->attachChild(pPauseText);
	pPauseText->setIsScreenObject(true);
	pPauseText->setMessage("Paused");
	pPauseText->setLocalPos(Vector2D(-496, -420));
	pPauseText->setLocalScale(Vector2D(0.83f, 0.83f));
	pPauseText->setLocalRot(20);
	pPauseText->setColor(SDL_Color(142, 27, 27, 255));

	GUIButton* pResume = new GUIButton("ResumeButton", "BrownButton");
	pMenu->attachChild(pResume);
	pResume->setIsScreenObject(true);
	pResume->attachComponent(new GUIToggle(EventKey::PAUSE_SCREEN));
	pResume->setLocalPos(Vector2D(-441, -240));
	pResume->setLocalScale(Vector2D(0.18f, 0.15f));
	pResume->setLocalRot(20);

	// text scale = 1 / button scale, so the text isn't squashed by the button
	Text* pResumeText = new Text("ResumeText", "JainiPurva-Regular.ttf", 60);
	pResume->attachChild(pResumeText);
	pResumeText->setIsScreenObject(true);
	pResumeText->setMessage("Resume");
	pResumeText->setLocalPos(Vector2D(-26.1f, -11.4f));
	pResumeText->setLocalScale(Vector2D(1 / 0.18f, 1 / 0.15f));
	pResumeText->setColor(SDL_Color(255, 255, 255, 255));

	GUIButton* pQuit = new GUIButton("QuitButton", "BrownButton");
	pMenu->attachChild(pQuit);
	pQuit->setIsScreenObject(true);
	pQuit->attachComponent(new SceneSwitcher(SceneTag::LOBBY_SCENE));
	pQuit->setLocalPos(Vector2D(-388, -83));
	pQuit->setLocalScale(Vector2D(0.18f, 0.15f));
	pQuit->setLocalRot(20);

	Text* pQuitText = new Text("QuitText", "JainiPurva-Regular.ttf", 60);
	pQuit->attachChild(pQuitText);
	pQuitText->setIsScreenObject(true);
	pQuitText->setMessage("Quit Stage");
	pQuitText->setLocalPos(Vector2D(8.08f, -46.13f));
	pQuitText->setLocalScale(Vector2D(1 / 0.18f, 1 / 0.15f));
	pQuitText->setColor(SDL_Color(255, 255, 255, 255));

	// the animation plays first, then shows the Menu (onAnimationFinished)
	AnimatedSprite* pAnimSprite = new AnimatedSprite("PauseAnim", "Pause", Vector2D(0.0f), Vector2D(1.0f), 0.0f, 12);
	this->attachChild(pAnimSprite);
	pAnimSprite->setIsScreenObject(true);
	pAnimSprite->setLocalPos(Vector2D(-946, -611));
	pAnimSprite->setLocalScale(Vector2D(0.95f, 0.96f));
	SpriteRenderer* pAnimR = (SpriteRenderer*)pAnimSprite->findComponentByName("SpriteRenderer");
	pAnimR->setPivot(Vector2D(0, 0));
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimSprite->findComponentByName("SpriteAnimator");
	pAnimator->getCurrentAnimation()->setType(AnimationType::ONCE);
	pAnimator->getCurrentAnimation()->setOnAnimFinished(OnAnimFinished::FUNC);
	pAnimator->addListener(this);

	pMenu->setEnabled(false);
	pAnimSprite->setEnabled(false);
	EventBroadcaster::getInstance()->registerListener(this);
}

void PauseScreen::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	AGameObject* pMenu = this->findChildByName("Menu");
	AnimatedSprite* pAnimSprite = (AnimatedSprite*)this->findChildByName("PauseAnim");
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimSprite->findComponentByName("SpriteAnimator");

	bool bFromToggle = false;
	if (mapParameter.find("Sender") != mapParameter.end())
	{
		std::string senderType = *static_cast<std::string*>(mapParameter["Sender"]);
		bFromToggle = (senderType == "ResumeButton");
	}

	if (!pMenu->getEnabled())
	{
		// opening: play the animation, the Menu appears when it finishes
		pAnimSprite->setEnabled(true);
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		pAnimator->getCurrentAnimation()->stop();
		pAnimator->getCurrentAnimation()->play();
	}
	else if (bFromToggle)
	{
		// closing from the Resume button
		pMenu->setEnabled(false);
		pAnimSprite->setEnabled(false);
		EventBroadcaster::getInstance()->enableAllListeners();
	}
}

EventKey PauseScreen::getKey()
{
	return this->EKey;
}

bool PauseScreen::isListenerEnabled()
{
	return this->bListenerEnabled;
}

void PauseScreen::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string PauseScreen::getListenerOwnerName()
{
	return this->strName;
}

void PauseScreen::onAnimationFinished()
{
	this->findChildByName("Menu")->setEnabled(true);
}