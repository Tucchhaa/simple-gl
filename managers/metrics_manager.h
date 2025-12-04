#pragma once

#include <chrono>
#include <iostream>
#include <iomanip>
#include <numeric>
#include <vector>

class MetricsManager {
public:
    MetricsManager();

    void addFrameTime(float time);
    void addPortalPassTime(float time);
    void incrementDrawCalls();
    void updateRecursionDepth(int depth);

    void endFrame();

private:
    void printMetrics();
    void resetMetrics();

    int frameCount;
    int totalFrames;

    float totalFrameTime;
    float totalPortalPassTime;
    unsigned int totalDrawCalls;
    int maxRecursionDepth;

    unsigned int frameDrawCalls;
    int frameRecursionDepth;

    std::chrono::steady_clock::time_point lastPrintTime;
};
