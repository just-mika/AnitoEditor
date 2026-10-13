#include "SDL3/SDL.h"
#include "Runner.h"
#include <memory>

// actual program entry logic
int main(int argc, char* argv[])
{
    Runner runner;
    runner.run();
    return 0;
}