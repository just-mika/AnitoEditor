#pragma once
#include "AScene.h"

class LoadedScene : public AScene
{
public:
    LoadedScene();
    ~LoadedScene();

public:
	void onLoadResources() override;
    void onLoadObjects() override;
    void onUnloadResources() override;
};