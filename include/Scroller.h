#pragma once
#include "AComponent.h"
#include "ButtonInput.h"
#include "Text.h"
#include "TextRenderer.h"
class Scroller:public AComponent
{
	private:
		Text* pScrollableText;
		bool bDragging;
		float fPrevMouseY;
		float fPrevMouseX;
		bool bVert;
		bool bHori;

		float fTopY;    //the text's starting local y (the top limit)
		bool bHasTop;   //fTopY has been recorded

		bool bScrolling;              //moved past the threshold during this press
		float fPressY;                //where the press started (y)
		static constexpr float kDragThreshold = 10.0f;
	public:
		Scroller(std::string strName, Text* pScrollableText);
		~Scroller();
	public:

		// Inherited via AComponent
		void perform() override;
	public:
		void setHori(bool bHori);
		void setVert(bool bVert);
};

