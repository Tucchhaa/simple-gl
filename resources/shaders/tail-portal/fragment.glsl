#version 410 core

uniform sampler2D albedoTexture;

in vec4 fVirtualProjPos;

out vec4 FragColor;

void main()
{
    vec3 ndc = fVirtualProjPos.xyz / fVirtualProjPos.w;
    vec2 uv = ndc.xy * 0.5 + 0.5; // [-1,1] -> [0,1]

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
        discard;

    FragColor = texture(albedoTexture, uv);
}