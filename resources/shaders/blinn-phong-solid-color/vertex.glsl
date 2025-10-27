#version 410 core

uniform mat4 transform;
uniform mat4 view;
uniform mat3 normalMatrix;
uniform mat4 projection;

in vec3 vPosition;
in vec3 vNormal;

out vec3 fPosition;
out vec3 fNormal;

void main()
{
    fPosition = vec3(transform * vec4(vPosition, 1.0));
//    fNormal = normalMatrix * vNormal;
    fNormal = transpose(inverse(mat3(transform))) * vNormal;
    gl_Position = projection * view * vec4(fPosition, 1.0);
}
