// PuzzleLoadedScene.h
#pragma once
#include "LoadedScene.h"
#include <string>
#include <vector>

class PuzzleLoadedScene : public LoadedScene
{
public:
    PuzzleLoadedScene();
    ~PuzzleLoadedScene() override;

    std::string getGenre() const override { return "puzzle"; }
    void onLoadResources() override;
    void onUnloadResources() override;
    void readLevelData(const nlohmann::json& level) override;
    void writeLevelData(nlohmann::json& level) const override;
    void rewire() override;

protected:
    void setUpSubsystems() override;

private:
    std::vector<Vector2D> startSlots, targetSlots;
    std::vector<std::string> combination;                 // one token name per target slot
    std::unordered_map<std::string, std::string> hints;   // token name -> hint text
    void setDefaults();
};