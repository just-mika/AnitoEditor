#include "GUIToggle.h"
#include "GameObjectManager.h"
#include "ButtonInput.h"

const GUIToggle::ClickOption GUIToggle::OPTIONS[] = {
	{ "Pause", EventKey::PAUSE_SCREEN },
	{ "Story", EventKey::STORY_SCREEN },
	{ "Inventory", EventKey::INVENTORY_SCREEN },
	{ "Objective", EventKey::OBJECTIVE_SCREEN },
	{ "Gem Input", EventKey::GEM_INPUT_SCREEN },
	{ "Find Objective", EventKey::OBJECTIVE_BUTTON }
};
const int GUIToggle::OPTION_COUNT = std::size(OPTIONS);

GUIToggle::GUIToggle(EventKey EKey): AComponent("GUIToggle", ComponentType::SCRIPT)
{
	this->EKey = EKey;
}

GUIToggle::~GUIToggle()
{

}

void GUIToggle::perform()
{
	ButtonInput* pInput = (ButtonInput*)pOwner->findComponentByName("ButtonInput");
	if (pInput && pInput->getClicked())
	{
		std::cout << "Click" << std::endl;

		pInput->setClicked(false);
		std::unordered_map <std::string, void*> mapParam;
		std::string strName = pOwner->getName();

		mapParam["Sender"] = static_cast<void*>(&strName);
		EventBroadcaster::getInstance()->broadcast(this->EKey, mapParam);
	}
}

EventKey GUIToggle::getKey() const
{
	return this->EKey;
}

void GUIToggle::setKey(EventKey EKey)
{
	this->EKey = EKey;
}

const char* GUIToggle::nameOf(EventKey key)
{
	for (int i = 0; i < OPTION_COUNT; i++)
		if (OPTIONS[i].key == key)
			return OPTIONS[i].name;
	return nullptr;
}

bool GUIToggle::keyOf(const std::string& name, EventKey& outKey)
{
	for (int i = 0; i < OPTION_COUNT; i++)
	{
		if (name == OPTIONS[i].name)
		{
			outKey = OPTIONS[i].key;
			return true;
		}
	}
	return false;
}
