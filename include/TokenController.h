#pragma once
#include "AComponent.h"
#include "ButtonInput.h"
#include "EventBroadcaster.h"
class TokenController : public AComponent
{
private:
	ButtonInput* pInput;
	bool bPickedup;
	Vector2D initialScale;
	Vector2D holdingScale;
	bool bIgnorePress = false;   //this press started while a screen was open

public:
	TokenController(ButtonInput* pInput);
	
	void onAttach() override;
	void perform() override;

private:
	bool isMinDistance(Vector2D pos);
	bool bMoved = false;        //moved past the tap radius during this press
	Vector2D pressPos;          //where the press started (world)
	static constexpr float kTapRadius = 20.0f;   //finger jitter allowance
};

