#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/shader.h>
#include <learnopengl/model.h>
#include <learnopengl/camera.h>
#define STB_IMAGE_IMPLEMENTATION
#include <learnopengl/stb_image.h>

#include <iostream>
#define M_PI 3.14159265359f
#include <random> 
#include <vector>

std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(2.0f, 5.0f);

// Tamaño de ventana
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Cámara
Camera camera(glm::vec3(10.0f, 30.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Tiempo
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Sistema de luces
struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
    float haloIntensity;  
};

struct DirLight {
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

DirLight dirLight;

// Configuración de focos
const int NUM_POINTLIGHTS = 6;
PointLight pointLights[NUM_POINTLIGHTS];

float lightRotation = 0.0f;
struct LightEffect {
    float haloIntensity;
    float timer;
    bool haloOn;
    float nextToggleTime;
};

LightEffect lightEffects[NUM_POINTLIGHTS];

// Estructura para la luz de la luna
struct MoonLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
};

MoonLight moonLight;

// ===== VARIABLES DE BOMBA Y EXPLOSIÓN CORREGIDAS =====
float explosionTimer = 0.0f;
bool explosionActive = false;
bool bombHasExploded = false;  // NUEVA: Para evitar múltiples explosiones
glm::vec3 explosionOrigin = glm::vec3(5.0f, 2.0f, -40.0f);
float shakeIntensity = 0.0f;
float whiteIntensity = 0.0f;
float explosionDuration = 20.0f;  // Reducido a 10 segundos
float currentExplosionRadius = 0.0f;
float maxExplosionRadius = 40.0f; //Radio de acción de la bomba
bool modelsDestroyed = false;  // Nueva variable para controlar visibilidad
PointLight explosionLight;
bool explosionLightActive = false;

// Variables de animación
float z_position = -60.0f;
float avanceZ = 0.0f;
float bombaY = 50.0f;  // Inicia más alto para ver la caída
float bombFallSpeed = 0.1f;  // Velocidad de caída controlada

//Variable de esphera
unsigned int sphereIndexCount;  // Para almacenar el número de índices de la esfera

// Callbacks
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void processInput(GLFWwindow* window) {
    float speed = deltaTime * 10;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, speed);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, speed);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, speed);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, speed);

    // NUEVA: Tecla R para reiniciar la bomba
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        bombaY = 35.0f;
        z_position = -60.0f;
        explosionActive = false;
        bombHasExploded = false;
        explosionTimer = 0.0f;
        currentExplosionRadius = 0.0f;
        whiteIntensity = 0.0f;
        shakeIntensity = 0.0f;
        modelsDestroyed = false;  // Volver a dibujar a todos
        explosionLightActive = false;  
        explosionLight.haloIntensity = 0.0f;
        // Restaurar luz direccional
        dirLight.ambient = glm::vec3(0.6f, 0.6f, 0.6f);
        dirLight.diffuse = glm::vec3(1.0f, 1.0f, 0.95f);

    }
    // RESTRICCIÓN SIMPLE DE CÚPULA (3 líneas)
    float maxDistance = 75.0f;  // Radio de la cúpula
    float distanceFromCenter = glm::length(camera.Position);
    if (distanceFromCenter > maxDistance) {
        camera.Position = glm::normalize(camera.Position) * maxDistance;
    }
}

void setupPointLights() {
    float radius = 40.0f;
    float height = 50.0f;

    for (int i = 0; i < NUM_POINTLIGHTS; i++) {
        float angle = (2.0f * M_PI * i) / NUM_POINTLIGHTS;

        float offsetX = 10.0f;
        float offsetY = 0.0f;
        float offsetZ = -10.0f;

        pointLights[i].position = glm::vec3(cos(angle) * radius + offsetX, height + offsetY, sin(angle) * radius + offsetZ);
        lightEffects[i].haloOn = false;
        lightEffects[i].timer = 0.0f;
        lightEffects[i].nextToggleTime = dis(gen);

        switch (i) {
        case 0: pointLights[i].diffuse = glm::vec3(0.1f, 0.4f, 1.0f); break;  // MÁS AZUL
        case 1: pointLights[i].diffuse = glm::vec3(0.0f, 0.3f, 1.0f); break;  // MÁS AZUL
        case 2: pointLights[i].diffuse = glm::vec3(0.2f, 0.5f, 1.0f); break;  // MÁS AZUL
        case 3: pointLights[i].diffuse = glm::vec3(0.0f, 0.4f, 1.0f); break;  // MÁS AZUL
        case 4: pointLights[i].diffuse = glm::vec3(0.1f, 0.3f, 1.0f); break;  // MÁS AZUL
        case 5: pointLights[i].diffuse = glm::vec3(0.0f, 0.5f, 1.0f); break;  // MÁS AZUL
        }

        // AMBIENT muy bajo para point lights (no deben dominar)
        pointLights[i].ambient = glm::vec3(0.02f, 0.02f, 0.02f);  // Reducido aún más
        pointLights[i].specular = glm::vec3(0.05f, 0.05f, 0.05f); // Reducido

        // Atenuación similar
        pointLights[i].constant = 1.0f;
        pointLights[i].linear = 0.001f;
        pointLights[i].quadratic = 0.0001f;
    }

    for (int i = 0; i < NUM_POINTLIGHTS; i++) {
        lightEffects[i].haloIntensity = 0.0f;
        lightEffects[i].timer = (rand() % 10) * 0.3f;
    }

    // MEJORAR la luz direccional para que sea más fuerte y natural
    dirLight.direction = glm::vec3(-0.2f, -1.0f, -0.3f);
    dirLight.ambient = glm::vec3(0.6f, 0.6f, 0.6f);   // AUMENTADO significativamente
    dirLight.diffuse = glm::vec3(1.0f, 1.0f, 0.95f);  // Más fuerte y ligeramente cálida
    dirLight.specular = glm::vec3(0.2f, 0.2f, 0.2f);  // Especular sutil

    // Configurar luz de la luna
    moonLight.position = glm::vec3(0.0f, 40.0f, -50.0f);
    moonLight.ambient = glm::vec3(0.03f, 0.03f, 0.08f); //Luna Morada
    moonLight.diffuse = glm::vec3(0.4f, 0.3f, 0.7f); //Más purpura
}

void updateLightEffects() {
    static int activeLightIndex = 0;
    static float switchTimer = 0.0f;
    const float switchInterval = 3.0f;

    switchTimer += deltaTime;

    // Apagar COMPLETAMENTE todos los focos
    for (int i = 0; i < NUM_POINTLIGHTS; i++) {
        lightEffects[i].haloIntensity = glm::mix(
            lightEffects[i].haloIntensity,
            0.0f,  // ← COMPLETAMENTE apagado
            deltaTime * 3.0f  // ← Más rápido para efecto dramático
        );
    }

    // Cambiar foco activo
    if (switchTimer > switchInterval) {
        activeLightIndex = (activeLightIndex + 1) % NUM_POINTLIGHTS;
        switchTimer = 0.0f;
    }

    // Encender COMPLETAMENTE el foco activo
    lightEffects[activeLightIndex].haloIntensity = 1.0f;
}

void updatePointLights() {
    /*lightRotation += deltaTime * 0.5f;
    float radius = 25.0f;
    float height = 50.0f;

    for (int i = 0; i < NUM_POINTLIGHTS; i++) {
        float angle = (2.0f * M_PI * i) / NUM_POINTLIGHTS + lightRotation;
        pointLights[i].position = glm::vec3(cos(angle) * radius, height, sin(angle) * radius);
        glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
        pointLights[i].direction = glm::normalize(target - pointLights[i].position);
    }*/
    updateLightEffects();
}

// ===== FUNCIÓN MEJORADA PARA ACTUALIZAR BOMBA Y EXPLOSIÓN =====
void updateBombAndExplosion() {
    // ===== FASE 1: CAÍDA DE LA BOMBA =====
    // Solo actualizar si la bomba no ha explotado
    if (!bombHasExploded) {
        if (bombaY > 2.0f) {
            // Caída de la bomba con física más realista
            bombaY -= bombFallSpeed * deltaTime;
            bombFallSpeed += 0.1f * deltaTime; // Aplicar gravedad progresiva
        }
        else {
            // La bomba toca el suelo - INICIAR EXPLOSIÓN
            bombaY = 2.0f;
            if (!explosionActive) {
                explosionActive = true;
                bombHasExploded = true;
                explosionTimer = 0.0f;
                std::cout << "¡EXPLOSIÓN INICIADA!" << std::endl;
            }
        }
    }

    // ===== FASE 2: EFECTOS DE EXPLOSIÓN ACTIVA =====
    if (explosionActive) {
        // Actualizar cronómetro y progreso general
        explosionTimer += deltaTime;
        float progress = explosionTimer / explosionDuration;

        // ===== EFECTO DE TEMBLOR DE CÁMARA =====
        // Temblor más duradero y realista (decae más lentamente)
        float baseShake = 2.0f * glm::exp(-explosionTimer * 0.8f); // Era 1.5f, ahora más duradero
        float randomShake = glm::sin(explosionTimer * 45.0f) * glm::cos(explosionTimer * 78.0f);
        float impactShake = glm::exp(-explosionTimer * 3.0f) * glm::sin(explosionTimer * 120.0f);
        shakeIntensity = baseShake * (0.7f * randomShake + 0.3f * impactShake);

        // ===== EXPANSIÓN DEL RADIO DE EXPLOSIÓN =====
        // El radio crece suavemente de 0 a maxExplosionRadius
        currentExplosionRadius = maxExplosionRadius * glm::smoothstep(0.0f, 1.0f, progress);

        // ===== EFECTO VISUAL BLANCO CON PARPADEO =====
        // Intensidad máxima al inicio, luego decae con parpadeo
        whiteIntensity = 4.0f * (1.0f - progress) * (0.7f + 0.3f * glm::sin(explosionTimer * 40.0f));

        // ===== CONFIGURACIÓN INICIAL DE LUZ DE EXPLOSIÓN =====
        if (!explosionLightActive) {
            // Configurar la luz como PointLight estándar
            explosionLight.position = explosionOrigin;
            explosionLight.ambient = glm::vec3(0.5f, 0.4f, 0.3f);   // Ambiente cálido
            explosionLight.diffuse = glm::vec3(1.0f, 0.9f, 0.7f);   // Difusa amarilla-naranja
            explosionLight.specular = glm::vec3(0.3f, 0.3f, 0.3f);  // Especular suave
            explosionLight.constant = 1.0f;
            explosionLight.linear = 0.00001f;      // Atenuación mínima para alcance máximo
            explosionLight.quadratic = 0.000001f;  // Atenuación cuadrática casi nula
            explosionLightActive = true;
            std::cout << "Luz de explosión activada" << std::endl;
        }

        // ===== INTENSIDAD DINÁMICA DE LA LUZ =====
        // Flash inicial muy intenso, luego decaimiento exponencial
        if (explosionTimer < 0.5f) {
            // Primer medio segundo: flash inicial máximo
            explosionLight.haloIntensity = 25.0f * (1.0f - explosionTimer * 2.0f);
        }
        else {
            // Después: decaimiento exponencial más suave
            explosionLight.haloIntensity = 8.0f * glm::exp(-(explosionTimer - 0.5f) * 1.5f);
        }

        // ===== INTENSIFICACIÓN PROGRESIVA DE LA LUZ =====
        if (whiteIntensity > 0.0f) {
            // La luz se intensifica conforme crece el radio visual
            float radiusRatio = currentExplosionRadius / maxExplosionRadius;

            // Multiplicar intensidad hasta 4x según el radio
            explosionLight.haloIntensity *= (1.0f + radiusRatio * 3.0f);

            // Hacer la atenuación aún más suave conforme crece la explosión
            explosionLight.linear = 0.00001f * (1.0f - radiusRatio * 0.8f);
            explosionLight.quadratic = 0.000001f * (1.0f - radiusRatio * 0.9f);
        }

        // ===== MODIFICACIÓN DE LUZ DIRECCIONAL PARA EFECTO BLANCO TOTAL =====
        if (whiteIntensity > 0.01f) {
            float whiteBoost = whiteIntensity * 2.5f;
            // Intensificar la luz ambiental y difusa para efecto "flash blanco"
            dirLight.ambient = glm::vec3(0.6f + whiteBoost, 0.6f + whiteBoost, 0.6f + whiteBoost);
            dirLight.diffuse = glm::vec3(1.0f + whiteBoost, 1.0f + whiteBoost, 0.95f + whiteBoost);
        }

        // ===== FINALIZACIÓN DE LA EXPLOSIÓN =====
        if (explosionTimer >= explosionDuration) {
            // Resetear todos los efectos
            explosionActive = false;
            whiteIntensity = 0.0f;
            shakeIntensity = 0.0f;
            modelsDestroyed = true;

            // Desactivar luz de explosión
            explosionLightActive = false;
            explosionLight.haloIntensity = 0.0f;

            // ===== CRÍTICO: Restaurar luz direccional a valores originales =====
            dirLight.ambient = glm::vec3(0.6f, 0.6f, 0.6f);    // Valores originales
            dirLight.diffuse = glm::vec3(1.0f, 1.0f, 0.95f);   // Valores originales

            std::cout << "EXPLOSIÓN FINALIZADA - Presiona R para reiniciar" << std::endl;
        }
    }
}

// Función simple para círculo
unsigned int createSphere(int segments = 32) {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    // Generar vértices
    for (int i = 0; i <= segments; i++) {
        float lat = M_PI * (-0.5f + (float)i / segments);
        float y = sin(lat);
        float r = cos(lat);

        for (int j = 0; j <= segments; j++) {
            float lng = 2 * M_PI * (float)j / segments;
            float x = cos(lng) * r;
            float z = sin(lng) * r;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    // Generar índices
    for (int i = 0; i < segments; i++) {
        for (int j = 0; j < segments; j++) {
            int first = (i * (segments + 1)) + j;
            int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    // Guardar el número de índices
    sphereIndexCount = indices.size();
    return VAO;
}

int main() {
    // Inicializar GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Ciudad con Bomba y Explosión", NULL, NULL);
    if (!window) {
        std::cerr << "Fallo al crear ventana GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Fallo al inicializar GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    unsigned int demonioLightmap;
    glGenTextures(1, &demonioLightmap);
    glBindTexture(GL_TEXTURE_2D, demonioLightmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load("textures/demonio_lightmap.png", &width, &height, &nrChannels, 4);
    if (data) {
        GLenum format = GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cerr << "No se pudo cargar demonio_lightmap.png\n";
    }
    stbi_image_free(data);

    Shader sceneShader("shaders/scene.vs", "shaders/scene.fs");
    Shader lightShader("shaders/lightning.vs", "shaders/lightning.fs");

    Model modelo("model/Ciudad/Ciudad.obj");
    Model modelo2("model/Baphomet/Baphomet.obj");
    Model modelo3("model/Avion/avion.obj");
    Model modelo4("model/Caza/caza.obj");
    Model modelo5("model/Zombie/Zombie.obj");
    Model modelo6("model/Bomba/Bomba.obj");
    Model modelo7("model/Luna/Luna.obj");


    unsigned int sphereVAO = createSphere(64); // 64 segmentos para una esfera suave
    setupPointLights();

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Actualizar animaciones básicas
        z_position += deltaTime * 5.0f;
        if (z_position >= 40.0f) z_position = -60.0f;

        avanceZ += deltaTime * 0.75f;
        if (avanceZ >= 8.7f) avanceZ = 0.0f;

        // ===== ACTUALIZAR BOMBA Y EXPLOSIÓN =====
        updateBombAndExplosion();
        updatePointLights();
        processInput(window);

        // Renderizado
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Matrices con efecto de temblor
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);

        glm::mat4 view = camera.GetViewMatrix();
        if (explosionActive && shakeIntensity > 0.0f) {
            std::uniform_real_distribution<> shakeDist(-shakeIntensity, shakeIntensity);
            view = glm::translate(view, glm::vec3(shakeDist(gen), shakeDist(gen), 0.0f));
        }

        // Renderizar escena
        sceneShader.use();
        sceneShader.setFloat("explosionRadius", currentExplosionRadius);
        sceneShader.setVec3("explosionOrigin", explosionOrigin);
        sceneShader.setFloat("explosionIntensity", whiteIntensity);
        sceneShader.setMat4("projection", projection);
        sceneShader.setMat4("view", view);
        sceneShader.setVec3("viewPos", camera.Position);

        // Configurar luces
        for (int i = 0; i < NUM_POINTLIGHTS; i++) {
            std::string base = "pointLights[" + std::to_string(i) + "]";
            sceneShader.setVec3(base + ".position", pointLights[i].position);
            sceneShader.setVec3(base + ".ambient", pointLights[i].ambient);
            sceneShader.setVec3(base + ".diffuse", pointLights[i].diffuse);
            sceneShader.setVec3(base + ".specular", pointLights[i].specular);
            sceneShader.setFloat(base + ".haloIntensity", lightEffects[i].haloIntensity);
            sceneShader.setFloat(base + ".constant", pointLights[i].constant);
            sceneShader.setFloat(base + ".linear", pointLights[i].linear);
            sceneShader.setFloat(base + ".quadratic", pointLights[i].quadratic);
        }

        // ===== NUEVA: Configurar luz de explosión como PointLight extra =====
        int totalLights = NUM_POINTLIGHTS;
        if (explosionLightActive && explosionLight.haloIntensity > 0.01f) {
            std::string base = "pointLights[" + std::to_string(NUM_POINTLIGHTS) + "]";
            sceneShader.setVec3(base + ".position", explosionLight.position);
            sceneShader.setVec3(base + ".ambient", explosionLight.ambient);
            sceneShader.setVec3(base + ".diffuse", explosionLight.diffuse);
            sceneShader.setVec3(base + ".specular", explosionLight.specular);
            sceneShader.setFloat(base + ".haloIntensity", explosionLight.haloIntensity);
            sceneShader.setFloat(base + ".constant", explosionLight.constant);
            sceneShader.setFloat(base + ".linear", explosionLight.linear);
            sceneShader.setFloat(base + ".quadratic", explosionLight.quadratic);
            totalLights++;
        }

        sceneShader.setInt("numPointLights", totalLights);

        // ===== CONFIGURACIÓN DE SPOTLIGHT =====
        if (!modelsDestroyed) {
            // Posición del primer caza
            glm::vec3 cazaPosition = glm::vec3(0.0f, 34.0f, z_position);

            sceneShader.setVec3("spotLight.position", cazaPosition);
            sceneShader.setVec3("spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
            sceneShader.setVec3("spotLight.ambient", glm::vec3(0.2f, 0.2f, 0.0f));   //AMRILLO
            sceneShader.setVec3("spotLight.diffuse", glm::vec3(8.0f, 8.0f, 0.0f));   // AMARILLO MÁS INTENSO
            sceneShader.setVec3("spotLight.specular", glm::vec3(3.0f, 3.0f, 0.0f));  //AMARILLO
            sceneShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f))); //CONO AMPLIO
            sceneShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f))); //CONO AMPLIIO
            sceneShader.setFloat("spotLight.constant", 1.0f);
            sceneShader.setFloat("spotLight.linear", 0.0001f); // ATENUACIÓN SUPER BAJA(LLEGA AL SUELO)
            sceneShader.setFloat("spotLight.quadratic", 0.00001f); //ATENUACIÓN SUPER BAJA (LLEGA AL SUELO)
            sceneShader.setBool("spotLight.enabled", true);
        }
        else {
            sceneShader.setBool("spotLight.enabled", false);
        }

        sceneShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);

        sceneShader.setVec3("material.ambient", 1.0f, 1.0f, 1.0f);
        sceneShader.setVec3("material.diffuse", 1.0f, 1.0f, 1.0f);
        sceneShader.setVec3("material.specular", 0.0f, 0.0f, 0.0f);
        sceneShader.setFloat("material.shininess", 2.0f);

        sceneShader.setVec3("dirLight.direction", dirLight.direction);
        sceneShader.setVec3("dirLight.ambient", dirLight.ambient);
        sceneShader.setVec3("dirLight.diffuse", dirLight.diffuse);
        sceneShader.setVec3("dirLight.specular", dirLight.specular);

        // Configurar luz de la luna
        sceneShader.setVec3("moonLight.position", moonLight.position);
        sceneShader.setVec3("moonLight.ambient", moonLight.ambient);
        sceneShader.setVec3("moonLight.diffuse", moonLight.diffuse);

        // Renderizar ciudad
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(1.0f));
        sceneShader.setMat4("model", model);
        modelo.Draw(sceneShader);

        // Renderizar luna 
        glm::mat4 model7 = glm::mat4(1.0f);
        model7 = glm::translate(model7, glm::vec3(0.0f, 40.0f, -50.0f));
        model7 = glm::scale(model7, glm::vec3(0.008f));
        sceneShader.setMat4("model", model7);
        modelo7.Draw(sceneShader);
        
        // Renderizar demonio 
        if (!modelsDestroyed) {
            // Activar el uso del light map SÓLO para el demonio
            sceneShader.setBool("useLightMap", true);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, demonioLightmap);
            sceneShader.setInt("lightMap", 3);

            glm::mat4 model2 = glm::mat4(1.0f);
            model2 = glm::translate(model2, glm::vec3(5.0f, 13.0f, -40.0f));
            model2 = glm::scale(model2, glm::vec3(0.008f));
            sceneShader.setMat4("model", model2);
            modelo2.Draw(sceneShader);

            // Importante: Desactivar después para que no afecte a otros modelos
            sceneShader.setBool("useLightMap", false);
        }

        // Desactivar light map para los siguientes modelos
        sceneShader.setBool("useLightMap", false);
        
        //Renderizar avión (solo si no está destruido)
        if (!modelsDestroyed) {
            glm::mat4 model3 = glm::mat4(1.0f);
            model3 = glm::translate(model3, glm::vec3(5.0f, 34.0f, z_position));
            model3 = glm::rotate(model3, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model3 = glm::scale(model3, glm::vec3(1.0f));
            sceneShader.setMat4("model", model3);
            modelo3.Draw(sceneShader);
        }
        
        // Renderizar cazas (solo si no están destruidos)
        if (!modelsDestroyed) {
            for (int i = 0; i < 4; ++i) {
                glm::mat4 model4 = glm::mat4(1.0f);
                float x_offset = i * 15.0f;
                model4 = glm::translate(model4, glm::vec3(0.0f + x_offset, 34.0f, z_position));
                model4 = glm::rotate(model4, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
                model4 = glm::scale(model4, glm::vec3(1.5f));
                sceneShader.setMat4("model", model4);
                modelo4.Draw(sceneShader);
            }
        }

        // Renderizar zombies (solo si no están destruidos)
        if (!modelsDestroyed) {
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 7; ++j) {
                    glm::mat4 model5 = glm::mat4(1.0f);
                    model5 = glm::translate(model5, glm::vec3(
                        5.0f + (j - 2) * 2.0f,
                        -0.15f,
                        -29.0f + i * 2.5f + avanceZ
                    ));
                    model5 = glm::scale(model5, glm::vec3(0.51f));
                    sceneShader.setMat4("model", model5);
                    modelo5.Draw(sceneShader);
                }
            }
        }
        
        // Renderizar bomba (solo si no ha explotado completamente)
        if (!bombHasExploded || explosionActive && z_position > 10.0f) {
            glm::mat4 model6 = glm::mat4(1.0f);
            model6 = glm::translate(model6, glm::vec3(5.0f, bombaY, -40.0f));
            model6 = glm::rotate(model6, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 1.0f));
            model6 = glm::scale(model6, glm::vec3(0.50f));
            sceneShader.setMat4("model", model6);
            modelo6.Draw(sceneShader);
        }
        

        // Renderizar fuentes de luz
        lightShader.use();
        lightShader.setMat4("projection", projection);
        lightShader.setMat4("view", view);

        glBindVertexArray(sphereVAO);
        for (int i = 0; i < NUM_POINTLIGHTS; i++) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, pointLights[i].position);
            model = glm::scale(model, glm::vec3(1.5f * lightEffects[i].haloIntensity));
            lightShader.setMat4("model", model);
            lightShader.setVec3("lightColor", pointLights[i].diffuse * lightEffects[i].haloIntensity);
            glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
        }

        // Efecto visual de explosión con esfera
        if (whiteIntensity > 0.0f) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE); // Mezcla aditiva para efecto brillante

            lightShader.use();
            lightShader.setMat4("projection", projection);
            lightShader.setMat4("view", view);

            // Parámetros base
            const float time = (float)glfwGetTime();
            const float pulseSpeed = 15.0f;
            const float rotationSpeed = 20.0f;

            // Tres capas concéntricas (como en tu versión original)
            for (int layer = 0; layer < 3; layer++) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, explosionOrigin);

                // Escala con efecto de pulso
                float layerScale = currentExplosionRadius * (0.3f + layer * 0.25f) *
                    (1.0f + 0.1f * sin(time * pulseSpeed + layer));
                model = glm::scale(model, glm::vec3(layerScale));

                // Rotación en diferentes ejes para cada capa
                glm::vec3 axis;
                switch (layer) {
                case 0: axis = glm::vec3(1.0f, 0.5f, 0.0f); break;
                case 1: axis = glm::vec3(0.0f, 1.0f, 0.5f); break;
                case 2: axis = glm::vec3(0.5f, 0.0f, 1.0f); break;
                }
                model = glm::rotate(model, time * rotationSpeed * (1.0f - layer * 0.3f), axis);

                lightShader.setMat4("model", model);

                // Transición de color como en tu versión original
                glm::vec3 layerColor;
                switch (layer) {
                case 0: layerColor = glm::vec3(1.0f, 1.0f, 0.9f); break; // Blanco-amarillo
                case 1: layerColor = glm::vec3(1.0f, 0.7f, 0.3f); break; // Amarillo-naranja
                case 2: layerColor = glm::vec3(1.0f, 0.4f, 0.1f); break; // Naranja-rojo
                }

                float intensity = whiteIntensity * (3.0f - layer) * 0.7f;
                lightShader.setVec3("lightColor", layerColor * intensity);

                // Dibujar la esfera
                glBindVertexArray(sphereVAO);
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
            }

            // Núcleo central brillante
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, explosionOrigin);
                float coreScale = currentExplosionRadius * 0.2f *
                    (1.0f + 0.3f * sin(time * 25.0f));
                model = glm::scale(model, glm::vec3(coreScale));

                lightShader.setMat4("model", model);
                lightShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 0.8f) * whiteIntensity * 4.0f);
                glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
            }

            glEnable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}