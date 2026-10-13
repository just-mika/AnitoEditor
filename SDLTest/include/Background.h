#pragma once
#include "AGameObject.h"

class Background : public AGameObject
{
private:
	std::string strImageName;
	

public:
	Background(const std::string& strName, const std::string& strImageName = "Square", Vector2D fVecScale = Vector2D(1.0f));
	void initialize() override;
};

