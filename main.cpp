#include <iostream>
#include <sstream>
#include "demos/map_import_demo.h"
#include "demos/basic_demo.h"
#include "demos/map_loading_demo.h"
#include "managers/engine.h"
#include "managers/mesh_manager.h"
#include "managers/shader_manager.h"
#include "managers/window_manager.h"

#include "entities/input.h"
#include "entities/mesh_data.h"
#include "entities/scene.h"
#include "entities/window.h"
#include "render-pipeline/framebuffers/msaa_frame_buffer.h"
#include "render-pipeline/framebuffers/screen_frame_buffer.h"

using namespace SimpleGL;

int main() {
    constexpr int MSAA_SAMPLES = 4;
    constexpr bool HDR_ENABLED = true;
    constexpr int SCREEN_WIDTH = 1200;
    constexpr int SCREEN_HEIGHT = 900;

    auto window = Engine::instance().windowManager()->createWindow("main", SCREEN_WIDTH, SCREEN_HEIGHT);
    window->setTitle("Learn OpenGL");
    window->makeCurrent();

    //auto demo = MapImportDemo();
    auto demo = MapLoadingDemo();
    //auto demo = BasicDemo();

    // create screen frame buffer
    auto frameShaderProgram = Engine::instance().shaderManager()->createShaderProgram(
        "shaders/frame/vertex.glsl",
        HDR_ENABLED ? "shaders/frame/hdr-fragment.glsl" : "shaders/frame/basic-fragment.glsl",
        "frame shader"
    );

    std::shared_ptr<MsaaFrameBuffer> msaaFrameBuffer = MSAA_SAMPLES > 1
        ? MsaaFrameBuffer::create(window, HDR_ENABLED, MSAA_SAMPLES)
        : nullptr;

    auto screenFrameBuffer = ScreenFrameBuffer::create(window, HDR_ENABLED);
    screenFrameBuffer->setShader(frameShaderProgram);

    demo.scene->emitStart();

    while (window->isOpen()) {
        demo.updateNodes();
        demo.scene->emitUpdate();

        if (msaaFrameBuffer != nullptr) {
            glBindFramebuffer(GL_FRAMEBUFFER, msaaFrameBuffer->FBO());

            demo.draw();

            glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFrameBuffer->FBO());
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFrameBuffer->FBO());
            glBlitFramebuffer(
                0, 0, msaaFrameBuffer->width(), msaaFrameBuffer->height(),
                0, 0, screenFrameBuffer->width(), screenFrameBuffer->height(),
                GL_COLOR_BUFFER_BIT, GL_NEAREST
            );
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, screenFrameBuffer->FBO());
            demo.draw();
        }

        screenFrameBuffer->renderFrame();
        window->pollEvents();

        if (window->input()->isKeyPressed(GLFW_KEY_ESCAPE)) {
            window->close();
        }
    }

    return 0;
}