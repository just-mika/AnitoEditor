#pragma once
#include "SDL3_ttf/SDL_ttf.h"
#include "ARenderer.h"
#include "FontManager.h"
#include "RendererContext.h"
#include "AGameObject.h"
#include "SpriteRenderer.h"
class TextRenderer:public ARenderer
{
private:
    std::string strFontType;
    int nFontSize;
    std::string strText;

	SDL_Texture* pTexture;
    SDL_FRect mDestRect;

    bool flipX, flipY;
    double dAngle;
    Vector2D texSize;
    Vector2D pivot;
    bool bCropEnabled;
    float fScrollOffset;
    float fViewHeight;
    int nWrapWidth = 0;

public:
	TextRenderer();
	~TextRenderer();
	void initialize(); // (when all resources are guaranteed to be loaded)
    void onAttach() override; 

	void perform() override;
    void drawWidget() override;

    void loadFromText(std::string fontType, int nFontSize, std::string strText, SDL_Color textColor);
    void resetText();
    void setColor(SDL_Color color) override;

    void setPivot(Vector2D pivot);
    void setWrapWidth(int nWidth) { nWrapWidth = nWidth; }
    int getWrapWidth() const { return nWrapWidth; }
    Vector2D getTexSize() const { return texSize; }
    float getTextHeight();
    SDL_FRect getRect() const;

    void setCropEnabled (bool bbCropEnabled);
    void setViewHeight(float fViewHeight);
    float getViewHeight();
    void setScrollOffset(float fOffset);
    float getScrollOffset();
};

