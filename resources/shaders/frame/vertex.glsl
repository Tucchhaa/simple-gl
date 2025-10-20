#version 410 core

in vec3 vPosition;
in vec2 vTextureCoord;

out vec2 fTextureCoord;

void main()
{
    gl_Position = vec4(vPosition.xz, 0.0, 1.0);
    fTextureCoord = vTextureCoord;
}
