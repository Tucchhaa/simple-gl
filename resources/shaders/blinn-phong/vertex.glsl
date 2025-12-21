#version 410 core

uniform mat4 transform;
uniform mat4 view;
uniform mat3 normalMatrix;
uniform mat4 projection;

uniform vec3 uvScale; 

in vec3 vPosition;
in vec2 vTextureCoord;
in vec3 vNormal;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTextureCoord;

void main()
{
    fPosition = vec3(transform * vec4(vPosition, 1.0));
    
    // Calculate normal
    fNormal = transpose(inverse(mat3(transform))) * vNormal;

    // --- TILING LOGIC ---
    // If uvScale is passed (length > 0.1), tile the texture.
    // We use max(x,z) for horizontal to handle walls (X/Y) and floors (X/Z) consistently
    vec2 tiling = vec2(1.0);
    if (length(uvScale) > 0.1) {
        tiling = vec2(max(uvScale.x, uvScale.z), uvScale.y);
    }
    
    fTextureCoord = vTextureCoord * tiling;
    // --------------------

    gl_Position = projection * view * vec4(fPosition, 1.0);
}