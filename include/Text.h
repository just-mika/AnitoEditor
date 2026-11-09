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
	void initialize() override;

	void setMessage(const std::string& strMessage);
	std::string getMessage();

	void setFontName(const std::string& strFontName);
	const std::string& getFontName() const { return strFontName; }
	void setFontSize(int nFontSize);
	int getFontSize() const { return nFontSize; }
	void setColor(SDL_Color color);
	SDL_Color getColor() const { return color; }

	void setWrapWidth(int nWidth);
	int getWrapWidth();
	void setPivot(Vector2D fVecPivot);
};

