#include "SceneSwitcher.h"
#include "AGameObject.h"
#include "SceneTransitionManager.h"

SceneSwitcher::SceneSwitcher(SceneTag ETag) : AComponent("SceneSwitcher", ComponentType::SCRIPT)
{
	this->ETag = ETag;
	this->pInput = NULL;
}

SceneSwitcher::SceneSwitcher(const std::string& strLevelFile) : AComponent("SceneSwitcher", ComponentType::SCRIPT)
{
	this->ETag = SceneTag::NONE;
	this->strLevelFile = strLevelFile;
	this->pInput = NULL;
}


SceneSwitcher::~SceneSwitcher()
{

}

void SceneSwitcher::perform()
{
	if (this->pInput == NULL)
		pInput = (ButtonInput*)pOwner->findComponentByName("ButtonInput");

	if (pInput->getClicked())
	{
		pInput->setClicked(false);
		if (!this->strLevelFile.empty())
			SceneTransitionManager::getInstance()->requestTransition(this->strLevelFile, TransitionType::FADE);
		else
			SceneTransitionManager::getInstance()->requestTransition(this->ETag, TransitionType::FADE);
	}
}
