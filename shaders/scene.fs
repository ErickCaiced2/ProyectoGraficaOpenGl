#version 330 core

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float haloIntensity;
};
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};
struct MoonLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
};
#define MAX_POINT_LIGHTS 7

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform vec3 viewPos;
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform sampler2D texture_diffuse1;
uniform SpotLight spotLight;
uniform MoonLight moonLight;

// Uniformes para la explosión
uniform float explosionRadius;
uniform vec3 explosionOrigin;
uniform float explosionIntensity;


// Luz direccional
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 ambient = light.ambient * texColor;
    vec3 diffuse = light.diffuse * diff * texColor;
    vec3 specular = vec3(0.0);
    
    return (ambient + diffuse + specular);
}

// Point Light calculation - Ilumina 360 grados
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    // Si la luz está "apagada", no contribuir nada
    if (light.haloIntensity <= 0.01) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Calcular atenuación por distancia solamente
    float distance = length(light.position - fragPos);
    distance = max(distance, 0.1); // Evitar división por cero
    
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    attenuation = clamp(attenuation, 0.0, 100.0); // Limitar valores extremos
    
    // Cálculo de iluminación difusa (sin restricción de ángulo)
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Cálculo especular básico
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;
    
    // TODAS las componentes dependen del haloIntensity - MÁS INTENSO
    float intensity = light.haloIntensity * 2.5;  // ? QUITAR LA 'f'
    vec3 ambient = light.ambient * texColor * intensity;
    vec3 diffuse = light.diffuse * diff * texColor * intensity;
    vec3 specular = light.specular * spec * texColor * intensity;
    
    // Resultado final con atenuación
    vec3 result = (ambient + diffuse + specular) * attenuation;
    
    // Asegurar que no hay valores negativos o NaN
    return max(vec3(0.0), result);
}

// Halo alrededor de las Point Lights (efecto omnidireccional)
vec3 CalcPointLightHalo(PointLight light, vec3 fragPos) {
    // Solo si el halo está activo
    if (light.haloIntensity <= 0.0) return vec3(0.0);
    
    float distance = length(light.position - fragPos);
    float haloRadius = 10.0; // ? AUMENTADO de 15.0 para mayor alcance
    
    if (distance > haloRadius) return vec3(0.0);
    
    // Halo esférico más intenso
    float haloIntensity = (1.0 - smoothstep(0.0, haloRadius, distance)) * light.haloIntensity;
    
    // Multiplicador mayor para efecto más dramático
    return max(vec3(0.0), light.diffuse * haloIntensity * 3.0); // ? AUMENTADO de 3.0 a 5.0
}
// Función para calcular Spotlight (como la del avión)
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    if (!light.enabled) return vec3(0.0);
    
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    distance = max(distance, 0.1);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * distance * distance);
    attenuation = clamp(attenuation, 0.0, 10.0);
    
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = max(light.cutOff - light.outerCutOff, 0.01);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    
    if (intensity < 0.001) return vec3(0.0);
    
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;
    
    vec3 ambient = light.ambient * texColor * intensity;  
    vec3 diffuse = light.diffuse * diff * texColor * intensity;
    vec3 specular = light.specular * spec * texColor * intensity;
    
    vec3 result = (ambient + diffuse + specular) * attenuation;
    return max(vec3(0.0), result);
}
// Función para calcular la luz de la luna
vec3 CalcMoonLight(MoonLight light, vec3 normal, vec3 fragPos) {
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    
    vec3 texColor = texture(texture_diffuse1, TexCoords).rgb;
    vec3 ambient = light.ambient * texColor * 2.0f;
    vec3 diffuse = light.diffuse * diff * texColor * 1.5f;
    
    return (ambient + diffuse);
}

// Función para el efecto de explosión
vec3 ApplyExplosionEffect(vec3 originalColor, vec3 fragPos) {
    float dist = distance(fragPos, explosionOrigin);
    if (dist < explosionRadius && explosionIntensity > 0.0) {
        float wave = smoothstep(explosionRadius, explosionRadius - 5.0, dist);
        float centerGlow = 1.0 - smoothstep(0.0, explosionRadius * 0.3, dist);
        
        vec3 explosionColor = mix(
            originalColor,
            vec3(1.0, 0.9, 0.7),
            centerGlow * explosionIntensity
        );
        
        return mix(explosionColor, originalColor, wave);
    }
    return originalColor;
}

void main() {
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Luz direccional
    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    
    // Point Lights - Iluminación principal omnidireccional
    for(int i = 0; i < numPointLights; i++) {
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);
    }
    
    // Halos de las Point Lights (efectos visuales esféricos)
    for(int i = 0; i < numPointLights; i++) {
        result += CalcPointLightHalo(pointLights[i], FragPos);
    }
    
    // Aplicar efecto de explosión
    result = ApplyExplosionEffect(result, FragPos);
        
    result += CalcSpotLight(spotLight, norm, FragPos, viewDir); // SpotLight (Avión)
        // Luz de la luna
    result += CalcMoonLight(moonLight, norm, FragPos);
    
    // Asegurar que el resultado final es válido
    FragColor = vec4(max(vec3(0.0), result), 1.0);
}