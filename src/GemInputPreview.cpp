#include "GemInputPreview.h"
#include "GUIButton.h"
#include "Sprite.h"
#include "Text.h"
#include "GameObjectManager.h"
#include "EventBroadcaster.h"
#include "GUIToggle.h"
#include "DataAssetManager.h"
#include "GUIUtils.h"
#include "TextureManager.h"

GemInputPreview::GemInputPreview(const std::string& name) : AGameObject(name)
{
    this->bIsScreenObject = true;
    this->EKey = EventKey::GEM_INPUT_PREVIEW;
	this->bListenerEnabled = true;
	this->pDataAsset = (CollectableGemDataAsset*)DataAssetManager::getInstance()->getDataAsset("CollectableGemDataAsset");
}

void GemInputPreview::setPreviewText(std::string strGemName)
{
	Text* pPreviewText = (Text*)this->findChildByName("GemInput_PreviewText");
	pPreviewText->setMessage(this->pDataAsset->getGemTextBreak(strGemName));
}

void GemInputPreview::initialize()
{
	if (!TextureManager::getInstance()->get("Panel"))   //shared by all panel screens; decode once
		TextureManager::getInstance()->load("GUI/Panel.png", "Panel");

	Sprite* pPreviewBG = new Sprite("GemInput_PreviewWindow", "Panel");
	this->attachChild(pPreviewBG);
	pPreviewBG->setIsScreenObject(true);
	pPreviewBG->setLocalPos(Vector2D(-536, 50));
	pPreviewBG->setLocalScale(Vector2D(0.1f, 0.17f));

	Text* pPreviewText = new Text("GemInput_PreviewText", "JainiPurva-Regular.ttf", 60, 0.f, false);
	this->attachChild(pPreviewText);
	pPreviewText->setIsScreenObject(true);
	pPreviewText->setMessage("Preview \n Text");
	pPreviewText->setLocalPos(Vector2D(-536, 0));
	pPreviewText->setColor({ 0, 0, 0, 255 });
	pPreviewText->setWrapWidth(400); 

	GUIButton* pClosePreview = new GUIButton("GemInput_ClosePreview", "Close_Button");
	this->attachChild(pClosePreview);
	pClosePreview->setIsScreenObject(true);
	pClosePreview->attachComponent(new GUIToggle(EventKey::GEM_INPUT_PREVIEW));
	pClosePreview->setLocalPos(Vector2D(-536, 232));
	pClosePreview->setLocalScale(Vector2D(0.1f, 0.1f));

	Text* pClosePreviewText = new Text("GemInput_ClosePreviewText", "JainiPurva-Regular.ttf", 50, 0.f, false);
	this->attachChild(pClosePreviewText);
	pClosePreviewText->setIsScreenObject(true);
	pClosePreviewText->setMessage("Close");
	pClosePreviewText->setLocalPos(Vector2D(-536, 231));
	pClosePreviewText->setColor({ 180, 20, 20, 255 });

	EventBroadcaster::getInstance()->registerListener(this);
	this->setEnabled(false);
}

void GemInputPreview::onEventTrigger(std::unordered_map<std::string, void*> mapParameter)
{
	// inside a disabled group: not in play (its own enabled flag just means open/closed)
	if (this->getParent() && !this->getParent()->isGloballyEnabled()) return;

	bool bFromToggle = false;
	if (mapParameter.find("Sender") != mapParameter.end())
	{
		std::string senderType = *static_cast<std::string*>(mapParameter["Sender"]);
		bFromToggle = (senderType == "GemInput_ClosePreview");
	}

	// a gem was tapped: show its text (also replaces the text if already open)
	if (mapParameter.find("GemName") != mapParameter.end())
	{
		std::string strGemName = *static_cast<std::string*>(mapParameter["GemName"]);
		this->setPreviewText(strGemName);
		this->setEnabled(true);
	}
	// the Close button
	else if (this->bEnabled && bFromToggle)
	{
		this->setEnabled(false);
	}
}

EventKey GemInputPreview::getKey()
{
	return this->EKey;
}

bool GemInputPreview::isListenerEnabled()
{
    return this->bListenerEnabled;
}

void GemInputPreview::setListenerEnabled(bool bListenerEnabled)
{
	this->bListenerEnabled = bListenerEnabled;
}

std::string GemInputPreview::getListenerOwnerName()
{
	return this->strName;
}

GemInputPreview::~GemInputPreview()
{
}