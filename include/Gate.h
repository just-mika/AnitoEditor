#pragma once
#include "AGameObject.h"
class Gate : public AGameObject
{
public:
	Gate(const std::string strName, Vector2D pos = Vector2D(0.0f, 0.0f));
	void initialize() override;

	void gateOpen();
};

