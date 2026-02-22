#include <iostream>
#include <sstream>

#include "demos/basic_demo.h"
#include "managers/engine.h"
#include "managers/mesh_manager.h"
#include "managers/shader_manager.h"
#include "entities/window.h"

#include "entities/input.h"
#include "entities/mesh_data.h"
#include "entities/scene.h"
#include "render-pipeline/window_panel.h"

#include "render-pipeline/framebuffers/msaa_frame_buffer.h"
#include "render-pipeline/framebuffers/screen_frame_buffer.h"

using namespace SimpleGL;

int main() {
    constexpr int MSAA_SAMPLES = 4;
    constexpr bool HDR_ENABLED = true;
    constexpr int SCREEN_WIDTH = 1200;
    constexpr int SCREEN_HEIGHT = 900;

    Engine::init();

    const auto& window = Engine::get()->window();
    window->open(SCREEN_WIDTH, SCREEN_HEIGHT);
    window->setTitle("Learn OpenGL");

    WindowPanelLocation windowPanelPosition;
    WindowPanelSettings windowPanelSettings;
    windowPanelSettings.hdrEnabled = HDR_ENABLED;
    windowPanelSettings.msaaSamples = MSAA_SAMPLES;
    const auto& panel = std::make_unique<WindowPanel>(windowPanelPosition, windowPanelSettings);

    auto demo = BasicDemo();
    const auto drawCallback = [&demo]() { demo.draw(); };

    demo.scene->start();

    while(window->isOpen())
    {
        // poll input events
        window->pollEvents();

        if (window->input()->isKeyPressed(GLFW_KEY_ESCAPE)) {
            window->close();
        }

        demo.scene->update();
        demo.stepPhysicsSimulation();
        demo.scene->rootNode()->transform()->recalculate();
        demo.camera->recalculateViewMatrix();

        panel->renderToFrame(drawCallback);
        panel->renderToScreen();
    }

    return 0;
}
