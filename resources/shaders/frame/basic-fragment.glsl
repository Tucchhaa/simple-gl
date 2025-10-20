#version 410 core

uniform sampler2D frameTexture;

in vec2 fTextureCoord;

out vec4 FragColor;

void main()
{
    FragColor = texture(frameTexture, fTextureCoord);
}
