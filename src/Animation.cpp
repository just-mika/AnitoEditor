#include "Animation.h"
#include <cmath>

Animation::Animation(
    std::string strName,
    std::vector<SDL_Texture*> vecFrames,
    Uint8 nFrameRate,
    AnimationType EType,
	OnAnimFinished EOnFinished,
    std::string strNextState
)
{
    this->strName = strName;
    this->vecFrames = vecFrames;
    this->nFrameRate = nFrameRate;
    this->EType = EType;
	this->EOnFinished = EOnFinished;
    this->strNextState = strNextState;

	this->stop();
}

void Animation::play()
{
	//a finished one-shot animation would otherwise resume past its last frame
	if (this->bFinished)
		this->stop();

	this->bIsPlaying = true;
	this->bFinished = false;
}

void Animation::pause()
{
	this->bIsPlaying = false;
}

void Animation::stop()
{
	this->bIsPlaying = false;
	this->bFinished = false;
	this->bIsReverse = false;
	this->fTicks = 0.0f;
	this->nFrameIndex = 0;
}

void Animation::step(float fDeltaTime)
{
	if (!this->bIsPlaying || this->vecFrames.empty()) return;

	this->fTicks += fDeltaTime;
	float ticksPerFrame = this->getTicksPerFrame();
	if (this->fTicks < ticksPerFrame) return;
	this->fTicks = std::fmod(this->fTicks, ticksPerFrame);

	unsigned int nLast = (unsigned int)this->vecFrames.size() - 1;

	switch (this->EType)
	{
	case AnimationType::ONCE:
		if (this->nFrameIndex < nLast) this->nFrameIndex++;
		if (this->nFrameIndex >= nLast) this->bFinished = true;
		break;

	case AnimationType::LOOP:
		this->nFrameIndex = (this->nFrameIndex >= nLast) ? 0 : this->nFrameIndex + 1;
		break;

	case AnimationType::YOYO:
	case AnimationType::YOYO_ONCE:
		if (!this->bIsReverse)
		{
			if (this->nFrameIndex < nLast) this->nFrameIndex++;
			if (this->nFrameIndex >= nLast) this->bIsReverse = true;
		}
		else
		{
			if (this->nFrameIndex > 0) this->nFrameIndex--;
			if (this->nFrameIndex == 0)
			{
				if (this->EType == AnimationType::YOYO) this->bIsReverse = false;
				else this->bFinished = true;   //YOYO_ONCE: back at the start, done
			}
		}
		break;
	}
}

bool Animation::finished() const
{
	return this->bFinished;
}

void Animation::setFrameRate(Uint8 nFrameRate)
{
	this->nFrameRate = (nFrameRate > 0) ? nFrameRate : 1;
}

void Animation::setType(AnimationType EType)
{
    this->EType = EType;
}

void Animation::setNextState(std::string strNextState)
{
    this->strNextState = strNextState;
}

void Animation::setOnAnimFinished(OnAnimFinished EOnFinished)
{
	this->EOnFinished = EOnFinished;
}

bool Animation::isPlaying() const
{
	return this->bIsPlaying;
}

std::string Animation::getName() const
{
    return this->strName;
}

SDL_Texture* Animation::getCurrentFrame() const
{
	if (this->vecFrames.empty()) return NULL;
	if (this->nFrameIndex >= this->vecFrames.size()) return this->vecFrames.back();
	return this->vecFrames[this->nFrameIndex];
}

unsigned int Animation::getFrameCount()
{
    return this->vecFrames.size();
}

float Animation::getTicksPerFrame() const
{
    return 1.0f / this->nFrameRate;
}

float Animation::getFrameRate() const
{
    return this->nFrameRate;
}

AnimationType Animation::getType() const
{
    return this->EType;
}

OnAnimFinished Animation::getOnAnimFinished() const
{
	return this->EOnFinished;
}

std::string Animation::getNextState() const
{
    return this->strNextState;
}

void Animation::stopAtEnd()
{
	this->stop();
	if (!this->vecFrames.empty())
		this->nFrameIndex = (unsigned int)this->vecFrames.size() - 1;
}

// replaces the frames and restarts from the first one
void Animation::setFrames(std::vector<SDL_Texture*> vecFrames)
{
	this->vecFrames = vecFrames;
	this->nFrameIndex = 0;
	this->fTicks = 0;
	this->bFinished = false;
}