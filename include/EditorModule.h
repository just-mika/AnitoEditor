#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include <unordered_map>
#include "Vector2D.h"
#include "AUIScreen.h"
#include "AGameObject.h"

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
        bool bIsDragging = false;
        bool bIsHolding = false;
        bool bIsMovingCam = false;
        bool bIsPlaying = false;

        // Positions below are all in LOGICAL (gameWidth x gameHeight) coordinates
        Vector2D mousePos;
        Vector2D offsetPos;

        // Camera zoom limits. Camera scale = world units per screen pixel,
        // so a SMALLER scale means zoomed IN.
        float fMinScale = 0.1f;
        float fMaxScale = 10.0f;
        float fWheelZoomStep = 0.9f;   // scale multiplier per wheel notch

        // Editor camera view, saved when entering play mode and restored on stop
        Vector2D editorCamPos;
        Vector2D editorCamScale = Vector2D(1.0f, 1.0f);

        // Touch (pinch-zoom / two-finger pan)
        std::unordered_map<SDL_FingerID, Vector2D> mapFingers;
        bool bIsPinching = false;
        float fLastPinchDist = 0.0f;
        Vector2D lastPinchCenter;

        AGameObject* selectedObject = nullptr;

    public:
        void processEditorInput(const SDL_Event* eEvent);
        void updateGameObjects(float fDeltaTime);
        void drawEditor(SDL_Renderer* pRenderer);
        bool isPlaying() const;
        void togglePlayMode();

        void setSelectedObject(AGameObject* selectedObject);
        AGameObject* getSelectedObject();

    private:
        Vector2D windowToLogical(float x, float y) const;
        Vector2D fingerToLogical(float nx, float ny) const;
        void panCamera(const Vector2D& screenDelta);
        void zoomCamera(float fFactor, const Vector2D& screenPivot);
        void updatePinch();
        void cancelInteraction();
        bool isPointerOverUI(const Vector2D& logicalPos) const;

        bool contains(AGameObject* pObject, Vector2D mousePos);
        Vector2D getMouseWorldPos() const;
        bool hasVisibleSprite(AGameObject* pObject);

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