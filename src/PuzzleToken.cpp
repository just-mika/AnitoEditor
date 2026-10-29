#include "PuzzleToken.h"
#include "SpriteRenderer.h"
#include "ButtonInput.h"
#include "TokenController.h"
#include "GemInputToken.h"

PuzzleToken::PuzzleToken(std::string strName, Vector2D position, Vector2D scale,
	std::string strTexture) : AGameObject(strName)
{
	this->strTexture = strTexture;

	this->nSlot = -1;
	this->bDropped = false;
	this->fVecTranslate = position;
	this->fVecScale = scale;
}

void PuzzleToken::initialize()
{
	SpriteRenderer* pRenderer = new SpriteRenderer(this->strTexture);
	ButtonInput* pInput = new ButtonInput(pRenderer);
	TokenController* pController = new TokenController(pInput);

	this->attachComponent(pRenderer);
	this->attachComponent(pInput);
	this->attachComponent(pController);

	std::cout << "Puzzle Token: [" << this->strName << "] initialized." << "\n";
}

int PuzzleToken::getSlot() const
{
	return this->nSlot;
}

void PuzzleToken::setSlot(int nSlot)
{
	this->nSlot = nSlot;
}



bool PuzzleToken::getDropped() const
{
	return this->bDropped;
}

void PuzzleToken::setDropped(bool bDropped)
{
	this->bDropped = bDropped;
}
