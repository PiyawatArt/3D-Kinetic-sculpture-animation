#version 330 core
// Upgraded lighting with Blinn/Phong toggle, optional toon quantization and gamma correction.
// Supports up to 8 colored point lights + 1 directional + 1 spotlight.

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
};

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define MAX_POINT_LIGHTS 8

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform SpotLight spotLight;
uniform Material material;

uniform bool useBlinn;
uniform bool useToon;
uniform bool useGamma;
uniform float gammaValue;

float qstep(float x){
    if (!useToon) return x;
    if (x < 0.2) return 0.1;
    else if (x < 0.5) return 0.35;
    else if (x < 0.8) return 0.65;
    else return 0.9;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 albedo = texture(material.diffuse, TexCoords).rgb;
    vec3 specMap = texture(material.specular, TexCoords).rgb;

    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = useBlinn
        ? pow(max(dot(normal, halfDir), 0.0), material.shininess)
        : pow(max(dot(viewDir, reflect(-lightDir, normal)), 0.0), material.shininess);

    diff = qstep(diff);
    spec = useToon ? qstep(spec) : spec;

    vec3 ambient  = light.ambient * albedo;
    vec3 diffuse  = light.diffuse * diff * albedo;
    vec3 specular = light.specular * spec * specMap;

    return ambient + diffuse + specular;
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 albedo = texture(material.diffuse, TexCoords).rgb;
    vec3 specMap = texture(material.specular, TexCoords).rgb;

    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = useBlinn
        ? pow(max(dot(normal, halfDir), 0.0), material.shininess)
        : pow(max(dot(viewDir, reflect(-lightDir, normal)), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diff = qstep(diff);
    spec = useToon ? qstep(spec) : spec;

    vec3 ambient  = light.ambient * albedo;
    vec3 diffuse  = (light.diffuse * diff) * albedo * light.color;
    vec3 specular = (light.specular * spec) * specMap * light.color;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 albedo = texture(material.diffuse, TexCoords).rgb;
    vec3 specMap = texture(material.specular, TexCoords).rgb;

    vec3 lightDir = normalize(light.position - fragPos);
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / max(epsilon, 0.0001), 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfDir = normalize(lightDir + viewDir);
    float spec = useBlinn
        ? pow(max(dot(normal, halfDir), 0.0), material.shininess)
        : pow(max(dot(viewDir, reflect(-lightDir, normal)), 0.0), material.shininess);

    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    diff = qstep(diff) * intensity;
    spec = (useToon ? qstep(spec) : spec) * intensity;

    vec3 ambient  = light.ambient * albedo * intensity;
    vec3 diffuse  = light.diffuse * diff   * albedo;
    vec3 specular = light.specular * spec  * specMap;

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return ambient + diffuse + specular;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 color = CalcDirLight(dirLight, norm, viewDir);
    for (int i=0; i<numPointLights; ++i)
        color += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    color += CalcSpotLight(spotLight, norm, FragPos, viewDir);

    if (useGamma) color = pow(color, vec3(1.0 / gammaValue));

    FragColor = vec4(color, 1.0);
}
