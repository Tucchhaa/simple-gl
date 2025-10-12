#version 410 core

uniform sampler2D albedoTexture;

in vec2 fTextureCoord;

out vec4 FragColor;

void main()
{
    FragColor = texture(albedoTexture, fTextureCoord);
}