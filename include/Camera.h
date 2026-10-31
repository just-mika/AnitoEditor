#pragma once
#include "AComponent.h"
#include "Vector2D.h"

class Camera
{
private:
	Vector2D position;
	float rotation;
	Vector2D scale;
	Vector2D windowSize;
    Vector2D windowScale;
	bool bCanvasInWorld = false;

public:
	Camera();
	~Camera();

	bool isInView(const SDL_FRect& worldRect) const;
	Vector2D screenToWorldPoint(const Vector2D& screenPoint) const;
	Vector2D worldToScreenPoint(const Vector2D& worldPoint) const;

	SDL_FRect screenToWorldRect(const SDL_FRect& screenRect) const;
	SDL_FRect worldToScreenRect(const SDL_FRect& worldRect) const;

	Vector2D getWindowSize() const;
    Vector2D getWindowScale() const;
    Vector2D getWindowSizeScaled() const;
    Vector2D getWindowOffset() const;
	float getHalfWidth() const;
	float getHalfHeight() const;

	Vector2D getPos();
	float getRot();
	Vector2D getScale();

	void setWindowSize(Vector2D windowSize);
    void setWindowScale(Vector2D windowScale);
	void setPos(Vector2D position);
	void setRot(float rotation);
	void setScale(Vector2D scale);

	void setCanvasInWorld(bool b);
	bool isCanvasInWorld() const;

	Vector2D canvasToWorldPoint(const Vector2D& canvasPoint) const;
	Vector2D worldToCanvasPoint(const Vector2D& worldPoint) const;
	SDL_FRect canvasToScreenRect(const SDL_FRect& canvasRect) const;
	Vector2D canvasToScreenPoint(const Vector2D& canvasPoint) const;
};

