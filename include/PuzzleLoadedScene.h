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
    std::vector<std::string> combination;   // 3 token names in solution order
};