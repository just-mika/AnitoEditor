#pragma once
#include "AGameObject.h"
#include "RigidBody.h"
#include "ICollisionListener.h"
class Player : public AGameObject, ICollisionListener
{
public:
	Player(Vector2D fVecTranslate, Vector2D fVecScale, float fRot);
	Player(const std::string& strName, Vector2D fVecTranslate = Vector2D(0.0f), Vector2D fVecScale = Vector2D(0.6f), float fRot = 0.0f);
	void initialize() override;

	 //Inherited via ACollisionListener
	void onCollisionEnter(ACollider* pCollider) override;

	void onCollisionContinue(ACollider* pCollider) override;

	void onCollisionExit(ACollider* pCollider) override;

};

