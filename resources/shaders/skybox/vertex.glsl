#version 410 core

uniform mat4 view;
uniform mat4 projection;

in vec3 vPosition;

out vec3 fTextureCoord;

void main()
{
    mat4 rotation = view;
    rotation[3] = vec4(0.0, 0.0, 0.0, 1.0);

    fTextureCoord = vPosition;
    gl_Position = (projection * rotation * vec4(vPosition, 1.0)).xyww;
}
