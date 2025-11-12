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

#include "render-pipeline/screen_frame_buffer.h"

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

    auto screenFrameBuffer = ScreenFrameBuffer::create(window);
    screenFrameBuffer->setShader(frameShaderProgram);

    demo.scene->emitStart();

    while(window->isOpen())
    {
        demo.updateNodes();
        demo.scene->emitUpdate();

        // draw scene
        glBindFramebuffer(GL_FRAMEBUFFER, screenFrameBuffer->FBO());

        demo.draw();

        // update screen & poll events
        screenFrameBuffer->renderFrame();
        window->pollEvents();

        if (window->input()->isKeyPressed(GLFW_KEY_ESCAPE)) {
            window->close();
        }
    }

    return 0;
}
