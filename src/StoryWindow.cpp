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
	TextureManager::getInstance()->unload("StoryPanel");
	TextureManager::getInstance()->unload("StoryBack");
}

// Layout copied from the editor (all values are LOCAL, same as the save file)
void StoryWindow::initialize()
{
	TextureManager::getInstance()->load("GUI/Panel.png", "StoryPanel");
	TextureManager::getInstance()->load("GUI/back.png", "StoryBack");

	Background* pTransBack = new Background("TransparentBG", "StoryPanel", Vector2D(1.0f));
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
	this->pMessage->setMessage("Bakunawa and the Seven Moons\n\nOnce upon a time, in the ancient Philippines, Bathala-a supreme\ngod-created seven moons that lit up the earth.\n\nThey floated across the night sky, turning darkness into beauty and\nsilence into wonder.\n\nBut deep beneath the waves was a scary creature.\n\nThat creature was Bakunawa, a great serpent-dragon of the sea,\nwho ruled over the ocean. His body wrapped around the world, and\nhis home was cold and shadowy.\n\nOne night, he rose from the depths and saw the moons for the first\ntime.\n\nHe was amazed.\n\nTheir soft glow lit the world in ways he had never known. They were\nso bright... so beautiful...\n\nAnd he wanted them-all of them.\n\nAnd so, one night, Bakunawa leapt from the black ocean and flew\ninto the sky.\n\nHe opened his massive jaws and swallowed one moon whole.\n\nAs the moon disappeared, the Earth trembled. A terrible earthquake\nshook the night.\nBakunawa returned to the sea, full and satisfied.\nBut soon, the moon inside him melted away, like a wax near fire.\n\nFurious, Bakunawa tried again the next night. He devoured a second\nmoon.\nAnd again... it melted.\n\n\"There are so many moons,\" he hissed. \"I will eat them all!\"\n\nNight after night, the serpent struck. And night after night, another\nmoon vanished.\n\nSix moons were gone.\n\nOnly one remained.\n\nThe people of the ancient lands were afraid.\n\nEach time a moon vanished, earthquakes shook their homes, and\ndarkness filled the skies.\n\nThey knew Bakunawa would return-and they had to stop him.\n\nSo they learned the sea dragon feared loud noises.\n\nSo they gathered their pots, pans, gongs, and drums. And they\nwaited.\n\nOn a moonless night, Bakunawa rose once more from the sea.\n\nWith a thunderous cry, he soared upwards, mouth opened wide, and\nwrapped his teeth around the seventh and final moon.\n\nThe world began to dim. Shadows spread across the land. It was\nalmost gone.\n\nBut then-BANG! CLANG! BOOM!\n\nThe people ran to the serpent dragon, shouting:\n\n\"Return our moon!\"\n\nThey banged pots and pans, beat gongs, and yelled into the sky. The\nnoise thundered across the land and sea.\n\nStartled and afraid, Bakunawa spat out the moon just before\nswallowing it whole. He roared and went back into the deep and\ndark ocean.\n\nThe moon rose again, casting light on the land. The people cheered\nand danced beneath its glow.\n\nLittle did Bakunawa know, Bathala had seen everything.\n\nTo guard the moon forever, Bathala planted tall bamboo trees on its\nsurface. From afar, they looked like dark spots-shadows that still\nremain today.\n\nBakunawa never gave up. Even now, sometimes, he tries to steal the\nmoon again.\n\nAnd whenever he does, people make noise-banging drums,\nlighting fireworks, and shouting to the skies-until the moon is safe\nonce more.\n\nBecause as long as the bamboo trees stand, and the people\nremember...\n\nBakunawa will never succeed.\n\n-THE END-");
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
		textStartPos = pText->getLocalPos();   //remember the editor-placed position
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		this->setEnabled(true);
	}
	else if (this->bEnabled && bFromToggle)
	{
		this->setEnabled(false);
		pText->setLocalPos(textStartPos);      //undo the scrolling
		EventBroadcaster::getInstance()->enableAllListeners();
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
