#pragma once

#include <functional>
#include <memory>

namespace SimpleGL {

class ShaderProgram;
class ScreenFrameBuffer;
class MsaaFrameBuffer;
class Node;
class MeshComponent;

// TODO: position panel relative to window left corner, not center
struct WindowPanelLocation {
    float x = 0;
    float y = 0;
    float width = 1;
    float height = 1;
};

struct WindowPanelSettings {
    int frameWidth = -1;
    int frameHeight = -1;
    int msaaSamples = 1;
    bool hdrEnabled = false;
};

class WindowPanel {
public:
    WindowPanel(WindowPanelLocation location, WindowPanelSettings settings);

    int frameWidth() const;
    int frameHeight() const;

    void renderToFrame(const std::function<void()>& drawCallback) const;
    void renderToScreen() const;

private:
    const WindowPanelLocation m_location;
    const WindowPanelSettings m_settings;

    std::unique_ptr<ScreenFrameBuffer> m_screenFrameBuffer;
    std::unique_ptr<MsaaFrameBuffer> m_msaaFrameBuffer;

    std::shared_ptr<Node> m_quadNode;
    std::shared_ptr<MeshComponent> m_quadMesh;
    std::shared_ptr<ShaderProgram> m_quadShader;

    void initFrameBuffers();
    void initQuadNode();
};

}
