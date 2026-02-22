#version 410 core

uniform mat4 transform;

in vec3 vPosition;
in vec2 vTextureCoord;

out vec2 fTextureCoord;

void main()
{
    gl_Position = transform * vec4(vPosition.xz, 0.0, 1.0);
    fTextureCoord = vTextureCoord;
}
