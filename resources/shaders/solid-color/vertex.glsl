#version 410 core

in vec3 vPosition;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * transform * vec4(vPosition, 1.0);
}
