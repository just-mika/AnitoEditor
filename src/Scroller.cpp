#include "Scroller.h"

Scroller::Scroller(std::string strName, Text* pScrollableText):AComponent(strName, ComponentType::SCRIPT)
{
	this->pScrollableText = pScrollableText;
	this->bDragging = false;
    this->bVert = false;
    this->bHori = false;

    this->bScrolling = false;
    this->fPressY = 0.0f;
}

Scroller::~Scroller()
{
}

void Scroller::perform()
{
	ButtonInput* pInput = (ButtonInput*)pOwner->findComponentByName("ButtonInput");
	TextRenderer* pTextRenderer = (TextRenderer*)this->pScrollableText->findComponentByName("TextRenderer");
	
    if (pInput->getDragging())
    {
        if (!bDragging)
        {
            // the press just started
            bDragging = true;
            bScrolling = false;
            this->fPressY = pInput->getMousePos().y;
            this->fPrevMouseX = pInput->getMousePos().x;
            this->fPrevMouseY = pInput->getMousePos().y;

            // the first drag starts from the editor-placed position: that's the top
            if (!this->bHasTop)
            {
                this->fTopY = this->pScrollableText->getLocalPos().y;
                this->bHasTop = true;
            }
        }
        else
        {
            // a click with a little jitter isn't a scroll
            if (!bScrolling)
            {
                if (SDL_fabsf(pInput->getMousePos().y - this->fPressY) < kDragThreshold)
                    return;
                bScrolling = true;
                this->fPrevMouseY = pInput->getMousePos().y;   //start from here, no jump
            }

            float fDeltaX = 0;
            float fDeltaY = 0;
            if(this->bHori)
            {
                fDeltaX = pInput->getMousePos().x - this->fPrevMouseX;
                this->fPrevMouseX = pInput->getMousePos().x;
            }
            if(this->bVert)
            {
                fDeltaY = pInput->getMousePos().y - this->fPrevMouseY;
                this->fPrevMouseY = pInput->getMousePos().y;
            }

            float fX = this->pScrollableText->getPos().x + fDeltaX;

            float fY = this->pScrollableText->getPos().y + fDeltaY;
            this->pScrollableText->setPos(Vector2D(fX, fY));

            // keep the text inside the window: no scrolling above the first line
            // or below the last one (in the text's local units)
            SpriteRenderer* pWindowR = (SpriteRenderer*)pOwner->findComponentByName("SpriteRenderer");
            float windowH = pWindowR->getTexSize().y;
            float textH = pTextRenderer->getTexSize().y * this->pScrollableText->getLocalScale().y;

            float maxY = this->fTopY;                      //first line at the top
            float minY = windowH - this->fTopY - textH;    //last line at the bottom
            if (minY > maxY) minY = maxY;                  //shorter than the window: no scrolling

            Vector2D local = this->pScrollableText->getLocalPos();
            local.y = SDL_clamp(local.y, minY, maxY);
            this->pScrollableText->setLocalPos(local);

        }
    }
    else
    {
        bDragging = false;
    }

}

void Scroller::setHori(bool bHori)
{
    this->bHori = bHori;
}

void Scroller::setVert(bool bVert)
{
    this->bVert = bVert;
}
