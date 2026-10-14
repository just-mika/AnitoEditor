#pragma once
#include "SDL3/SDL.h"
extern bool showWidgets;
static float gameHeight = 1080;
static float gameWidth = 1920;
static int gameFPS = 60;
static int frameDelay = 1000 / gameFPS;
static SDL_Color colorBlack{ 0,0,0,255 };
static SDL_Color colorWhite{ 255,255,255,255 };
static SDL_Color colorRed{ 255,0,0,255 };
static SDL_Color colorYellow{ 255,255,0,255 };
static SDL_Color colorBlue{ 0,0,255,255 };
static SDL_Color colorCyan{ 0,255,255,255 };
static SDL_Color colorPurple{ 128,0,128,255 };

static const float kToolbarH = 56.0f;
static const float kBottomBarH = 230.0f;
static const float kHierarchyW = 600.0f;

inline float uiWidth = 1920.0f;   // logical size of the whole window (UI canvas)
inline float uiHeight = 1080.0f;
inline SDL_FRect gameViewDst = { 0, 0, 1920.0f, 1080.0f };   // where the game image sits in the UI canvas

// window/UI coordinates -> 1920x1080 game coordinates
inline void uiToGame(float ux, float uy, float& gx, float& gy)
{
    gx = (ux - gameViewDst.x) * (gameWidth / gameViewDst.w);
    gy = (uy - gameViewDst.y) * (gameHeight / gameViewDst.h);
}