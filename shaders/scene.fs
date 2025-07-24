#version 330 core
struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;

uniform sampler2D texture_diffuse1;
uniform bool useTexture;

uniform Material material;
uniform Light light;

void main()
{
    vec3 ambient = light.ambient;
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuseLight = light.diffuse * diff;

    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specularLight = light.specular * spec;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;

    if (useTexture) {
        vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;
        ambientColor = texColor * material.ambient;
        diffuseColor = texColor * material.diffuse;
        specularColor = texColor * material.specular;
    } else {
        ambientColor = material.ambient;
        diffuseColor = material.diffuse;
        specularColor = material.specular;
    }

    vec3 result = ambientColor * ambient +
                  diffuseColor * diffuseLight +
                  specularColor * specularLight;

    FragColor = vec4(result, 1.0);
}
