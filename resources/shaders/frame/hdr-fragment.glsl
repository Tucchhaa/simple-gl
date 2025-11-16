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

        // dithering
        float n = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453123); // random value

        float amp = 1.0 / 255.0;
        float dither = (n - 0.5) * amp;   // [-amp/2, +amp/2]

        mapped += dither;

        FragColor = vec4(mapped, 1.0);
}
