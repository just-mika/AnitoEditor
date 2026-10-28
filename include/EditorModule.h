#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <unordered_map>
#include "Vector2D.h"
#include "AUIScreen.h"
#include "AGameObject.h"
#include "EnumInteractionState.h"

namespace Editor
{
    class EditorModule
    {
    private:
        SDL_Texture* pWidget = NULL;
        float fTexW = 64.0f;
        float fTexH = 64.0f;

        const double D_MAX_DRAG_TICKS = 0.2;
        double dTicks = 0.0;
        bool bIsPlaying = false;
        InteractionState eState = InteractionState::Idle;

        Vector2D mousePos;
        Vector2D offsetPos;

        // Camera zoom limits.
        float fMinScale = 0.1f;
        float fMaxScale = 10.0f;
        float fWheelZoomStep = 0.9f;   // scale multiplier per wheel notch

        // Editor camera view
        Vector2D editorCamPos;
        Vector2D editorCamScale = Vector2D(1.0f, 1.0f);

        AGameObject* selectedObject = nullptr;

    public:
        void processEditorInput(const SDL_Event* eEvent);
        void updateObjectDragging(float fDeltaTime);
        void draw(SDL_Renderer* pRenderer);
        bool isPlaying() const;
        void togglePlayMode();

        void setSelectedObject(AGameObject* selectedObject);
        AGameObject* getSelectedObject();

    private:
        Vector2D windowToLogical(float x, float y) const;
        void panCamera(const Vector2D& screenDelta);
        void zoomCamera(float fFactor, const Vector2D& screenPivot);
        void cancelInteraction();
        bool isPointerOverUI(const Vector2D& logicalPos) const;
        AGameObject* hitTestObject(AGameObject* obj, const Vector2D& pos);

        bool contains(AGameObject* pObject, Vector2D mousePos);
        Vector2D getMouseWorldPos() const;
        bool hasVisibleSprite(AGameObject* pObject);
        SDL_FRect getHandleRect(AGameObject* pObject, SDL_FRect* pOutBounds = nullptr) const;

        /* * * * * * * * * * * * * * * * * * * * *
         *       SINGLETON-RELATED CONTENT       *
         * * * * * * * * * * * * * * * * * * * * */
    private:
        static EditorModule* P_SHARED_INSTANCE;

    private:
        EditorModule();
        EditorModule(const EditorModule&) = delete;
        EditorModule& operator=(const EditorModule&) = delete;

    public:
        static void initialize();
        static void destroy();

        static EditorModule* getInstance();
        /* * * * * * * * * * * * * * * * * * * * */
    };
}
