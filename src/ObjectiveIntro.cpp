#include "ObjectiveIntro.h"

ObjectiveIntro::ObjectiveIntro(std::string strName, std::string strMessage):AGameObject(strName)
{
	this->strMessage = strMessage;
	this->EKey = EventKey::OBJECTIVE_SCREEN;
	this->bListenerEnabled = true;
	this->bIsScreenObject = true;
}

ObjectiveIntro::~ObjectiveIntro()
{
}

void ObjectiveIntro::initialize()
{
	Background* pTransBG = new Background("Transparent_BG", "Square", Vector2D(1.0f));
	this->attachChild(pTransBG);
	pTransBG->setIsScreenObject(true);
	pTransBG->setLocalPos(Vector2D(-1057, -564));
	pTransBG->setLocalScale(Vector2D(3.95f, 2.18f));
	SpriteRenderer* pTransBGR = (SpriteRenderer*)pTransBG->findComponentByName("SpriteRenderer");
	pTransBGR->setPivot(Vector2D(0, 0));
	pTransBGR->setColor({ 0, 0, 0, 127 });

	Sprite* pTablet = new Sprite("Tablet", "Tablet");
	this->attachChild(pTablet);
	pTablet->setIsScreenObject(true);
	pTablet->setLocalPos(Vector2D(-96, -75));
	pTablet->setLocalScale(Vector2D(0.99f, 0.78f));

	// text scale.y = 1 / tablet scale.y, so the text isn't squashed
	Text* pTabletText = new Text("Tablet_Text", "JainiPurva-Regular.ttf", 90);
	pTablet->attachChild(pTabletText);
	pTabletText->setIsScreenObject(true);
	pTabletText->setMessage(this->strMessage);
	pTabletText->setLocalPos(Vector2D(-496, 2.38f));
	pTabletText->setLocalScale(Vector2D(1.0f, 1 / 0.84f));
	pTabletText->setColor(SDL_Color(0, 0, 0, 255));
	TextRenderer* pTabletTextR = (TextRenderer*)pTabletText->findComponentByName("TextRenderer");
	pTabletTextR->setPivot(Vector2D(0.0f, 0.5f));   // left-aligned

	AnimatedSprite* pIntro = new AnimatedSprite("Intro", "Intro", Vector2D(0.0f), Vector2D(1.0f), 0.0f, 12);
	this->attachChild(pIntro);
	pIntro->setIsScreenObject(true);
	pIntro->setLocalPos(Vector2D(-909, -566));
	pIntro->setLocalScale(Vector2D(0.92f, 0.92f));
	SpriteRenderer* pIntroR = (SpriteRenderer*)pIntro->findComponentByName("SpriteRenderer");
	pIntroR->setPivot(Vector2D(0, 0));
	SpriteAnimator* pAnimator = (SpriteAnimator*)pIntro->findComponentByName("SpriteAnimator");
	pAnimator->getCurrentAnimation()->setType(AnimationType::ONCE);

	GUIButton* pClose = new GUIButton("Close", "Close_Button");
	this->attachChild(pClose);
	pClose->setIsScreenObject(true);
	pClose->attachComponent(new GUIToggle(EventKey::OBJECTIVE_SCREEN));
	pClose->setLocalPos(Vector2D(-96, 410));
	pClose->setLocalScale(Vector2D(0.21f, 0.18f));

	Text* pCloseText = new Text("CloseText", "JainiPurva-Regular.ttf", 70);
	pClose->attachChild(pCloseText);
	pCloseText->setIsScreenObject(true);
	pCloseText->setMessage("Dismiss");
	pCloseText->setLocalPos(Vector2D(0, -33.33f));
	pCloseText->setLocalScale(Vector2D(1 / 0.21f, 1 / 0.18f));
	pCloseText->setColor(SDL_Color(157, 107, 97, 255));

	EventBroadcaster::getInstance()->registerListener(this);
	EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
}

void ObjectiveIntro::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	if (this->bEnabled)
	{
		this->setEnabled(false);
		std::cout << "Closing" << std::endl;
		EventBroadcaster::getInstance()->enableAllListeners();
	}



}

EventKey ObjectiveIntro::getKey()
{
	return this->EKey;
}

bool ObjectiveIntro::isListenerEnabled()
{
	return this->bListenerEnabled;
}

void ObjectiveIntro::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string ObjectiveIntro::getListenerOwnerName()
{
	return this->strName;
}

// hidden (dismissed, or disabled in the level): give the other screens their input back
void ObjectiveIntro::onDisable()
{
	EventBroadcaster::getInstance()->enableAllListeners();
}

// the intro blocked at creation, but listeners created after it weren't affected;
// the scene calls this once everything (including scene-made listeners) exists
void ObjectiveIntro::blockOthers()
{
	EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
}