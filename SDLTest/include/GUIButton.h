#pragma once
#include "AGameObject.h"

class GUIButton : public AGameObject
{
private:
	std::string strSpriteName;
	bool bFlipX;
public:
	GUIButton(std::string strName, std::string strSpriteName = "Close_Button", bool bFlipX = false);
	void initialize() override;
	void setColor(SDL_Color color);
};

