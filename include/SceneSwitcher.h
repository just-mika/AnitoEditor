#pragma once
#include "AComponent.h"
#include "EnumSceneTag.h"
#include "ButtonInput.h"

class SceneSwitcher : public AComponent
{
private:
	SceneTag ETag;
	std::string strLevelFile;
	ButtonInput* pInput;
public:
	SceneSwitcher(SceneTag ETag);
	SceneSwitcher(const std::string& strLevelFile);
	~SceneSwitcher();

	void perform() override;
};

