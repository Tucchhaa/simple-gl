#version 410 core

struct DirectLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    float distance;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_DIRECT_LIGHTS_NUM 3
#define MAX_POINT_LIGHTS_NUM 10

uniform DirectLight directLights[MAX_DIRECT_LIGHTS_NUM];
uniform PointLight pointLights[MAX_POINT_LIGHTS_NUM];

uniform int directLightsNum;
uniform int pointLightsNum;

uniform vec3 color;
uniform vec3 viewPosition;

in vec3 fPosition;
in vec3 fNormal;

out vec4 FragColor;

vec3 calcLighting();

void main()
{
    vec3 diffuse = calcLighting() * color;

    FragColor = vec4(diffuse, 1.0f);
}

vec3 calcDirectLight(DirectLight light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir);

vec3 calcLighting() {
    vec3 result = vec3(0.0);

    vec3 normal = normalize(fNormal);
    vec3 viewDir = normalize(viewPosition - fPosition);

    for(int i = 0; i < directLightsNum; i++) {
        result += calcDirectLight(directLights[i], normal, viewDir);
    }

    for(int i = 0;i < pointLightsNum; i++) {
        result += calcPointLight(pointLights[i], normal, viewDir);
    }

    return result;
}

vec3 calcDirectLight(DirectLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = -light.direction;

    float diffuse = max(dot(normal, lightDir), 0.0);

    vec3 diffuseLight  = light.diffuse * diffuse + light.ambient;

    return diffuseLight;
}

vec3 calcPointLight(PointLight light, vec3 normal, vec3 viewDir) {
    vec3 lightVec = light.position - fPosition;
    vec3 lightDir = normalize(lightVec);
    float distance = length(lightVec);

    float diffuse = max(dot(normal, lightDir), 0.0);

    float attenuation = max(1 - distance / light.distance, 0.0);

    vec3 diffuseLight = attenuation * light.diffuse * diffuse;

    return diffuseLight;
}
