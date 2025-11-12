#include <iostream>
#include <sstream>

#include "demos/basic_demo.h"
#include "managers/engine.h"
#include "managers/mesh_manager.h"
#include "managers/shader_manager.h"
#include "managers/texture_manager.h"
#include "managers/window_manager.h"

#include "entities/input.h"
#include "entities/mesh_data.h"
#include "entities/scene.h"
#include "entities/window.h"
#include "render-pipeline/framebuffers/msaa_frame_buffer.h"

#include "render-pipeline/framebuffers/screen_frame_buffer.h"

using namespace SimpleGL;

int main() {
    auto window = Engine::instance().windowManager()->createWindow("main", 800, 600);
    window->setTitle("Learn OpenGL");
    window->makeCurrent();

    auto demo = BasicDemo();

    // create screen frame buffer
    auto frameShaderProgram = Engine::instance().shaderManager()->createShaderProgram(
        "shaders/frame/vertex.glsl",
        "shaders/frame/basic-fragment.glsl",
        "frame shader"
    );

    auto msaaFrameBuffer = MsaaFrameBuffer::create(window, 8);

    auto screenFrameBuffer = ScreenFrameBuffer::create(window);
    screenFrameBuffer->setShader(frameShaderProgram);


    demo.scene->emitStart();

    while(window->isOpen())
    {
        demo.updateNodes();
        demo.scene->emitUpdate();

        // draw scene
        // glBindFramebuffer(GL_FRAMEBUFFER, screenFrameBuffer->FBO());
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFrameBuffer->FBO());

        demo.draw();

        // resolve ms fbo & render frame onto the screen
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFrameBuffer->FBO());
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, screenFrameBuffer->FBO());
        glBlitFramebuffer(
            0, 0, msaaFrameBuffer->width(), msaaFrameBuffer->height(),
            0, 0, screenFrameBuffer->width(), screenFrameBuffer->height(),
            GL_COLOR_BUFFER_BIT, GL_NEAREST
        );

        screenFrameBuffer->renderFrame();

        // poll input events
        window->pollEvents();

        if (window->input()->isKeyPressed(GLFW_KEY_ESCAPE)) {
            window->close();
        }
    }

    return 0;
}
