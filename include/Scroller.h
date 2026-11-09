#pragma once
#include "AComponent.h"
#include "ButtonInput.h"
#include "Text.h"
#include "TextRenderer.h"
class Scroller:public AComponent
{
	private:
		static constexpr float kDragThreshold = 10.0f;
		Text* pScrollableText;
		bool bVert;
		bool bHori;

		bool bDragging; //mouse is down on the text
		bool bScrolling; 
		float fPressY;
		float fPrevMouseY;
		float fPrevMouseX;
		

		float fTopY;    //the text's starting local y (the top limit)
		bool bHasTop;   //if fTopY has been recorded

	public:
		Scroller(std::string strName, Text* pScrollableText);
		~Scroller();
	public:
		// Inherited via AComponent
		void perform() override;
		void setHori(bool bHori);
		void setVert(bool bVert);
};

