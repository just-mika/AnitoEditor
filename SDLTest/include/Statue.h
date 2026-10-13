#pragma once
#include "AGameObject.h"
class Statue: public AGameObject
{
public:
	Statue(const std::string strName, Vector2D pos = Vector2D(0.0f, 0.0f));
	void initialize() override;
};

