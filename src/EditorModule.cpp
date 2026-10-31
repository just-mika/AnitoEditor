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

#include "ACollider.h"
#include "LevelDataManager.h"
#include "Settings.h"
#include "TextRenderer.h"

//gizmo size in screen pixels (stays the same at any zoom)
static const float GIZMO_SCALE = 1.75f;   //change this to resize the whole gizmo
static const float GIZMO_LENGTH = 80.0f * GIZMO_SCALE;
static const float GIZMO_SHAFT = 3.0f * GIZMO_SCALE;
static const float GIZMO_HEAD = 12.0f * GIZMO_SCALE;
static const float GIZMO_FREE = 16.0f * GIZMO_SCALE;
static const float GIZMO_GRAB = 14.0f * GIZMO_SCALE;

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

    if (Camera* c = CameraManager::getInstance()->getCurrentCamera())
        c->setCanvasInWorld(!this->bIsPlaying);
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
    this->eGizmoPart = GizmoPart::None;
    this->eState = InteractionState::Idle;
}

//helper to find the topmost hitted object recursively
AGameObject* Editor::EditorModule::hitTestObject(AGameObject* obj, const Vector2D& pos, bool bGroupPass)
{
    const auto& children = obj->getChildren();

    for (auto child = children.rbegin(); child != children.rend(); ++child)
    {
        AGameObject* hit = hitTestObject(*child, pos, bGroupPass);
        if (hit)
            return hit;
    }

    if (contains(obj, pos, bGroupPass))
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
                AGameObject* hit = nullptr;
                const auto& roots = GameObjectManager::getInstance()->getAllObjects();

                //the selected object's gizmo comes first, so it can be grabbed over other objects
                GizmoPart part = this->hitTestGizmo(this->mousePos);
                if (part != GizmoPart::None)
                {
                    AGameObject* selected = this->getSelectedObject();
                    this->eGizmoPart = part;
                    this->eState = InteractionState::Dragging;   //no hold delay: grabbing the gizmo is deliberate

                    if (selected->getIsScreenObject())
                        this->offsetPos = this->getMouseCanvasPos() - selected->getPos();
                    else
                        this->offsetPos = this->getMouseWorldPos() - selected->getPos();
                    break;
                }

                //first pass: things you can actually see; second pass: empty parents' group boxes
                for (auto root = roots.rbegin(); root != roots.rend() && !hit; ++root)
                    hit = hitTestObject(*root, this->mousePos, false);

                for (auto root = roots.rbegin(); root != roots.rend() && !hit; ++root)
                    hit = hitTestObject(*root, this->mousePos, true);

                this->setSelectedObject(hit);

                if (!hit)
                    this->eState = InteractionState::PanningCam; // drag on empty space = pan
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


    if (this->eState == InteractionState::Dragging) //drag object
    {
        //where the free drag would put the object
        Vector2D target;
        if (selected->getIsScreenObject())
            target = this->getMouseCanvasPos() - this->offsetPos;
        else
            target = this->getMouseWorldPos() - this->offsetPos;

        //an arrow keeps the other coordinate where it is
        Vector2D current = selected->getPos();
        if (this->eGizmoPart == GizmoPart::X)
            target.y = current.y;
        else if (this->eGizmoPart == GizmoPart::Y)
            target.x = current.x;

        selected->setPos(target);
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

// Screen-space box around an object and everything under it (sprites only).
// Returns false if nothing in the group is visible.
bool Editor::EditorModule::getGroupBounds(AGameObject* pObject, SDL_FRect& outBounds) const
{
    std::vector<AGameObject*> vecGroup = pObject->getChildrenRecursively();
    vecGroup.push_back(pObject);

    bool bHasBounds = false;
    for (AGameObject* pObj : vecGroup)
    {
        if (!pObj->isGloballyEnabled())
            continue;

        SpriteRenderer* pSprite = (SpriteRenderer*)pObj->findComponentByName("SpriteRenderer");
        if (!pSprite || !pSprite->getTexture())
            continue;

        SDL_FRect r = pSprite->getRect();
        if (!bHasBounds)
        {
            outBounds = r;
            bHasBounds = true;
        }
        else
        {
            SDL_GetRectUnionFloat(&outBounds, &r, &outBounds);
        }
    }
    return bHasBounds;
}

//draws the editor overlay on top of the game (called in the render loop)
void Editor::EditorModule::draw(SDL_Renderer* pRenderer)
{
    Camera* pCam = CameraManager::getInstance()->getCurrentCamera();
    AGameObject* selected = this->getSelectedObject();

    SDL_SetRenderDrawBlendMode(pRenderer, SDL_BLENDMODE_BLEND);

    //outline of the screen area (where screen objects live)
    SDL_FRect canvas = pCam->canvasToScreenRect({ 0, 0, gameWidth, gameHeight });
    SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 90);
    SDL_RenderRect(pRenderer, &canvas);

    if (!selected)
        return;

    //white outline around the selected object's group
    AGameObject* pGroup = selected->getParent() ? selected->getParent() : selected;
    SDL_FRect groupBounds;
    if (this->getGroupBounds(pGroup, groupBounds))
    {
        SDL_SetRenderDrawColor(pRenderer, 255, 255, 255, 127);
        SDL_RenderRect(pRenderer, &groupBounds);
    }

    //a selected empty parent has no sprite to highlight itself,
    //so draw the cyan selection box around its children instead
    SDL_FRect ownRect;
    SDL_FRect bounds;
    if (!this->getClickRect(selected, ownRect) && this->getGroupBounds(selected, bounds))
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

    this->drawGizmo(pRenderer);
    // MetricsManager::getInstance()->drawGUI();
}

// The screen rect you click to select an object: its sprite, its text, or its collider box.
// Returns false if the object draws nothing of its own (e.g. an empty parent).
bool Editor::EditorModule::getClickRect(AGameObject* pObject, SDL_FRect& outRect) const
{
    SpriteRenderer* pSprite = (SpriteRenderer*)pObject->findComponentByName("SpriteRenderer");
    if (pSprite && pSprite->getTexture())
    {
        outRect = pSprite->getRect();
        return true;
    }

    TextRenderer* pText = (TextRenderer*)pObject->findComponentByName("TextRenderer");
    if (pText)
    {
        outRect = pText->getRect();
        return true;
    }

    //invisible objects with a collider (e.g. Platform): click their collider box
    for (AComponent* pComp : pObject->getAllComponents())
    {
        if (ACollider* pCollider = dynamic_cast<ACollider*>(pComp))
        {
            Camera* pCam = CameraManager::getInstance()->getCurrentCamera();
            SDL_FRect bounds = pCollider->getGlobalBounds();
            if (pObject->getIsScreenObject())
                outRect = pCam->canvasToScreenRect(bounds);
            else
                outRect = pCam->worldToScreenRect(bounds);
            return true;
        }
    }

    return false;
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
// Is the mouse over this object?
// First pass: the object's own sprite, text or collider.
// Group pass: only empty parents, using the box around their children.
bool Editor::EditorModule::contains(AGameObject* pObject, Vector2D mousePos, bool bGroupPass)
{
    if (!pObject->isGloballyEnabled() || pObject->isSystemObject())
        return false;

    SDL_FPoint p = { mousePos.x, mousePos.y };
    SDL_FRect rect;

    if (!bGroupPass)
        return this->getClickRect(pObject, rect) && SDL_PointInRectFloat(&p, &rect);

    //group pass: only objects that draw nothing themselves
    if (this->getClickRect(pObject, rect))
        return false;
    return this->getGroupBounds(pObject, rect) && SDL_PointInRectFloat(&p, &rect);
}

//gets the mouse position mapped inside the game world space
Vector2D Editor::EditorModule::getMouseWorldPos() const
{
    Camera* cam = CameraManager::getInstance()->getCurrentCamera();
    Vector2D mouseWorldPos = cam->screenToWorldPoint(this->mousePos);

    return mouseWorldPos;
}

Vector2D Editor::EditorModule::getMouseCanvasPos()
{
    Camera* cam = CameraManager::getInstance()->getCurrentCamera();
    return cam->worldToCanvasPoint(this->getMouseWorldPos());
}

// The selected object's pivot in screen space. False if nothing is selected.
bool Editor::EditorModule::getSelectedPivotScreen(Vector2D& outPos) const
{
    AGameObject* selected = this->selectedObject;
    if (!selected)
        return false;

    Camera* pCam = CameraManager::getInstance()->getCurrentCamera();
    if (selected->getIsScreenObject())
        outPos = pCam->canvasToScreenPoint(selected->getPos());
    else
        outPos = pCam->worldToScreenPoint(selected->getPos());
    return true;
}

// Where each part of the gizmo is on screen, for a gizmo at origin
Editor::GizmoShape Editor::EditorModule::getGizmoShape(const Vector2D& origin) const
{
    GizmoShape g;
    g.origin = origin;

    //red arrow: from the pivot to the right
    g.xArea = { origin.x, origin.y - GIZMO_GRAB * 0.5f, GIZMO_LENGTH, GIZMO_GRAB };

    //green arrow: from the pivot upward (screen y goes down, so "up" is minus)
    g.yArea = { origin.x - GIZMO_GRAB * 0.5f, origin.y - GIZMO_LENGTH, GIZMO_GRAB, GIZMO_LENGTH };

    //free-move square, just up and right of the pivot, between the arrows
    g.freeArea = { origin.x, origin.y - GIZMO_FREE, GIZMO_FREE, GIZMO_FREE };

    return g;
}

// fills a triangle (used for the arrowheads)
static void fillTriangle(SDL_Renderer* pRenderer, SDL_FPoint a, SDL_FPoint b, SDL_FPoint c, SDL_FColor color)
{
    SDL_Vertex v[3];
    v[0] = { a, color,{ 0, 0 } };
    v[1] = { b, color,{ 0, 0 } };
    v[2] = { c, color,{ 0, 0 } };
    SDL_RenderGeometry(pRenderer, nullptr, v, 3, nullptr, 0);
}

// Draws the move gizmo on the selected object
void Editor::EditorModule::drawGizmo(SDL_Renderer* pRenderer)
{
    Vector2D origin;
    if (!this->getSelectedPivotScreen(origin))
        return;

    GizmoShape g = this->getGizmoShape(origin);
    float tipX = origin.x + GIZMO_LENGTH;   //end of the red arrow
    float tipY = origin.y - GIZMO_LENGTH;   //end of the green arrow

    SDL_FColor red = { 0.90f, 0.25f, 0.25f, 1.0f };
    SDL_FColor green = { 0.35f, 0.85f, 0.35f, 1.0f };

    //the part being dragged turns yellow
    SDL_FColor active = { 1.0f, 0.9f, 0.3f, 1.0f };
    if (this->eGizmoPart == GizmoPart::X) red = active;
    if (this->eGizmoPart == GizmoPart::Y) green = active;

    //red X arrow: shaft, then head
    SDL_FRect xShaft = { origin.x, origin.y - GIZMO_SHAFT * 0.5f, GIZMO_LENGTH - GIZMO_HEAD, GIZMO_SHAFT };
    SDL_SetRenderDrawColorFloat(pRenderer, red.r, red.g, red.b, red.a);
    SDL_RenderFillRect(pRenderer, &xShaft);
    fillTriangle(pRenderer,
        { tipX, origin.y },
        { tipX - GIZMO_HEAD, origin.y - GIZMO_HEAD * 0.5f },
        { tipX - GIZMO_HEAD, origin.y + GIZMO_HEAD * 0.5f }, red);

    //green Y arrow: shaft, then head
    SDL_FRect yShaft = { origin.x - GIZMO_SHAFT * 0.5f, tipY + GIZMO_HEAD, GIZMO_SHAFT, GIZMO_LENGTH - GIZMO_HEAD };
    SDL_SetRenderDrawColorFloat(pRenderer, green.r, green.g, green.b, green.a);
    SDL_RenderFillRect(pRenderer, &yShaft);
    fillTriangle(pRenderer,
        { origin.x, tipY },
        { origin.x - GIZMO_HEAD * 0.5f, tipY + GIZMO_HEAD },
        { origin.x + GIZMO_HEAD * 0.5f, tipY + GIZMO_HEAD }, green);

    //free-move square: semi-transparent fill with an outline
    SDL_SetRenderDrawColor(pRenderer, 255, 220, 80, 90);
    SDL_RenderFillRect(pRenderer, &g.freeArea);
    SDL_SetRenderDrawColor(pRenderer, 255, 220, 80, 255);
    SDL_RenderRect(pRenderer, &g.freeArea);
}

// Which part of the selected object's gizmo is under the mouse (None if no selection or no hit)
Editor::GizmoPart Editor::EditorModule::hitTestGizmo(const Vector2D& mouse) const
{
    Vector2D origin;
    if (!this->getSelectedPivotScreen(origin))
        return GizmoPart::None;

    GizmoShape g = this->getGizmoShape(origin);
    SDL_FPoint p = { mouse.x, mouse.y };

    //the square overlaps the start of both arrows, so it's checked first
    if (SDL_PointInRectFloat(&p, &g.freeArea)) return GizmoPart::Free;
    if (SDL_PointInRectFloat(&p, &g.xArea))    return GizmoPart::X;
    if (SDL_PointInRectFloat(&p, &g.yArea))    return GizmoPart::Y;
    return GizmoPart::None;
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

    if (Camera* c = CameraManager::getInstance()->getCurrentCamera())
        c->setCanvasInWorld(!this->bIsPlaying);
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