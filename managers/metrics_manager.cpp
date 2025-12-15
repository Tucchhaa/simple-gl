#include "metrics_manager.h"

MetricsManager::MetricsManager() {
    resetMetrics();
    lastPrintTime = std::chrono::steady_clock::now();
}

void MetricsManager::addFrameTime(float time) {
    totalFrameTime += time;
}

void MetricsManager::addPortalPassTime(float time) {
    totalPortalPassTime += time;
}

void MetricsManager::incrementDrawCalls() {
    frameDrawCalls++;
}

void MetricsManager::updateRecursionDepth(int depth) {
    if (depth > frameRecursionDepth) {
        frameRecursionDepth = depth;
    }
}

void MetricsManager::endFrame() {
    totalDrawCalls += frameDrawCalls;
    if (frameRecursionDepth > maxRecursionDepth) {
        maxRecursionDepth = frameRecursionDepth;
    }

    frameDrawCalls = 0;
    frameRecursionDepth = 0;
    frameCount++;
    totalFrames++;

    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - lastPrintTime).count();

    if (elapsed >= 1.0) {
        printMetrics();
        resetMetrics();
        lastPrintTime = now;
    }
}

void MetricsManager::printMetrics() {
    if (frameCount == 0) return;

    double avgFrameTime = totalFrameTime / frameCount;
    double avgPortalTime = totalPortalPassTime / frameCount;
    double avgDrawCalls = static_cast<double>(totalDrawCalls) / frameCount;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "-------------------- METRICS --------------------" << std::endl;
    std::cout << std::left << std::setw(25) << "Avg. Frame Time (ms): " << avgFrameTime << std::endl;
    std::cout << std::left << std::setw(25) << "Avg. Portal Pass Time (ms): " << avgPortalTime << std::endl;
    std::cout << std::left << std::setw(25) << "Avg. Draw Calls: " << avgDrawCalls << std::endl;
    std::cout << std::left << std::setw(25) << "Max Recursion Depth: " << maxRecursionDepth << std::endl;
    std::cout << "-----------------------------------------------" << std::endl;
}

void MetricsManager::resetMetrics() {
    frameCount = 0;
    totalFrameTime = 0.0f;
    totalPortalPassTime = 0.0f;
    totalDrawCalls = 0;
    maxRecursionDepth = 0;
    frameDrawCalls = 0;
    frameRecursionDepth = 0;
}