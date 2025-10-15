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

struct LightComponents {
    vec3 diffuse;
    vec3 specular;
};

#define MAX_DIRECT_LIGHTS_NUM 3
#define MAX_POINT_LIGHTS_NUM 10

uniform DirectLight directLights[MAX_DIRECT_LIGHTS_NUM];
uniform PointLight pointLights[MAX_POINT_LIGHTS_NUM];

uniform int directLightsNum;
uniform int pointLightsNum;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform vec3 viewPosition;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTextureCoord;

out vec4 FragColor;

LightComponents calcLighting();

void main()
{
    LightComponents lighting = calcLighting();

    vec3 diffuse = lighting.diffuse * vec3(texture(diffuseTexture, fTextureCoord));
    vec3 specular = lighting.specular * vec3(texture(specularTexture, fTextureCoord));

    FragColor = vec4(diffuse + specular, 1.0f);
}

LightComponents calcDirectLight(DirectLight light, vec3 normal, vec3 viewDir);
LightComponents calcPointLight(PointLight light, vec3 normal, vec3 viewDir);

LightComponents calcLighting() {
    LightComponents result = LightComponents(vec3(0.0), vec3(0.0));

    vec3 normal = normalize(fNormal);
    vec3 viewDir = normalize(viewPosition - fPosition);

    for(int i = 0; i < directLightsNum; i++) {
        LightComponents directLightResult = calcDirectLight(directLights[i], normal, viewDir);

        result.diffuse += directLightResult.diffuse;
        result.specular += directLightResult.specular;
    }

    for(int i = 0;i < pointLightsNum; i++) {
        LightComponents pointLightResult = calcPointLight(pointLights[i], normal, viewDir);

        result.diffuse += pointLightResult.diffuse;
        result.specular += pointLightResult.specular;
    }

    return result;
}

LightComponents calcDirectLight(DirectLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = -light.direction;
    vec3 reflectDir = reflect(-lightDir, normal);

    float diffuse = max(dot(normal, lightDir), 0.0);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 1);

    vec3 diffuseLight  = light.diffuse * diffuse + light.ambient;
    vec3 specularLight = light.specular * specular;

    return LightComponents(diffuseLight, specularLight);
}

LightComponents calcPointLight(PointLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fPosition);
    vec3 reflectDir = -reflect(lightDir, normal);

    float diffuse = max(dot(normal, lightDir), 0.0);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 1);

    float distance = length(light.position - fPosition);
    float attenuation = max(1 - distance / light.distance, 0.0);

    // combine results
    vec3 diffuseLight = attenuation * (light.diffuse * diffuse + light.ambient);
    vec3 specularLight = attenuation * light.specular * specular;

    return LightComponents(diffuseLight, specularLight);
}
