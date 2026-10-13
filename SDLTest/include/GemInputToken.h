#pragma once
#include "AGameObject.h"
class GemInputToken:public AGameObject
{
private:
	bool bDropped;
public:
	GemInputToken(std::string strName, Vector2D position = Vector2D(0.0f, 0.0f), Vector2D scale = Vector2D(0.3f));
	void initialize() override;

	bool getDropped() const;
	void setDropped(bool bDropped);
};

