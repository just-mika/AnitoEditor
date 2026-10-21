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
#include "Settings.h"

bool showWidgets = false;

bool Editor::EditorModule::isPlaying() const {
    return this->bIsPlaying;
}

void Editor::EditorModule::togglePlayMode(){
    this->bIsPlaying = !this->bIsPlaying;

    // Drop any in-progress pan/drag/pinch so it can't carry across the mode switch
    this->cancelInteraction();
    this->mapFingers.clear();
    this->bIsPinching = false;

    Camera* cam = CameraManager::getInstance()->getCurrentCamera();
    if (cam)
    {
        if (this->bIsPlaying)
        {
            // Remember the editor view, then play at the default zoom
            this->editorCamPos = cam->getPos();
            this->editorCamScale = cam->getScale();
            cam->setScale(Vector2D(1.0f, 1.0f));
        }
        else
        {
            // Back to editing: restore the view the user had
            cam->setPos(this->editorCamPos);
            cam->setScale(this->editorCamScale);
        }
    }
    showWidgets = !this->bIsPlaying;
    UIManager::getInstance()->setEnabled("INSPECTOR_SCREEN", !this->bIsPlaying);
    UIManager::getInstance()->setEnabled("HIERARCHY_SCREEN", !this->bIsPlaying);
    UIManager::getInstance()->setEnabled("OBJECT_PALETTE", !this->bIsPlaying);
    AudioManager::getInstance()->setVolume(AudioGroupTag::MUSIC, this->bIsPlaying ? 1.0f : 0.0f);
}

Vector2D Editor::EditorModule::windowToLogical(float x, float y) const
{
    SDL_Renderer* renderer = RendererContext::getInstance()->getRenderer();
    float lx = x, ly = y;
    if (renderer) SDL_RenderCoordinatesFromWindow(renderer, x, y, &lx, &ly);
    uiToGame(lx, ly, lx, ly);          // this line is the fix
    return Vector2D(lx, ly);
}

// Finger events give NORMALIZED (0..1) coordinates over the whole window.
Vector2D Editor::EditorModule::fingerToLogical(float nx, float ny) const
{
    SDL_Renderer* renderer = RendererContext::getInstance()->getRenderer();
    if (!renderer) return Vector2D(nx * gameWidth, ny * gameHeight);

    int w = 0, h = 0;
    SDL_GetWindowSize(SDL_GetRenderWindow(renderer), &w, &h);
    return windowToLogical(nx * w, ny * h);
}

// screenDelta is in logical screen pixels. Dragging right moves the camera left;
// dragging down moves the camera up in world space (world +Y is up).
void Editor::EditorModule::panCamera(const Vector2D& screenDelta)
{
    Camera* cam = CameraManager::getInstance()->getCurrentCamera();
    Vector2D scale = cam->getScale();
    Vector2D pos = cam->getPos();

    pos.x -= screenDelta.x * scale.x;
    pos.y += screenDelta.y * scale.y;
    cam->setPos(pos);
}

// Zooms while keeping the world point under screenPivot fixed on screen.
void Editor::EditorModule::zoomCamera(float fFactor, const Vector2D& screenPivot)
{
    Camera* cam = CameraManager::getInstance()->getCurrentCamera();

    Vector2D worldBefore = cam->screenToWorldPoint(screenPivot);

    float fNewScale = SDL_clamp(cam->getScale().x * fFactor, this->fMinScale, this->fMaxScale);
    cam->setScale(Vector2D(fNewScale, fNewScale));

    Vector2D worldAfter = cam->screenToWorldPoint(screenPivot);
    cam->setPos(cam->getPos() + (worldBefore - worldAfter));
}

bool Editor::EditorModule::isPointerOverUI(const Vector2D& logicalPos) const
{
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (!g) return false;

    const ImVec2 p(logicalPos.x * (gameViewDst.w / gameWidth) + gameViewDst.x,
        logicalPos.y * (gameViewDst.h / gameHeight) + gameViewDst.y);

    for (ImGuiWindow* w : g->Windows)
    {
        if (!w->Active || w->Hidden) continue;
        if (w->Flags & ImGuiWindowFlags_ChildWindow) continue;     // inside its parent's rect anyway
        if (w->Flags & ImGuiWindowFlags_NoMouseInputs) continue;   // tooltips, drag previews, overlays
        if (w->Rect().Contains(p)) return true;                    // a collapsed window is just its title bar
    }
    return false;
}

void Editor::EditorModule::cancelInteraction()
{
    this->dTicks = 0.0;
    this->bIsHolding = false;
    this->bIsDragging = false;
    this->bIsMovingCam = false;
}

void Editor::EditorModule::updatePinch()
{
    if (this->mapFingers.size() != 2) { this->bIsPinching = false; return; }

    auto it = this->mapFingers.begin();
    Vector2D a = it->second; ++it;
    Vector2D b = it->second;

    Vector2D center = (a + b) * 0.5f;
    float fDist = (a - b).Magnitude();

    if (this->bIsPinching && this->fLastPinchDist > 1.0f && fDist > 1.0f)
    {
        // Fingers spreading apart (dist grows) => zoom in => scale shrinks
        this->zoomCamera(this->fLastPinchDist / fDist, center);
        this->panCamera(center - this->lastPinchCenter);
    }

    this->bIsPinching = true;
    this->fLastPinchDist = fDist;
    this->lastPinchCenter = center;
}

void Editor::EditorModule::processEditorInput(const SDL_Event* eEvent)
{
    InspectorScreen* inspector = static_cast<InspectorScreen*>(
            UIManager::getInstance()->getUIScreen("INSPECTOR_SCREEN"));
    ImGuiIO& io = ImGui::GetIO();

    switch (eEvent->type)
    {
        case SDL_EVENT_MOUSE_WHEEL:
        {
            //SDL_Log("[Editor] WHEEL y=%.2f (integer_y=%d) wantCaptureMouse=%d",
            //        eEvent->wheel.y, eEvent->wheel.integer_y, (int)io.WantCaptureMouse);

            Vector2D wheelPos = this->windowToLogical(eEvent->wheel.mouse_x, eEvent->wheel.mouse_y);
            if (this->isPointerOverUI(wheelPos)) break;

            // wheel.y > 0 = scroll up = zoom in = smaller scale
            Vector2D pivot = this->windowToLogical(eEvent->wheel.mouse_x, eEvent->wheel.mouse_y);
            this->zoomCamera(powf(this->fWheelZoomStep, eEvent->wheel.y), pivot);

            //SDL_Log("[Editor] camera scale now %.3f",
            //        CameraManager::getInstance()->getCurrentCamera()->getScale().x);
            break;
        }

            // Keyboard zoom fallback (also handy on emulators that don't send wheel events).
            // '=' / '+' / numpad+ zoom in, '-' / numpad- zoom out. Zooms around screen center.
        case SDL_EVENT_KEY_DOWN:
        {
            if (io.WantTextInput) break;

            float fFactor = 0.0f;
            if (eEvent->key.key == SDLK_EQUALS || eEvent->key.key == SDLK_PLUS ||
                eEvent->key.key == SDLK_KP_PLUS)
                fFactor = this->fWheelZoomStep;
            else if (eEvent->key.key == SDLK_MINUS || eEvent->key.key == SDLK_KP_MINUS)
                fFactor = 1.0f / this->fWheelZoomStep;

            if (fFactor != 0.0f)
                this->zoomCamera(fFactor, Vector2D(gameWidth * 0.5f, gameHeight * 0.5f));
            break;
        }

        case SDL_EVENT_MOUSE_MOTION:
        {
            Vector2D newPos = this->windowToLogical(eEvent->motion.x, eEvent->motion.y);
            Vector2D delta = newPos - this->mousePos;
            this->mousePos = newPos;
            if (inspector) inspector->setMousePos(this->mousePos);

            // While two fingers are down, the pinch handler owns the camera
            if (this->bIsMovingCam && this->mapFingers.size() < 2)
                this->panCamera(delta);
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        {
            // IMPORTANT for touch: a finger tap has no hover motion before it, so
            // mousePos would be stale. Always refresh it from the press itself.
            this->mousePos = this->windowToLogical(eEvent->button.x, eEvent->button.y);
            if (inspector) inspector->setMousePos(this->mousePos);

            if (this->isPointerOverUI(this->mousePos)) break; // clicked on an ImGui panel
            if (this->mapFingers.size() >= 2) break; // pinching, not clicking

            if (eEvent->button.button == SDL_BUTTON_LEFT)
            {
                this->bIsHolding = true;
                bool bHitSomething = false;

                std::vector<AGameObject*> vecObject = GameObjectManager::getInstance()->getAllObjects();
                for (auto parent : vecObject)
                {
                    std::vector<AGameObject*> vecObjectTree = parent->getChildrenRecursively();
                    vecObjectTree.push_back(parent);
                    for (auto obj : vecObjectTree)
                    {
                        if (contains(obj, this->mousePos))
                        {
                            bHitSomething = true;
                            this->setSelectedObject(obj);
                            if (obj->getIsScreenObject())
                                this->offsetPos = this->mousePos - obj->getPos();
                            else
                                this->offsetPos = this->getMouseWorldPos() - obj->getPos();
                        }
                    }
                }

                if (!bHitSomething)
                {
                    this->setSelectedObject(nullptr);
                    this->bIsMovingCam = true;   // drag on empty space = pan
                }
            }
            else if (eEvent->button.button == SDL_BUTTON_RIGHT ||
                     eEvent->button.button == SDL_BUTTON_MIDDLE)
            {
                this->bIsMovingCam = true;
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP:
            this->cancelInteraction();
            break;

            // ---- Touch: two fingers = pinch zoom + pan ----
        case SDL_EVENT_FINGER_DOWN:
            this->mapFingers[eEvent->tfinger.fingerID] =
                    this->fingerToLogical(eEvent->tfinger.x, eEvent->tfinger.y);

            if (this->mapFingers.size() >= 2)
            {
                // Second finger landed: abort whatever the first finger started
                // (SDL's synthetic mouse events would otherwise select/drag/pan).
                this->cancelInteraction();
                this->bIsPinching = false;
                this->updatePinch();   // initialise baseline distance/center
            }
            break;

        case SDL_EVENT_FINGER_MOTION:
            if (this->mapFingers.count(eEvent->tfinger.fingerID))
            {
                this->mapFingers[eEvent->tfinger.fingerID] =
                        this->fingerToLogical(eEvent->tfinger.x, eEvent->tfinger.y);
                this->updatePinch();
            }
            break;

        case SDL_EVENT_FINGER_UP:
        case SDL_EVENT_FINGER_CANCELED:
            this->mapFingers.erase(eEvent->tfinger.fingerID);
            this->bIsPinching = false;
            if (this->mapFingers.size() < 2) this->cancelInteraction();
            break;

        default:
            break;
    }
}

void Editor::EditorModule::updateGameObjects(float fDeltaTime)
{
    // Camera pan/zoom is now applied directly in processEditorInput (it is
    // event-driven and must not be scaled by frame time).
    AGameObject* selected = this->getSelectedObject();
    if (!selected) return;

    if (this->bIsHolding)
    {
        this->dTicks += fDeltaTime;
        if (this->dTicks >= D_MAX_DRAG_TICKS)
        {
            this->dTicks = 0.0;
            this->bIsDragging = true;
        }
    }

    if (this->bIsDragging)
    {
        if (selected->getIsScreenObject())
            selected->setPos(this->mousePos - this->offsetPos);
        else
            selected->setPos(this->getMouseWorldPos() - this->offsetPos);
    }
}

bool Editor::EditorModule::hasVisibleSprite(AGameObject* pObject)
{
    if (!pObject->getComponents(ComponentType::ANIMATOR).empty()) return true;

    for (auto pComp : pObject->getComponents(ComponentType::RENDERER))
    {
        SpriteRenderer* pSprite = dynamic_cast<SpriteRenderer*>(pComp);
        if (pSprite) { if (pSprite->getTexture() != nullptr) return true; }
        else return true;
    }
    return false;
}

void Editor::EditorModule::drawEditor(SDL_Renderer* pRenderer)
{
    Camera* pCam = CameraManager::getInstance()->getCurrentCamera();

    //SDL_RenderLine(pRenderer, 0, 540, 1920, 540);

    std::vector<AGameObject*> vecObject = GameObjectManager::getInstance()->getAllObjects();
    for (auto parent : vecObject)
    {
        std::vector<AGameObject*> vecObjectTree = parent->getChildrenRecursively();
        vecObjectTree.push_back(parent);
        for (auto obj : vecObjectTree)
        {
            if (obj->isGloballyEnabled() && hasVisibleSprite(obj))
            {
                SDL_FRect mDestRect{};
                mDestRect.w = this->fTexW;
                mDestRect.h = this->fTexH;

                Vector2D screenPos;
                if (obj->getIsScreenObject()) screenPos = obj->getPos();
                else screenPos = pCam->worldToScreenPoint(obj->getPos());

                mDestRect.x = screenPos.x - (mDestRect.w * 0.5f);
                mDestRect.y = screenPos.y - (mDestRect.h * 0.5f);

                SDL_RenderTexture(pRenderer, this->pWidget, NULL, &mDestRect);
            }
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

bool Editor::EditorModule::contains(AGameObject* pObject, Vector2D mousePos)
{
    if (!this->pWidget) return false;
    if (!pObject->isGloballyEnabled() || !hasVisibleSprite(pObject)) return false;

    Camera* cam = CameraManager::getInstance()->getCurrentCamera();

    SDL_FRect pointRect = { mousePos.x, mousePos.y, 1, 1};
    SDL_FRect spriteRect = {};
    Vector2D objPos;

    if (pObject->getIsScreenObject()) objPos = pObject->getPos();
    else objPos = cam->worldToScreenPoint(pObject->getPos());
    //std::cout << objPos << " " << objScreenPos << "\n";

    spriteRect.x = objPos.x - (fTexW * 0.5f);
    spriteRect.y = objPos.y - (fTexH * 0.5f);
    spriteRect.w = fTexW;
    spriteRect.h = fTexH;


    return SDL_HasRectIntersectionFloat(&spriteRect, &pointRect);
}

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