#pragma once
#include "AUIScreen.h"
#include "AGameObject.h"

class ObjectPalette : public AUIScreen
{
private:
	AGameObject* selectedObject;
	Vector2D mousePos;
	bool wasCollapsed = false;

public:
	void setSelectedObject(AGameObject* selectedObject);
	AGameObject* getSelectedObject();
	void setMousePos(Vector2D mousePos);

protected:
    ObjectPalette();
	~ObjectPalette();
	void DrawUI() override;

	friend class UIManager;

private:
	void spawnObject(const std::string& typeKey, bool isScreenObject, const std::string& variant = "");
};