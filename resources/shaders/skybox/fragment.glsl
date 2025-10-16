#version 410 core

uniform samplerCube cubeMap;

in vec3 fTextureCoord;

out vec4 FragColor;

void main()
{
    FragColor = texture(cubeMap, fTextureCoord);
}
