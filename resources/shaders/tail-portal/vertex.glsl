#version 410 core

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 tailCameraView;
uniform mat4 tailCameraProjection;

in vec3 vPosition;

out vec4 fVirtualProjPos;

void main()
{
    vec4 worldPos = transform * vec4(vPosition, 1.0);
    gl_Position = projection * view * worldPos;
    fVirtualProjPos = tailCameraProjection * tailCameraView * worldPos;
}
