#pragma once
#include "AGameObject.h"

class Gem : public AGameObject
{
private:
	std::string strPickupTitle;
	std::string strPickupText;
public:
	Gem(std::string strName);

	void initialize() override;

	const std::string& getPickupTitle() const { return strPickupTitle; }
	const std::string& getPickupText() const { return strPickupText; }
	void setPickupTitle(const std::string& s) { strPickupTitle = s; }
	void setPickupText(const std::string& s) { strPickupText = s; }
};

