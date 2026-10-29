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
    void rewire() override;

protected:
    void setUpSubsystems() override;
};