// LoadedScene.h
#pragma once
#include "AScene.h"
#include "json.hpp"

class LoadedScene : public AScene
{
public:
    LoadedScene(SceneTag tag);
    ~LoadedScene() override;

    virtual std::string getGenre() const = 0;

    // scene-wide settings that aren't objects (e.g. the puzzle's combination)
    virtual void readLevelData(const nlohmann::json& level) {}
    virtual void writeLevelData(nlohmann::json& level) const {}

    // connect objects to this genre's subsystems. Called after a load, and (add it yourself) when play starts
    virtual void rewire() {}

    void onLoadObjects() final;
    void onUnloadObjects() override;

protected:
    virtual void setUpSubsystems() = 0;   // create this genre's managers
};