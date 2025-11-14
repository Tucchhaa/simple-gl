#version 410 core

uniform sampler2D frameTexture;

in vec2 fTextureCoord;

out vec4 FragColor;

void main()
{
    const float gamma = 2.2;

    vec3 color = texture(frameTexture, fTextureCoord).rgb;

    FragColor = vec4(pow(color, vec3(1.0 / gamma)), 1);
}
