#include "InputManager.h"
#include "CameraManager.h"
#include "RendererContext.h"
#include "Settings.h"
InputManager* InputManager::P_SHARED_INSTANCE = nullptr;

void InputManager::initialize()
{
    if (!P_SHARED_INSTANCE)
    {
        P_SHARED_INSTANCE = new InputManager();
    }
}

void InputManager::destroy()
{
    delete P_SHARED_INSTANCE;
    P_SHARED_INSTANCE = nullptr;
}

InputManager* InputManager::getInstance()
{
    return P_SHARED_INSTANCE;
}

void InputManager::setLogicalSize(float x, float y)
{
    logicalX = x;
    logicalY = y;
}

void InputManager::processEvents(SDL_Event* e)
{
    switch (e->type)
    {

    case SDL_EVENT_KEY_DOWN:
    {
        if (!this->activeKeySet.contains(e->key.scancode) && !e->key.repeat)
        {
            this->activeKeySet.insert(e->key.scancode);
            this->frame++;
            if (debugKeyboard) SDL_Log("Key Down: %c, %ld", e->key.key, frame);
        }
        break;
    }

    case SDL_EVENT_KEY_UP:
    {
        if (this->activeKeySet.contains(e->key.scancode))
        {
            this->activeKeySet.erase(e->key.scancode);
            this->frame++;
            if (debugKeyboard) SDL_Log("Key Up: %c", e->key.key);
        }
        break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    {
        if (e->button.which == SDL_TOUCH_MOUSEID || e->button.button != SDL_BUTTON_LEFT) break;
        setMouseFinger(e->button.windowID, e->button.x, e->button.y);
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP:
    {
        if (e->button.which == SDL_TOUCH_MOUSEID || e->button.button != SDL_BUTTON_LEFT) break;
        activeTouchMap.erase(MOUSE_FINGER_ID);
        break;
    }

    case SDL_EVENT_MOUSE_MOTION:
    {
        if (e->motion.which == SDL_TOUCH_MOUSEID) break;
        if (activeTouchMap.contains(MOUSE_FINGER_ID))
            setMouseFinger(e->motion.windowID, e->motion.x, e->motion.y);
        break;
    }
    default:
        break;
    }
}

//simulates touch screen by converting mouse coordinates to touch input format
// then registers it as an actual touch input in activeTouchMap.
void InputManager::setMouseFinger(Uint32 windowID, float mouseX, float mouseY)
{
    SDL_Renderer* renderer = RendererContext::getInstance()->getRenderer();
    if (!renderer) return;

    SDL_Window* window = SDL_GetRenderWindow(renderer);
    int w = 1, h = 1;
    SDL_GetWindowSizeInPixels(window, &w, &h);

    SDL_TouchFingerEvent fakeTouch{};
    fakeTouch.fingerID = MOUSE_FINGER_ID;

    // Normalize absolute mouse pixels into [0.0, 1.0] range expected by touch math
    fakeTouch.x = mouseX / static_cast<float>(w);
    fakeTouch.y = mouseY / static_cast<float>(h);
    fakeTouch.pressure = 1.0f;

    activeTouchMap[MOUSE_FINGER_ID] = fakeTouch;
}

// Finds the active finger and scales its normalized touch coordinates directly to logical resolution (ignores camera/viewport offsets).
bool InputManager::getFingerRawPosition(Uint64 fingerId, float& outX, float& outY) const {
    auto it = activeTouchMap.find(fingerId);
    if (it == activeTouchMap.end())
    {
        return false;
    }

    outX = toLogicalX(it->second.x);
    outY = toLogicalY(it->second.y);
    return true;
}

// Finds the active finger and converts its normalized touch coordinates into active viewport/camera screen space.
bool InputManager::getFingerPosition(Uint64 fingerId, float& outX, float& outY) const
{
    auto it = activeTouchMap.find(fingerId);
    if (it == activeTouchMap.end())
    {
        return false;
    }

    Vector2D windowPos = rawToScreenPosition(it->second.x, it->second.y);
    Vector2D windowOffset = CameraManager::getInstance()->getWindowOffset();

    outX = windowPos.x;
    outY = windowPos.y;
    return true;
}

// Transforms raw normalized touch input into viewport screen space using window dimensions and camera offsets.
Vector2D InputManager::rawToScreenPosition(float x, float y) const
{
    SDL_Renderer* renderer = RendererContext::getInstance()->getRenderer();
    if (!renderer) return Vector2D(x, y);

    SDL_Window* window = SDL_GetRenderWindow(renderer);
    int w, h;
    SDL_GetWindowSizeInPixels(window, &w, &h);

    float winX = x * w;
    float winY = y * h;

    float lx = 0.0f, ly = 0.0f;
    SDL_RenderCoordinatesFromWindow(renderer, winX, winY, &lx, &ly);

    uiToGame(lx, ly, lx, ly);
    return Vector2D(lx, ly);
}

bool InputManager::isKeyDown(const SDL_Scancode& key) const
{
    bool isKeyDown = this->activeKeySet.contains(key);
    return isKeyDown;
}

bool InputManager::isMouseButtonDown(Uint8 button) const
{
    SDL_MouseButtonFlags mouseState = SDL_GetMouseState(nullptr, nullptr);
    return (mouseState & SDL_BUTTON_MASK(button)) != 0;
}

bool InputManager::getMousePosition(float& outX, float& outY) const
{
    SDL_GetMouseState(&outX, &outY);
    return true;
}

bool InputManager::hasActiveTouches() const
{
    return !activeTouchMap.empty();
}

bool InputManager::isAnyFingerInRect(SDL_FRect fRect, Uint64& fingerId) const
{
    for (auto touch : getActiveTouches())
    {
        Vector2D windowPos = rawToScreenPosition(touch.x, touch.y);
        SDL_FPoint point = {windowPos.x, windowPos.y};
        fingerId = touch.fingerID;
        if (SDL_PointInRectFloat(&point, &fRect))
            return true;
    }

    return false;
}

std::vector<SDL_TouchFingerEvent> InputManager::getActiveTouches() const
{
    std::vector<SDL_TouchFingerEvent> touches;
    touches.reserve(activeTouchMap.size());

    for (const auto& pair : activeTouchMap)
    {
        touches.push_back(pair.second);
    }

    return touches;
}

float InputManager::toLogicalX(float normalizedX) const
{
    if (logicalX <= 0.0f)
    {
        return normalizedX;
    }
    return normalizedX * logicalX;
}

float InputManager::toLogicalY(float normalizedY) const
{
    if (logicalY <= 0.0f)
    {
        return normalizedY;
    }
    return normalizedY * logicalY;
}