#version 410 core

uniform sampler2D frameTexture;

in vec2 fTextureCoord;

out vec4 FragColor;

void main()
{
        const float gamma = 2.2;
        vec3 hdrColor = texture(frameTexture, fTextureCoord).rgb;

        // reinhard tone mapping
        vec3 mapped = hdrColor / (hdrColor + vec3(1.0));
        mapped = pow(mapped, vec3(1.0 / gamma));
        FragColor = vec4(mapped, 1.0);
}
