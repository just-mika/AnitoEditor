#pragma once
#include "AGameObject.h"
class PuzzleToken : public AGameObject
{
private:
	int nSlot;
	bool bDropped;

	std::string strTargetName;
	bool bHasTarget = false;
	std::string strHint;

public:
	bool hasTargetPos() const { return bHasTarget; }
	const std::string& getTargetName() const { return strTargetName; }
	void setTargetName(const std::string& name) { strTargetName = name; }
	const std::string& getHint() const { return strHint; }
	void setHint(const std::string& h) { strHint = h; }

public:
	PuzzleToken(std::string strName, Vector2D position = Vector2D(0.0f),
		Vector2D scale = Vector2D(0.08f), std::string strTexture = "Token_Horns");

	std::string strTexture;	void initialize() override;

	int getSlot() const;
	void setSlot(int nSlot);
	bool getDropped() const;
	void setDropped(bool bDropped);

};

