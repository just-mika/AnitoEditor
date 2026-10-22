#include "PuzzleHint.h"

PuzzleHint::PuzzleHint(std::string strName):AGameObject(strName)
{
	this->EKey = EventKey::RIGHT_CLICK;
	this->pText = NULL;
	this->bListenerEnabled = true;
}

PuzzleHint::~PuzzleHint()
{
	TextureManager::getInstance()->unload("HintPage");
}

void PuzzleHint::initialize()
{
	TextureManager::getInstance()->load("sequence_game/Popup_Panel.png", "HintPage");
	Sprite* pSprite = new Sprite("HintPage", "HintPage", Vector2D(0), Vector2D(1), 0);
	this->attachChild(pSprite);

	this->pText = new Text("TextHint", "Maragsa.otf", 90);
	pText->setPos(Vector2D(0, 50));
	pText->setScale(Vector2D(.75, .75));
	this->attachChild(this->pText);
	this->pText->setMessage("Place Holder");

	GUIButton* pButton = new GUIButton("CloseButton", "Close_Button");
	GUIToggle* pToggle = new GUIToggle(EventKey::RIGHT_CLICK);
	pButton->setPos(Vector2D(0, -300));
	pButton->setScale(Vector2D(.15, .15));
	pButton->attachComponent(pToggle);
	this->attachChild(pButton);


	Text* pDeclineText = new Text("Decline_Text", "JainiPurva-Regular.ttf", 90, 0.f, false);
	pDeclineText->setPos(Vector2D(0, -300));
	pDeclineText->setScale(Vector2D(.75, .75));
	this->attachChild(pDeclineText);
	pDeclineText->setMessage("Close");

	EventBroadcaster::getInstance()->registerListener(this);
}
void PuzzleHint::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	bool bFromToggle = false;
	// checks if the sender is the button that closes it.
	if (mapParameter.find("Sender") != mapParameter.end())
	{
		std::string senderType = *static_cast<std::string*>(mapParameter["Sender"]);
		bFromToggle = (senderType == "CloseButton");
		std::cout << senderType << std::endl;
	}
	// if object not enabled enable it based on name
	if(!this->bEnabled)
	{
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		std::string strName = *static_cast<std::string*>(mapParameter["TokenName"]);
		auto it = this->mapHint.find(strName);
		if (it != this->mapHint.end())
		{
			this->pText->setMessage(it->second);
			this->setEnabled(true);
		}
	}
	//do not change this handles the closing if separated any object that throws the event will close it
	else if(this->bEnabled && bFromToggle)
	{
		this->setEnabled(false);
		EventBroadcaster::getInstance()->enableAllListeners();
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
