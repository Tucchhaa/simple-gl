#version 410 core

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

in vec3 vPosition;

out vec3 fTextureCoord;

void main()
{
    gl_Position = (projection * view * transform * vec4(vPosition, 1.0)).xyww;
    fTextureCoord = vPosition;
}
