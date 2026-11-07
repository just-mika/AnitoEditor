#include "GemInputScreen.h"
#include "Background.h"
#include "GUIButton.h"
#include "Sprite.h"
#include "Text.h"
#include "GameObjectManager.h"
#include "EventBroadcaster.h"
#include "GemInputToken.h"
#include "GemInputManager.h"
#include "GUIToggle.h"
#include "ButtonInput.h"
#include "EmptyObject.h"
#include "GemInputPreview.h"

GemInputScreen::GemInputScreen(const std::string& name) : AGameObject(name)
{
	this->bIsScreenObject = true;
	this->EKey = EventKey::GEM_INPUT_SCREEN;
	this->bListenerEnabled = true;
}

void GemInputScreen::initialize()
{
	// one gem input screen per level: forget any previous screen's pieces before registering ours
	GemInputManager::getInstance()->clearScreen();

	Background* pBG = new Background("GemInputBG", "typing_screen", Vector2D(1.0f));
	this->attachChild(pBG);
	pBG->setLocalPos(Vector2D(-1046, -540));
	pBG->setLocalScale(Vector2D(0.96f, 0.9f));

	Sprite* pTopSprite = new Sprite("TopSprite", "typing_top");
	this->attachChild(pTopSprite);
	pTopSprite->setIsScreenObject(true);
	pTopSprite->setLocalPos(Vector2D(-86, -520));
	pTopSprite->setLocalScale(Vector2D(1.0f, 0.9f));
	((SpriteRenderer*)pTopSprite->findComponentByName("SpriteRenderer"))->setPivot(Vector2D(0.5f, 0.0f));

	Sprite* pTabletSprite = new Sprite("TabletSprite", "typing_tablet");
	this->attachChild(pTabletSprite);
	pTabletSprite->setIsScreenObject(true);
	pTabletSprite->setLocalPos(Vector2D(-86, 70));
	pTabletSprite->setLocalScale(Vector2D(0.95f, 0.85f));

	Sprite* pCirclesBG = new Sprite("CirclesBG", "Square");
	this->attachChild(pCirclesBG);
	pCirclesBG->setIsScreenObject(true);
	pCirclesBG->setLocalPos(Vector2D(-546, -15));
	pCirclesBG->setLocalScale(Vector2D(0.9f, 0.65f));
	pCirclesBG->setColor({ 90, 45, 20, 200 });

	// ---- answer rows (right side): box, placeholder text, undo button per row ----
	static const char* kRowText[] = { "Somebody...", "Wanted...", "But...", "So...", "Then..." };

	EmptyObject* pRows = new EmptyObject("GemInput");
	this->attachChild(pRows);
	pRows->setIsScreenObject(true);
	pRows->setLocalPos(Vector2D(12.28f, -254.56f));

	for (int i = 0; i < 5; i++)
	{
		std::string strIndex = std::to_string(i + 1);
		float fRowY = 104.56f + i * 100.0f;

		EmptyObject* pRow = new EmptyObject("GemInput" + strIndex);
		pRows->attachChild(pRow);
		pRow->setIsScreenObject(true);

		Sprite* pSquare = new Sprite("GemInput_Square" + strIndex, "Square");
		pRow->attachChild(pSquare);
		pSquare->setIsScreenObject(true);
		pSquare->setLocalPos(Vector2D(481.72f, fRowY));
		pSquare->setLocalScale(Vector2D(1.4f, 0.15f));
		((SpriteRenderer*)pSquare->findComponentByName("SpriteRenderer"))->setPivot(Vector2D(1.0f, 0.5f));
		pSquare->setColor({ 90, 45, 20, 255 });
		GemInputManager::getInstance()->addTextBox(pSquare);

		Text* pSquareText = new Text("GemInput_SquareText" + strIndex, "JainiPurva-Regular.ttf", 40, 0.f, false);
		pRow->attachChild(pSquareText);
		pSquareText->setIsScreenObject(true);
		pSquareText->setPivot(Vector2D(0.0f, 0.5f));
		pSquareText->setMessage(kRowText[i]);
		pSquareText->setLocalPos(Vector2D(-208.28f, fRowY - 5.0f));
		pSquareText->setColor({ 255, 255, 255, 127 });
		GemInputManager::getInstance()->addText(pSquareText);

		GUIButton* pUndo = new GUIButton("GemInput_Undo" + strIndex, "Return");
		pRow->attachChild(pUndo);
		pUndo->setIsScreenObject(true);
		pUndo->setLocalPos(Vector2D(581.72f, fRowY));
		pUndo->setLocalScale(Vector2D(0.035f, 0.035f));
		((SpriteRenderer*)pUndo->findComponentByName("SpriteRenderer"))->setPivot(Vector2D(1.0f, 0.5f));
		GemInputManager::getInstance()->addUndoButton((ButtonInput*)pUndo->findComponentByName("ButtonInput"));
	}

	// ---- circles and gem tokens (left side) ----
	EmptyObject* pContainers = new EmptyObject("GemContainers");
	this->attachChild(pContainers);
	pContainers->setIsScreenObject(true);
	pContainers->setLocalPos(Vector2D(-852, -309));

	const Vector2D circlesPos(295.15f, 287.72f);   //where the circle group sits in GemContainers
	EmptyObject* pCircles = new EmptyObject("Circles");
	pContainers->attachChild(pCircles);
	pCircles->setIsScreenObject(true);
	pCircles->setLocalPos(circlesPos);

	EmptyObject* pTokens = new EmptyObject("GemTokens");
	pContainers->attachChild(pTokens);
	pTokens->setIsScreenObject(true);

	// circle layout (relative to the circle group); token i sits 10 below circle i
	static const Vector2D kCirclePos[] = {
		Vector2D(-150, -95), Vector2D(150, -95), Vector2D(0, 0), Vector2D(-150, 95), Vector2D(150, 95) };
	static const char* kGemNames[] = { "Gem_Red", "Gem_Orange", "Gem_Purple", "Gem_Green", "Gem_Cyan" };

	for (int i = 0; i < 5; i++)
	{
		Sprite* pCircle = new Sprite("CircleSprite" + std::to_string(i + 1), "Circle");
		pCircles->attachChild(pCircle);
		pCircle->setIsScreenObject(true);
		pCircle->setLocalPos(kCirclePos[i]);
		pCircle->setLocalScale(Vector2D(0.4f, 0.4f));
		pCircle->setColor({ 60, 0, 8, 255 });

		GemInputToken* pToken = new GemInputToken(kGemNames[i], Vector2D(0.0f), Vector2D(.3f));
		pTokens->attachChild(pToken);
		pToken->setLocalPos(circlesPos + kCirclePos[i] + Vector2D(0, 10));
		GemInputManager::getInstance()->addToken(pToken);   //after setLocalPos: it remembers this spot
	}

	// the correct order: which token goes in each row
	GemInputManager::getInstance()->addWinCon(4);
	GemInputManager::getInstance()->addWinCon(3);
	GemInputManager::getInstance()->addWinCon(1);
	GemInputManager::getInstance()->addWinCon(2);
	GemInputManager::getInstance()->addWinCon(0);

	// ---- title and buttons ----
	Text* pTitleText = new Text("TitleText", "JainiPurva-Regular.ttf", 80, 0.f, false);
	this->attachChild(pTitleText);
	pTitleText->setIsScreenObject(true);
	pTitleText->setMessage("Map SWBST");
	pTitleText->setLocalPos(Vector2D(-86, -425));
	pTitleText->setColor({ 0, 0, 0, 255 });

	GUIButton* pReturn = new GUIButton("GemInput_Return", "Return");
	this->attachChild(pReturn);
	pReturn->setIsScreenObject(true);
	pReturn->setLocalPos(Vector2D(-996, -490));
	pReturn->setLocalScale(Vector2D(0.07f, 0.07f));
	((SpriteRenderer*)pReturn->findComponentByName("SpriteRenderer"))->setPivot(Vector2D(0.0f, 0.0f));
	pReturn->attachComponent(new GUIToggle(EventKey::GEM_INPUT_SCREEN));

	GUIButton* pRecall = new GUIButton("GemInput_Recall", "Tablet");
	this->attachChild(pRecall);
	pRecall->setIsScreenObject(true);
	pRecall->setLocalPos(Vector2D(754, -492));
	pRecall->setLocalScale(Vector2D(0.15f, 0.15f));
	pRecall->attachComponent(new GUIToggle(EventKey::STORY_SCREEN));   //Recall = reread the story
	((SpriteRenderer*)pRecall->findComponentByName("SpriteRenderer"))->setPivot(Vector2D(1.0f, 0.0f));

	Text* pRecallText = new Text("RecallText", "JainiPurva-Regular.ttf", 45, 0.f, false);
	pRecall->attachChild(pRecallText);
	pRecallText->setIsScreenObject(true);
	pRecallText->setMessage("Recall");
	pRecallText->setLocalPos(Vector2D(-826.67f, 393.33f));
	pRecallText->setLocalScale(Vector2D(8.4f, 8.4f));

	GUIButton* pScore = new GUIButton("GemInput_Score", "Tablet");
	this->attachChild(pScore);
	pScore->setIsScreenObject(true);
	pScore->setLocalPos(Vector2D(638, 530));
	pScore->setLocalScale(Vector2D(0.16f, 0.16f));
	((SpriteRenderer*)pScore->findComponentByName("SpriteRenderer"))->setPivot(Vector2D(1.0f, 1.0f));

	Text* pScoreText = new Text("ScoreTxt", "JainiPurva-Regular.ttf", 45, 0.f, false);
	pScore->attachChild(pScoreText);
	pScoreText->setIsScreenObject(true);
	pScoreText->setMessage("Score: ???");
	pScoreText->setLocalPos(Vector2D(-800, -475));
	pScoreText->setLocalScale(Vector2D(7.3125f, 7.3125f));

	GUIButton* pSubmit = new GUIButton("GemInput_Submit", "tablet_bottom");
	this->attachChild(pSubmit);
	pSubmit->setIsScreenObject(true);
	pSubmit->setLocalPos(Vector2D(-86, 536));
	pSubmit->setLocalScale(Vector2D(0.5f, 0.8f));
	((SpriteRenderer*)pSubmit->findComponentByName("SpriteRenderer"))->setPivot(Vector2D(0.5f, 1.0f));
	GemInputManager::getInstance()->addSubmitButton((ButtonInput*)pSubmit->findComponentByName("ButtonInput"));

	Text* pSubmitText = new Text("SubmitText", "JainiPurva-Regular.ttf", 70, 0.f, false);
	pSubmit->attachChild(pSubmitText);
	pSubmitText->setIsScreenObject(true);
	pSubmitText->setMessage("Submit");
	pSubmitText->setLocalPos(Vector2D(0, -94.67f));
	pSubmitText->setLocalScale(Vector2D(2.0f, 1.333f));

	// the gem text popup (built by GemInputPreview itself)
	GemInputPreview* pPreview = new GemInputPreview();
	this->attachChild(pPreview);

	EventBroadcaster::getInstance()->registerListener(this);
	this->setEnabled(false);

	GemInputManager::getInstance()->setScreen(this);
}

void GemInputScreen::onEnable()
{
	GemInputManager::getInstance()->checkCollection();
}

void GemInputScreen::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	// inside a disabled group: not in play (its own enabled flag just means open/closed)
	if (this->getParent() && !this->getParent()->isGloballyEnabled()) return;

	bool bFromToggle = false;

	if (mapParameter.find("Sender") != mapParameter.end())
	{
		std::string senderType = *static_cast<std::string*>(mapParameter["Sender"]);
		if (senderType == "GemInput_Return" || senderType == "GemInputManager")
		{
			bFromToggle = true;
		}

	}
	if (!this->getEnabled())
	{
		// opening branch: the screen was closed and is being opened
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		EventBroadcaster::getInstance()->enableListener("GemInputPreview");
		EventBroadcaster::getInstance()->enableListenersWithKey(EventKey::STORY_SCREEN);   //Recall opens the story
		this->setEnabled(true);
	}
	else
	{
		if (bFromToggle)
		{
			GemInputManager::getInstance()->resetAllText();
			this->setEnabled(false);
			EventBroadcaster::getInstance()->enableAllListeners();
		}
	}
}

EventKey GemInputScreen::getKey()
{
	return this->EKey;
}

bool GemInputScreen::isListenerEnabled()
{
	return this->bListenerEnabled;
}

void GemInputScreen::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string GemInputScreen::getListenerOwnerName()
{
	return this->strName;
}

GemInputScreen::~GemInputScreen()
{
	// deleted in the editor: don't leave the manager pointing at our freed pieces
	GemInputManager* pManager = GemInputManager::getInstance();
	if (pManager && pManager->getScreen() == this)
		pManager->clearScreen();
}