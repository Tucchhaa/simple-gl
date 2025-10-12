#version 410 core

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

in vec3 vPosition;
in vec2 vTextureCoord;

out vec2 fTextureCoord;

void main()
{
    gl_Position = projection * view * transform * vec4(vPosition, 1.0);
    fTextureCoord = vTextureCoord;
}
