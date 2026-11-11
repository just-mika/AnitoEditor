#include "StoryWindow.h"

#include "TextureManager.h"

StoryWindow::StoryWindow(std::string strName):AGameObject(strName)
{
	this->EKey = EventKey::STORY_SCREEN;
	this->bListenerEnabled = true;
	this->bIsScreenObject = true;
	this->pMessage = NULL;
	this->pTitle = NULL;
}

StoryWindow::~StoryWindow()
{
	this->pMessage = NULL;
	TextureManager::getInstance()->unload("StoryBack");
}

// Layout copied from the editor (all values are LOCAL, same as the save file)
void StoryWindow::initialize()
{
	if (!TextureManager::getInstance()->get("Panel"))
		TextureManager::getInstance()->load("GUI/Panel.png", "Panel");
	TextureManager::getInstance()->load("GUI/back.png", "StoryBack");

	Background* pTransBack = new Background("TransparentBG", "Panel", Vector2D(1.0f));
	this->attachChild(pTransBack);
	pTransBack->setLocalPos(Vector2D(-1050, -545));
	pTransBack->setLocalScale(Vector2D(0.39f, 0.37f));

	// ParentBG is the scroll window: its rect crops the story text,
	// and dragging inside it moves the text (ButtonInput + Scroller)
	Background* pParentBG = new Background("ParentBG", "Square", Vector2D(1.0f));
	this->attachChild(pParentBG);
	pParentBG->setLocalPos(Vector2D(-1055, -251));
	pParentBG->setLocalScale(Vector2D(3.83f, 1.41f));
	SpriteRenderer* pParentBGR = (SpriteRenderer*)pParentBG->findComponentByName("SpriteRenderer");
	pParentBGR->setColor({ 255, 255, 255, 0 });   //invisible, only its rect matters
	pParentBG->attachComponent(new ButtonInput(pParentBGR));

	// the story text: top-left inside the window, scale undoes ParentBG's scale
	// (the story itself is level data, typed in the Inspector)
	this->pMessage = new Text("Message", "JainiPurva-Regular.ttf", 72, 0, false);
	pParentBG->attachChild(this->pMessage);
	this->pMessage->setIsScreenObject(true);
	this->pMessage->setLocalPos(Vector2D(34.2f, -3.55f));
	this->pMessage->setLocalScale(Vector2D(0.2585f, 1 / 1.41f));
	this->pMessage->setColor(SDL_Color(139, 58, 47, 255));
	this->pMessage->setMessage("Story text");
	TextRenderer* pMessageR = (TextRenderer*)this->pMessage->findComponentByName("TextRenderer");
	pMessageR->setCropEnabled(true);
	pMessageR->setPivot(Vector2D(0, 0));

	Scroller* pScroller = new Scroller("StoryScroller", this->pMessage);
	pParentBG->attachComponent(pScroller);
	pScroller->setVert(true);

	this->pTitle = new Text("TitleText", "JainiPurva-Regular.ttf", 113, 0, false);
	this->attachChild(this->pTitle);
	this->pTitle->setIsScreenObject(true);
	this->pTitle->setMessage("Story");
	this->pTitle->setLocalPos(Vector2D(-96, -425));
	this->pTitle->setColor(SDL_Color(0, 0, 0, 255));

	// a thin black bar under the title
	Sprite* pLineBreak = new Sprite("LineBreak", "Square");
	this->attachChild(pLineBreak);
	pLineBreak->setIsScreenObject(true);
	pLineBreak->setLocalPos(Vector2D(-86, -294));
	pLineBreak->setLocalScale(Vector2D(3.27f, 0.02f));
	SpriteRenderer* pLineBreakR = (SpriteRenderer*)pLineBreak->findComponentByName("SpriteRenderer");
	pLineBreakR->setColor({ 0, 0, 0, 255 });

	// back arrow at the top-left (no label)
	GUIButton* pButton = new GUIButton("CloseButton", "StoryBack");
	this->attachChild(pButton);
	pButton->setIsScreenObject(true);
	pButton->attachComponent(new GUIToggle(EventKey::STORY_SCREEN));
	pButton->setLocalPos(Vector2D(-919, -418));
	pButton->setLocalScale(Vector2D(0.09f, 0.09f));

	EventBroadcaster::getInstance()->registerListener(this);
}

Text* StoryWindow::getStoryText()
{
	return this->pMessage;
}

Text* StoryWindow::getTitleText()
{
	return this->pTitle;
}

void StoryWindow::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	// inside a disabled group: not in play (its own enabled flag just means open/closed)
	if (this->getParent() && !this->getParent()->isGloballyEnabled()) return;

	bool bFromToggle = false;
	Text* pText = (Text*)this->findChildByName("ParentBG")->findChildByName("Message");
	if (mapParameter.find("Sender") != mapParameter.end())
	{
		std::string senderType = *static_cast<std::string*>(mapParameter["Sender"]);
		bFromToggle = (senderType == "CloseButton");
		std::cout << senderType << std::endl;
	}

	if (!this->bEnabled)
	{
		textStartPos = pText->getLocalPos();
		this->vecRestore = EventBroadcaster::getInstance()->getEnabledListeners();   //remember who was active
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		this->setEnabled(true);
	}
	else if (this->bEnabled && bFromToggle)
	{
		this->setEnabled(false);
		pText->setLocalPos(textStartPos);
		EventBroadcaster::getInstance()->restoreEnabledListeners(this->vecRestore);   //back to how it was
	}
}

EventKey StoryWindow::getKey()
{
	return this->EKey;
}

bool StoryWindow::isListenerEnabled()
{
	return this->bListenerEnabled;
}

void StoryWindow::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string StoryWindow::getListenerOwnerName()
{
	return this->getName();
}
