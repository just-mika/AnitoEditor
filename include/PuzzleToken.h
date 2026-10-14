#pragma once
#include "AGameObject.h"
class PuzzleToken : public AGameObject
{
private:
	int nSlot;
	bool bDropped;

public:
	PuzzleToken(std::string strName, Vector2D position = Vector2D(0.0f, 0.0f), Vector2D scale = Vector2D(0.08f));
	void initialize() override;

	int getSlot() const;
	void setSlot(int nSlot);
	bool getDropped() const;
	void setDropped(bool bDropped);

};

