#pragma once
#include "AGeneralInput.h"
class PlayerInput : public AGeneralInput
{
private:
	bool bHoldingLeft;
	bool bHoldingRight;
	bool bHoldingDown;
	bool bHoldingUp;

	bool bInteracted;         // keyboard press, latched until consumed
	bool bVirtualInteract = false;
	bool bJumped;
	Vector2D movement;

public:
	PlayerInput();
	~PlayerInput();

	void perform() override;

	Vector2D getMovement() const;
	bool getJumped() const;
	bool getInteracted() const;
	// Consumes the interact request (clears both keyboard and touch sources).
	void setInteracted(bool interacted);
	// Touch/mouse Interact button. Kept separate so it can't clear a keyboard press.
	void setVirtualInteract(bool pressed);
	bool getVirtualInteract() const;

    void setVirtualMovement(const Vector2D& v);
    void setVirtualJump(bool pressed);

    // virtual (touch) input
    Vector2D virtualMovement = Vector2D::Zero();
    bool virtualJump = false;

private:
	void onKeyDown(SDL_Keycode key);
	void onKeyUp(SDL_Keycode key);

	void updateValues(bool virtualControls = false);
};

