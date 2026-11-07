#pragma once
#include "AComponent.h"
#include "ButtonInput.h"
class GemController: public AComponent
{
private:
	ButtonInput* pInput;
	bool bPickedup;
	Vector2D initialScale;
	Vector2D holdingScale;

	bool bMoved = false;        //moved past the tap radius during this press
	Vector2D pressPos;          //where the press started
	static constexpr float kTapRadius = 20.0f;   //finger jitter allowance
public:
	GemController(ButtonInput* pInput);


	void onAttach() override;
	// Inherited via AComponent
	void perform() override;

};

