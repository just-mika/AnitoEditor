#pragma once
#include "LoadedScene.h"
#include "EnumEventKey.h"

class PlatformerLoadedScene : public LoadedScene
{
public:
    PlatformerLoadedScene();
    ~PlatformerLoadedScene() override;

    std::string getGenre() const override { return "platformer"; }
    void onLoadResources() override;
    void onUnloadResources() override;
    void onUnloadObjects() override;
    void resetSubsystems() override;
    void rewire() override;

protected:
    void setUpSubsystems() override;

private:
    void attachToggle(const char* name, EventKey key);
};