#pragma once
#include "AGameObject.h"
#include "TextRenderer.h"
class Text:public AGameObject
{
private:
	std::string strFontName;
	std::string strMessage;
	int nFontSize;
	SDL_Color color;
public:
	Text(const std::string& strName, std::string strFontName = "JainiPurva-Regular.ttf", int nFontSize = 45, float fRot = 0.0f, bool bFlipX = false);
	~Text() override;

	void setMessage(const std::string& strMessage);
	std::string getMessage();
	void setColor(SDL_Color color);
	SDL_Color getColor() const { return color; }
	int getFontSize() const { return nFontSize; }
	void setFontSize(int nFontSize);
	void setPivot(Vector2D fVecPivot);
	void initialize() override;
};

