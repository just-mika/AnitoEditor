#include "PuzzleHint.h"

#include "GameObjectManager.h"
#include "PuzzleToken.h"

PuzzleHint::PuzzleHint(std::string strName) :AGameObject(strName)
{
	this->EKey = EventKey::RIGHT_CLICK;
	this->pText = NULL;
	this->bListenerEnabled = true;
	this->bIsScreenObject = true;   //drawn on the canvas, like the other screens
}

PuzzleHint::~PuzzleHint()
{
	TextureManager::getInstance()->unload("HintPage");
}

// Layout copied from the editor (all values are LOCAL, same as the save file)
void PuzzleHint::initialize()
{
	TextureManager::getInstance()->load("sequence_game/Popup_Panel.png", "HintPage");

	// dims the board behind the hint (drawn first, so everything else is on top)
	Sprite* pTransBG = new Sprite("Transparent_BG", "Square");
	this->attachChild(pTransBG);
	pTransBG->setIsScreenObject(true);
	pTransBG->setLocalPos(Vector2D(-86, 0));   //centered on the canvas (hint sits at 1046, 540)
	pTransBG->setLocalScale(Vector2D(4.04f, 2.17f));
	SpriteRenderer* pTransBGR = (SpriteRenderer*)pTransBG->findComponentByName("SpriteRenderer");
	pTransBGR->setColor({ 0, 0, 0, 127 });

	Sprite* pPage = new Sprite("HintPage", "HintPage");
	this->attachChild(pPage);
	pPage->setIsScreenObject(true);
	pPage->setLocalPos(Vector2D(-96, 10));
	pPage->setLocalScale(Vector2D(0.97f, 0.97f));

	this->pText = new Text("TextHint", "Maragsa.otf", 90);
	pPage->attachChild(this->pText);
	this->pText->setIsScreenObject(true);
	this->pText->setMessage("Placeholder");
	this->pText->setLocalPos(Vector2D(0, -11.34f));
	this->pText->setLocalScale(Vector2D(0.7732f, 0.7732f));
	this->pText->setColor(SDL_Color(138, 75, 75, 255));

	GUIButton* pButton = new GUIButton("CloseButton", "Close_Button");
	this->attachChild(pButton);
	pButton->setIsScreenObject(true);
	pButton->attachComponent(new GUIToggle(EventKey::RIGHT_CLICK));
	pButton->setLocalPos(Vector2D(-96, 360));
	pButton->setLocalScale(Vector2D(0.15f, 0.15f));

	Text* pDeclineText = new Text("Decline_Text", "JainiPurva-Regular.ttf", 90);
	pButton->attachChild(pDeclineText);
	pDeclineText->setIsScreenObject(true);
	pDeclineText->setMessage("Close");
	pDeclineText->setLocalScale(Vector2D(5.0f, 5.0f));
	pDeclineText->setColor(SDL_Color(157, 107, 97, 255));

	EventBroadcaster::getInstance()->registerListener(this);
}

void PuzzleHint::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	// inside a disabled group: not in play (its own enabled flag just means open/closed)
	if (this->getParent() && !this->getParent()->isGloballyEnabled()) return;

	// a token was tapped: show its hint (also replaces the text if already open)
	if (mapParameter.find("Token") != mapParameter.end())
	{
		PuzzleToken* t = static_cast<PuzzleToken*>(mapParameter["Token"]);
		if (t && !t->getHint().empty())
		{
			this->pText->setMessage(t->getHint());
			this->setEnabled(true);
			EventBroadcaster::getInstance()->disableOtherListenerExcept(this);   //block the board while open
		}
		return;
	}

	// the Close button: hide
	if (mapParameter.find("Sender") != mapParameter.end())
	{
		std::string senderType = *static_cast<std::string*>(mapParameter["Sender"]);
		if (senderType == "CloseButton")
		{
			this->setEnabled(false);
			EventBroadcaster::getInstance()->enableAllListeners();
		}
	}
}

EventKey PuzzleHint::getKey()
{
	return this->EKey;
}

bool PuzzleHint::isListenerEnabled()
{
	return this->bListenerEnabled;
}

void PuzzleHint::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string PuzzleHint::getListenerOwnerName()
{
	return this->strName;
}
