#include "InventoryGUI.h"
#include "Background.h"
#include "SpriteRenderer.h"
#include "SpriteAnimator.h"
#include "EventBroadcaster.h"
#include "TextureManager.h"
#include "GameObjectManager.h"
#include "GUIButton.h"
#include "Text.h"
#include "GUIToggle.h"
#include "DataAssetManager.h"
#include "EmptyObject.h"

InventoryGUI::InventoryGUI(std::string strName):AGameObject(strName)
{
	this->pDataAsset = (CollectableGemDataAsset*)DataAssetManager::getInstance()->getDataAsset("CollectableGemDataAsset");
	this->EKey = EventKey::INVENTORY_SCREEN;
	this->bListenerEnabled = true;
	this->bIsScreenObject = true;
	this->pPlayer = NULL;

}

InventoryGUI::~InventoryGUI()
{
	TextureManager::getInstance()->unload("InventoryPanel");
	TextureManager::getInstance()->unload("GemHighlight");
}

// Layout copied from the editor (all values are LOCAL, same as the save file)
void InventoryGUI::initialize()
{
	TextureManager::getInstance()->load("GUI/Panel.png", "InventoryPanel");
	TextureManager::getInstance()->load("gems/gem_highlight.png", "GemHighlight");

	Background* pTransBack = new Background("TransparentBG", "InventoryPanel", Vector2D(1.0f));
	this->attachChild(pTransBack);
	pTransBack->setLocalPos(Vector2D(-6, -10));
	pTransBack->setLocalScale(Vector2D(0.39f, 0.37f));
	SpriteRenderer* pTransBGR = (SpriteRenderer*)pTransBack->findComponentByName("SpriteRenderer");
	pTransBGR->setColor({ 255, 255, 255, 179 });

	// the tablet on the right, turned on its side, holding the gem buttons
	Sprite* pInventoryBG = new Sprite("InventoryBG", "Tablet");
	this->attachChild(pInventoryBG);
	pInventoryBG->setIsScreenObject(true);
	pInventoryBG->setLocalPos(Vector2D(1897, 568));
	pInventoryBG->setLocalScale(Vector2D(0.8f, 0.8f));
	pInventoryBG->setLocalRot(-90);

	EmptyObject* pGemButtons = new EmptyObject("Gem_Buttons");
	pInventoryBG->attachChild(pGemButtons);
	pGemButtons->setIsScreenObject(true);
	pGemButtons->setLocalPos(Vector2D(-376.55f, 149.04f));
	pGemButtons->setLocalScale(Vector2D(1.25f, 1.25f));
	pGemButtons->setLocalRot(90);   //undoes the tablet's -90, so the gems stand upright

	// outline behind the selected gem (moved onto the clicked button)
	Sprite* pHighlight = new Sprite("Gem_Highlight", "GemHighlight");
	pGemButtons->attachChild(pHighlight);
	pHighlight->setIsScreenObject(true);
	pHighlight->setLocalPos(Vector2D(-15.77f, -118.76f));
	pHighlight->setLocalScale(Vector2D(0.57f, 0.57f));

	// one button per gem color, top to bottom; shown only if that gem was collected
	static const char* kGems[] = { "Gem_Cyan", "Gem_Green", "Gem_Orange", "Gem_Purple", "Gem_Red" };
	static const float kGemY[] = { -120.76f, 87.24f, 302.24f, 505.24f, 711.24f };
	this->vecGems.clear();
	for (int i = 0; i < 5; i++)
	{
		std::string strGem = kGems[i];
		GUIButton* pButton = new GUIButton(strGem + "_Button", strGem, false);
		pGemButtons->attachChild(pButton);
		pButton->setIsScreenObject(true);
		pButton->attachComponent(new GUIToggle(EventKey::INVENTORY_SCREEN));
		pButton->setLocalPos(Vector2D(-15.77f, kGemY[i]));
		pButton->setLocalScale(Vector2D(0.49f, 0.49f));
		this->vecGems.push_back(pButton);
	}

	GUIButton* pReturn = new GUIButton("ReturnButton", "Return", false);
	this->attachChild(pReturn);
	pReturn->setIsScreenObject(true);
	pReturn->attachComponent(new GUIToggle(EventKey::INVENTORY_SCREEN));
	pReturn->setLocalPos(Vector2D(127, 122));
	pReturn->setLocalScale(Vector2D(0.09f, 0.09f));

	// plays first, stays on its last frame while the inventory is open
	AnimatedSprite* pAnimatedCharacter = new AnimatedSprite("InventoryAnim", "Inventory", Vector2D(0.0f), Vector2D(1.0f), 0.0f, 8);
	this->attachChild(pAnimatedCharacter);
	pAnimatedCharacter->setIsScreenObject(true);
	pAnimatedCharacter->setLocalPos(Vector2D(0, -24));
	pAnimatedCharacter->setLocalScale(Vector2D(0.92f, 0.92f));
	SpriteRenderer* pAnimR = (SpriteRenderer*)pAnimatedCharacter->findComponentByName("SpriteRenderer");
	pAnimR->setPivot(Vector2D(0, 0));
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimatedCharacter->findComponentByName("SpriteAnimator");
	pAnimator->getCurrentAnimation()->setType(AnimationType::ONCE);
	pAnimator->getCurrentAnimation()->setOnAnimFinished(OnAnimFinished::FUNC);
	pAnimator->addListener(this);

	// the selected gem's big picture, title and text (scale undoes the gem's 0.89)
	Sprite* pGemInfo = new Sprite("InfoGem", "Gem_Colorless_Inventory");
	this->attachChild(pGemInfo);
	pGemInfo->setIsScreenObject(true);
	pGemInfo->setLocalPos(Vector2D(919, 529));
	pGemInfo->setLocalScale(Vector2D(0.89f, 0.89f));

	Text* pTitle = new Text("Title", "JainiPurva-Regular.ttf", 90, 0, false);
	pGemInfo->attachChild(pTitle);
	pTitle->setIsScreenObject(true);
	pTitle->setMessage("No Gem");
	pTitle->setLocalPos(Vector2D(55.56f, -333.33f));
	pTitle->setLocalScale(Vector2D(1 / 0.89f, 1 / 0.89f));
	pTitle->setLocalRot(-14);
	pTitle->setColor(SDL_Color(0, 0, 0, 255));

	Text* pText = new Text("Text", "JainiPurva-Regular.ttf", 45, 0, false);
	pGemInfo->attachChild(pText);
	pText->setIsScreenObject(true);
	pText->setMessage("No gem collected yet!");
	pText->setLocalPos(Vector2D(0, -111.11f));
	pText->setLocalScale(Vector2D(1 / 0.89f, 1 / 0.89f));
	pText->setLocalRot(-14);
	pText->setColor(SDL_Color(0, 0, 0, 255));

	this->setPanelVisible(false);
	pAnimatedCharacter->setEnabled(false);
	EventBroadcaster::getInstance()->registerListener(this);
}

// the parts that appear after the animation (the animation itself is handled separately)
void InventoryGUI::setPanelVisible(bool bVisible)
{
	this->findChildByName("TransparentBG")->setEnabled(bVisible);
	this->findChildByName("InventoryBG")->setEnabled(bVisible);
	this->findChildByName("ReturnButton")->setEnabled(bVisible);
	this->findChildByName("InfoGem")->setEnabled(bVisible);
}

std::string InventoryGUI::gemOfButton(const std::string& strButton)
{
	const std::string suffix = "_Button";
	if (strButton.size() <= suffix.size() || strButton.compare(0, 4, "Gem_") != 0 ||
		strButton.compare(strButton.size() - suffix.size(), suffix.size(), suffix) != 0)
		return "";
	return strButton.substr(0, strButton.size() - suffix.size());
}

void InventoryGUI::changeGem(std::string strGemName)
{
	SpriteRenderer* pRenderer = (SpriteRenderer*)this->findChildByName("InfoGem")->findComponentByName("SpriteRenderer");
	std::vector<SDL_Texture*> vecTex = TextureManager::getInstance()->getTexture(strGemName);
	if (!vecTex.empty())
		pRenderer->setTexture(vecTex[0]);
}
void InventoryGUI::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	// switched off in the level (or inside a disabled group): this screen isn't in play
	if (!this->isGloballyEnabled()) return;

	AnimatedSprite* pAnimSprite = (AnimatedSprite*)this->findChildByName("InventoryAnim");
	SpriteAnimator* pAnimator = (SpriteAnimator*)pAnimSprite->findComponentByName("SpriteAnimator");
	AGameObject* pGemInfo = this->findChildByName("InfoGem");
	Text* pTitle = (Text*)pGemInfo->findChildByName("Title");
	Text* pText = (Text*)pGemInfo->findChildByName("Text");
	AGameObject* pGemButtons = this->findChildByName("InventoryBG")->findChildByName("Gem_Buttons");
	AGameObject* pHighlight = pGemButtons->findChildByName("Gem_Highlight");

	std::string strSender;
	if (mapParameter.find("Sender") != mapParameter.end())
		strSender = *static_cast<std::string*>(mapParameter["Sender"]);

	std::string strSelect;   //the gem button to select this time ("" = none)

	if (!pAnimSprite->getEnabled())
	{
		// closed: open it. Start clean whatever the level saved; only the animation shows until it ends
		this->setPanelVisible(false);
		pAnimSprite->setEnabled(true);
		EventBroadcaster::getInstance()->disableOtherListenerExcept(this);
		pAnimator->getCurrentAnimation()->stop();
		pAnimator->getCurrentAnimation()->play();

		// only the collected gems get a button; the first one (top to bottom) starts selected
		for (GUIButton* pButton : this->vecGems)
		{
			bool bCollected = this->pDataAsset->getGemPickedUp(gemOfButton(pButton->getName()));
			pButton->setEnabled(bCollected);
			if (bCollected && strSelect.empty())
				strSelect = pButton->getName();
		}

		// nothing collected: empty info panel
		if (strSelect.empty())
		{
			pHighlight->setEnabled(false);
			pTitle->setMessage("No Gem");
			pText->setMessage("No gem collected yet!");
			this->changeGem("Gem_Colorless_Inventory");
		}
	}
	else if (strSender == "ReturnButton")
	{
		// open: Return closes everything
		this->setPanelVisible(false);
		pAnimSprite->setEnabled(false);
		EventBroadcaster::getInstance()->enableAllListeners();
		return;
	}
	else
	{
		// open: a gem button was clicked
		strSelect = strSender;
	}

	// show the selected gem and move the highlight onto its button
	std::string strGem = gemOfButton(strSelect);
	if (strGem.empty()) return;

	pTitle->setMessage(this->pDataAsset->getGemTitle(strGem));
	pText->setMessage(this->pDataAsset->getGemTextBreak(strGem));
	this->changeGem(strGem + "_Inventory");

	AGameObject* pButton = pGemButtons->findChildByName(strSelect);
	pHighlight->setLocalPos(pButton->getLocalPos() + Vector2D(0, 2));   //sits 2 below each button in the layout
	pHighlight->setEnabled(true);
}

EventKey InventoryGUI::getKey()
{
	return this->EKey;
}

bool InventoryGUI::isListenerEnabled()
{
	return this->bListenerEnabled;
}

void InventoryGUI::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string InventoryGUI::getListenerOwnerName()
{
	return this->strName;
}

void InventoryGUI::onAnimationFinished()
{
	this->setPanelVisible(true);
}
