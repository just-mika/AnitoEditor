#include "EditorModule.h"
#include "TextureManager.h"
#include "GameObjectManager.h"
#include "UIManager.h"
#include "InspectorScreen.h"
#include "CameraManager.h"
#include <iostream>
#include "MetricsManager.h"
#include "RendererContext.h"
#include "SpriteRenderer.h"
#include "AudioManager.h"
#include "Settings.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cmath>

#include "LevelDataManager.h"
#include "Settings.h"
#include "TextRenderer.h"

bool showWidgets = false;

bool Editor::EditorModule::isPlaying() const {
    return this->bIsPlaying;
}

//called to switch Play mode -> Editor mode and vice versa
void Editor::EditorModule::togglePlayMode(){
    this->bIsPlaying = !this->bIsPlaying;

    // Drop any in-progress pan/drag/pinch so it can't carry across the mode switch
    this->cancelInteraction();

    Camera* cam = CameraManager::getInstance()->getCurrentCamera();
    if (cam)
    {
        if (this->bIsPlaying)
        {
            // Record editor camera state then play at the default zoom
            this->editorCamPos = cam->getPos();
            this->editorCamScale = cam->getScale();
            cam->setScale(Vector2D(1.0f, 1.0f));
            cam->setPos(Vector2D(0.0f, 0.0f));
        }
        else
        {
            // Restore the previous editor camera state
            cam->setPos(this->editorCamPos);
            cam->setScale(this->editorCamScale);
        }
    }

    //only show at editor mode
    showWidgets = !this->bIsPlaying; 
    UIManager::getInstance()->setEnabled("INSPECTOR_SCREEN", !this->bIsPlaying);
    UIManager::getInstance()->setEnabled("HIERARCHY_SCREEN", !this->bIsPlaying);
    UIManager::getInstance()->setEnabled("OBJECT_PALETTE", !this->bIsPlaying);

    //only play music at play mode
    AudioManager::getInstance()->setVolume(AudioGroupTag::MUSIC, this->bIsPlaying ? 1.0f : 0.0f);

    if (this->bIsPlaying) 
        LevelDataManager::getInstance()->beginPlay();
    else 
    	LevelDataManager::getInstance()->endPlay();
}

// Takes mouse input, converts window coordinates to logical coordinates
Vector2D Editor::EditorModule::windowToLogical(float x, float y) const
{
    SDL_Renderer* renderer = RendererContext::getInstance()->getRenderer();
    float lx = x, ly = y;

    if (renderer) 
        SDL_RenderCoordinatesFromWindow(renderer, x, y, &lx, &ly);

    uiToGame(lx, ly, lx, ly);
    return Vector2D(lx, ly);
}

//pans the camera in a 2d space
void Editor::EditorModule::panCamera(const Vector2D& screenDelta)
{
    Camera* cam = CameraManager::getInstance()->getCurrentCamera();
    Vector2D scale = cam->getScale();
    Vector2D pos = cam->getPos();

    pos.x -= screenDelta.x * scale.x;
    pos.y += screenDelta.y * scale.y;
    cam->setPos(pos);
}

// zooms the camera in a 2d space, while keeping world point fixed on screen.
void Editor::EditorModule::zoomCamera(float fFactor, const Vector2D& screenPivot)
{
    Camera* cam = CameraManager::getInstance()->getCurrentCamera();
    Vector2D worldBefore = cam->screenToWorldPoint(screenPivot); //gets mouse position before zoom

    //multiply and set scale
    float fNewScale = SDL_clamp(cam->getScale().x * fFactor, this->fMinScale, this->fMaxScale);
    cam->setScale(Vector2D(fNewScale, fNewScale));

    Vector2D worldAfter = cam->screenToWorldPoint(screenPivot); //get same mouse position AFTER zoom
    cam->setPos(cam->getPos() + (worldBefore - worldAfter));
    //shift camera position by worldBefore - worldAfter to reflect drift caused by scaling
}

// checks if a point in the screen is over a UI panel
// mostly used to block editor input over the UI
bool Editor::EditorModule::isPointerOverUI(const Vector2D& logicalPos) const
{
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (!g) return false;

    const ImVec2 p(logicalPos.x * (gameViewDst.w / gameWidth) + gameViewDst.x,
        logicalPos.y * (gameViewDst.h / gameHeight) + gameViewDst.y);

    for (ImGuiWindow* w : g->Windows)
    {
        if (!w->Active || w->Hidden) continue; //skip inactive windows
        if (w->Flags & ImGuiWindowFlags_ChildWindow) continue; //skip subregions
        if (w->Flags & ImGuiWindowFlags_NoMouseInputs) continue; // skip tooltips, drag previews, overlays
        if (w->Rect().Contains(p)) return true; //point in screen is over a UI panel
    }
    return false;
}

//cancels editor interaction
void Editor::EditorModule::cancelInteraction()
{
    this->dTicks = 0.0;
    this->eState = InteractionState::Idle;
}

//helper to find the topmost hitted object recursively
AGameObject* Editor::EditorModule::hitTestObject(AGameObject* obj, const Vector2D& pos)
{
    const auto& children = obj->getChildren();

    for (auto child = children.rbegin(); child != children.rend(); ++child)
    {
        //recursive case: check if the hit object has children
        AGameObject* hit = hitTestObject(*child, pos);
        if (hit)
            return hit;
    }
    
    //base case: obj has no children
    if (contains(obj, pos))
        return obj;
	return nullptr;
}

//processes editor input from keyboard or mouse
void Editor::EditorModule::processEditorInput(const SDL_Event* eEvent)
{
    InspectorScreen* inspector = static_cast<InspectorScreen*>(
            UIManager::getInstance()->getUIScreen("INSPECTOR_SCREEN"));
    ImGuiIO& io = ImGui::GetIO();

    switch (eEvent->type)
    {
        case SDL_EVENT_MOUSE_WHEEL: //for zooming camera via mouse scroll
        {
            Vector2D wheelPos = this->windowToLogical(eEvent->wheel.mouse_x, eEvent->wheel.mouse_y);
            if (this->isPointerOverUI(wheelPos)) break;

            this->zoomCamera(powf(this->fWheelZoomStep, eEvent->wheel.y), wheelPos);
            break;
        }

        case SDL_EVENT_KEY_DOWN: //for zooming camera via +, -, and = keys
        {
            if (io.WantTextInput) break;
            float fFactor = 0.0f;

    		//zoom in
            if (eEvent->key.key == SDLK_EQUALS || eEvent->key.key == SDLK_PLUS ||
                eEvent->key.key == SDLK_KP_PLUS)
                fFactor = this->fWheelZoomStep;

            //zoom out
            else if (eEvent->key.key == SDLK_MINUS || eEvent->key.key == SDLK_KP_MINUS)
                fFactor = 1.0f / this->fWheelZoomStep;

            if (fFactor != 0.0f)
                this->zoomCamera(fFactor, Vector2D(gameWidth * 0.5f, gameHeight * 0.5f));
            break;
        }

        case SDL_EVENT_MOUSE_MOTION: //for camera panning
        {
            Vector2D newPos = this->windowToLogical(eEvent->motion.x, eEvent->motion.y);
            Vector2D delta = newPos - this->mousePos;
            this->mousePos = newPos;

            if (inspector) 
                inspector->setMousePos(this->mousePos);

            if (this->eState == InteractionState::PanningCam)
                this->panCamera(delta);
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            this->mousePos = this->windowToLogical(eEvent->button.x, eEvent->button.y);
        	
            if (this->isPointerOverUI(this->mousePos)) break; //ignore editor input if clicked on a UI panel

        	if (inspector) 
                inspector->setMousePos(this->mousePos);

            if (eEvent->button.button == SDL_BUTTON_LEFT)
            {
                this->eState = InteractionState::Holding;

                AGameObject* hit = nullptr;
                const auto& roots = GameObjectManager::getInstance()->getAllObjects();

                for (auto root = roots.rbegin(); root != roots.rend() && !hit; ++root)
                    hit = hitTestObject(*root, this->mousePos);

                this->setSelectedObject(hit);

                if (hit)
                {
                    if (hit->getIsScreenObject())
                        this->offsetPos = this->mousePos - hit->getPos();
                    else
                        this->offsetPos = this->getMouseWorldPos() - hit->getPos();
                }
				else
                {
                    this->eState = InteractionState::PanningCam; // drag on empty space = pan
                }
            }
            else if (eEvent->button.button == SDL_BUTTON_RIGHT ||
                     eEvent->button.button == SDL_BUTTON_MIDDLE)
            {
                this->eState = InteractionState::PanningCam;
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP:
            this->cancelInteraction();
            break;

        default:
            break;
    }
}

//updates object position from dragging
void Editor::EditorModule::updateObjectDragging(float fDeltaTime)
{
    AGameObject* selected = this->getSelectedObject();
    if (!selected) return;

    if (this->eState == InteractionState::Holding)
    {
        this->dTicks += fDeltaTime;
        if (this->dTicks >= D_MAX_DRAG_TICKS) //wait for max threshold before dragging
        {
            this->dTicks = 0.0;
            this->eState = InteractionState::Dragging;
        }
    }

    if (this->eState == InteractionState::Dragging) //drag object
    {
        if (selected->getIsScreenObject())
            selected->setPos(this->mousePos - this->offsetPos);
        else
            selected->setPos(this->getMouseWorldPos() - this->offsetPos);
    }
}

// Decides whether an object gets an editor handle (and can be clicked/dragged)
bool Editor::EditorModule::hasVisibleSprite(AGameObject* pObject)
{
    if (!pObject->getComponents(ComponentType::ANIMATOR).empty()) //all objects w/ animator component have a visible sprite
        return true;

    for (auto pComp : pObject->getComponents(ComponentType::RENDERER)) //find sprite component 
    {

        SpriteRenderer* pSprite = dynamic_cast<SpriteRenderer*>(pComp);
        if (pSprite) {
            if (pSprite->getTexture() != nullptr) //object has a loaded sprite
                return true;
        }
        else if (dynamic_cast<TextRenderer*>(pComp)) //text objects are always visible
            return true;
    }

    //no visible renderer of its own: still gets a handle if any child (or deeper) is visible
    for (AGameObject* pChild : pObject->getChildren())
    {
        if (pChild->getEnabled() && hasVisibleSprite(pChild))
            return true;
    }

    return false; //if spriteRender is not found = default no visible sprite.
}

// Screen rect of an object's editor handle
SDL_FRect Editor::EditorModule::getHandleRect(AGameObject* pObject, SDL_FRect* pOutBounds) const
{
    Camera* pCam = CameraManager::getInstance()->getCurrentCamera();

    Vector2D pos;

    if (pObject->getIsScreenObject())
        pos = pObject->getPos();
	else
		pos = pCam->worldToScreenPoint(pObject->getPos());

    SDL_FRect handle = { pos.x - fTexW * 0.5f, pos.y - fTexH * 0.5f, fTexW, fTexH };
    SDL_FRect bounds{};

    //checks if object is an empty container
    if (!pObject->findComponentByName("SpriteRenderer"))
    {
        bool bHasBounds = false;
        //traverse through children
        for (AGameObject* pDesc : pObject->getChildrenRecursively())
        {
            if (!pDesc->isGloballyEnabled()) //skip children currently disabled
                continue;

            SpriteRenderer* pSprite = (SpriteRenderer*)pDesc->findComponentByName("SpriteRenderer");
            
            //skip children with no sprite
        	if (!pSprite || !pSprite->getTexture()) 
                continue;

            //uses AABB Merging 
            SDL_FRect r = pSprite->getRect();
            if (!bHasBounds)
            {
                bounds = r; //child's rectangle area
                bHasBounds = true; 
            }
            else
            {
                //if bounds are already found (from prev. valid sprite,
                // compute the bound union of current and prev sprite
                SDL_GetRectUnionFloat(&bounds, &r, &bounds);
            }
        }

        if (bHasBounds)
        {
            handle.x = bounds.x - fTexW;   // put widget in the top-left corner
            handle.y = bounds.y - fTexH;
        }
    }

    if (pOutBounds)
        *pOutBounds = bounds; //pass bounds to the pointer
    return handle;
}

//draws editor (called in render loop)
void Editor::EditorModule::draw(SDL_Renderer* pRenderer)
{
    AGameObject* selected = this->getSelectedObject();

    std::vector<AGameObject*> vecObject = GameObjectManager::getInstance()->getAllObjects();
    
	//draw object selections
	for (auto parent : vecObject)
    {
        std::vector<AGameObject*> vecObjectTree = parent->getChildrenRecursively();
        vecObjectTree.push_back(parent);
        for (auto obj : vecObjectTree)
        {
            if (!obj->isGloballyEnabled() || !hasVisibleSprite(obj)) 
                continue; //skip disabled objects

            SDL_FRect bounds{};
            SDL_FRect handle = this->getHandleRect(obj, &bounds);

            // Outline around children
            if (bounds.w > 0 && bounds.h > 0)
            {
                //render outline
                SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 127);
                SDL_RenderRect(pRenderer, &bounds);

                if (obj == selected) //change color of outline for selected parent
                {
                    SDL_SetRenderDrawColor(pRenderer, 0, 230, 204, 255);
                    for (int i = 0; i < 2; i++)
                    {
                        SDL_FRect b = bounds;
                        b.x -= i;
                        b.y -= i;
                        b.w += i * 2.0f;
                        b.h += i * 2.0f;
                        SDL_RenderRect(pRenderer, &b);
                    }
                }
            }

            // Handle & widget is cyan when selected, normal colors otherwise
            if (obj == selected)
                SDL_SetTextureColorMod(this->pWidget, 0, 230, 204);
            else
                SDL_SetTextureColorMod(this->pWidget, 255, 255, 255);

            SDL_RenderTexture(pRenderer, this->pWidget, NULL, &handle);
        }
    }
    // MetricsManager::getInstance()->drawGUI();
}

void Editor::EditorModule::setSelectedObject(AGameObject* selectedObject)
{
    this->selectedObject = selectedObject;
}

AGameObject* Editor::EditorModule::getSelectedObject()
{
    return this->selectedObject;
}

// checks if the mouse position is currently inside the object's rectangle area 
bool Editor::EditorModule::contains(AGameObject* pObject, Vector2D mousePos)
{
    if (!this->pWidget) //no widget
        return false;
    //not enabled
    if (!pObject->isGloballyEnabled() || !hasVisibleSprite(pObject)) 
        return false;

    SDL_FPoint p = { mousePos.x, mousePos.y }; //mouse current position as SDL Point
    SDL_FRect handle = this->getHandleRect(pObject); //get rectangle area
    return SDL_PointInRectFloat(&p, &handle);
}

//gets the mouse position mapped inside the game world space
Vector2D Editor::EditorModule::getMouseWorldPos() const
{
    Camera* cam = CameraManager::getInstance()->getCurrentCamera();
    Vector2D mouseWorldPos = cam->screenToWorldPoint(this->mousePos);

    return mouseWorldPos;
}

/* * * * * * * * * * * * * * * * * * * * *
 *       SINGLETON-RELATED CONTENT       *
 * * * * * * * * * * * * * * * * * * * * */
Editor::EditorModule::EditorModule()
{
    TextureManager::getInstance()->load("editor/game_object_widget.png", "._widget");
    this->pWidget = TextureManager::getInstance()->get("._widget");
    SDL_GetTextureSize(this->pWidget, &this->fTexW, &this->fTexH);

    AudioManager::getInstance()->setVolume(AudioGroupTag::MUSIC, 0.0f);
    showWidgets = !this->bIsPlaying;
}

Editor::EditorModule* Editor::EditorModule::P_SHARED_INSTANCE = NULL;

void Editor::EditorModule::initialize()
{
    P_SHARED_INSTANCE = new EditorModule();
}

void Editor::EditorModule::destroy()
{
    delete P_SHARED_INSTANCE;
}

Editor::EditorModule* Editor::EditorModule::getInstance()
{
    return P_SHARED_INSTANCE;
}
/* * * * * * * * * * * * * * * * * * * * */