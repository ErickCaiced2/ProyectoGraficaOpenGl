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
    float yoffset = lastY - ypos;  // y al revés

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}
float z_position = -45.0f;  // Posición inicial
float avanceZ = 0.0f;
float bombaY = 33.0f; // posición inicial en Y
void processInput(GLFWwindow* window) {
    float speed = deltaTime*10;
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
}

int main() {
    // Inicializar GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Crear ventana
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Modelo con cámara", NULL, NULL);
    if (!window) {
        std::cerr << "Fallo al crear ventana GLFW\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Capturar el mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Cargar funciones OpenGL con GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Fallo al inicializar GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Shaders
    Shader shader("shaders/model_loading.vs", "shaders/model_loading.fs");

    // Cargar modelo
    Model modelo("C:/Users/ERICK CAICEDO/Documents/Visual Studio 2022/OpenGL/OpenGL/model/Ciudad/Ciudad.obj");  // Ajusta tu ruta aquí
    Model modelo2("C:/Users/ERICK CAICEDO/Documents/Visual Studio 2022/OpenGL/OpenGL/model/Baphomet/Baphomet.obj");
    Model modelo3("C:/Users/ERICK CAICEDO/Documents/Visual Studio 2022/OpenGL/OpenGL/model/Avion/avion.obj");
    Model modelo4("C:/Users/ERICK CAICEDO/Documents/Visual Studio 2022/OpenGL/OpenGL/model/Caza/caza.obj");
    Model modelo5("C:/Users/ERICK CAICEDO/Documents/Visual Studio 2022/OpenGL/OpenGL/model/Zombie/Zombie.obj");
    Model modelo6("C:/Users/ERICK CAICEDO/Documents/Visual Studio 2022/OpenGL/OpenGL/model/Bomba/Bomba.obj");
    // Bucle principal
    while (!glfwWindowShouldClose(window)) {
        // Tiempo por frame
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        z_position += deltaTime * 5.0f;  // Velocidad: 5 unidades por segundo
        if (z_position >= 40.0f) {
            z_position = -45.0f;  // Reinicia a la posición original
        }
        avanceZ += deltaTime * 0.75f;  // velocidad: 1.5 unidades por segundo
        if (avanceZ >= 8.7f)
            avanceZ = 0.0f;
        if (bombaY > 2.0f) {
            bombaY -= 0.8f * deltaTime;
            if (bombaY < 2.0f)
                bombaY = 33.0f; // límite mínimo
        }
        // Entrada
        processInput(window);

        // Renderizado
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Activar shader
        shader.use();

        // Matrices
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 500.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);
        //MAPA
        model = glm::scale(model, glm::vec3(1.00f)); 
        shader.setMat4("model", model);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        modelo.Draw(shader);
        //Demonio
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(5.0f, 13.0f, -40.0f));  // <- Mueve el modelo
        model2 = glm::scale(model2, glm::vec3(0.0080f));              // <- Escala el modelo
        shader.setMat4("model", model2);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        modelo2.Draw(shader);
        //Avion
        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3, glm::vec3(5.0f, 34.0f, z_position)); // Z se mueve
        model3 = glm::rotate(model3, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotación Y
        model3 = glm::scale(model3, glm::vec3(1.0f));
        shader.setMat4("model", model3);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        modelo3.Draw(shader);
        //Cazas
        for (int i = 0; i < 4; ++i) {
            glm::mat4 model4 = glm::mat4(1.0f);

            float x_offset = i * 15.0f; // cada modelo separado 15 unidades en X
            model4 = glm::translate(model4, glm::vec3(0.0f + x_offset, 34.0f, z_position));
            model4 = glm::rotate(model4, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model4 = glm::scale(model4, glm::vec3(1.5f));

            shader.setMat4("model", model4);
            shader.setMat4("projection", projection);
            shader.setMat4("view", view);

            modelo4.Draw(shader);
        }
		//zombies
        glm::mat4 model5 = glm::mat4(1.0f);
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 7; ++j) {
                glm::mat4 model5 = glm::mat4(1.0f);
                model5 = glm::translate(model5, glm::vec3(
                    5.0f + (j - 2) * 2.0f,   // X
                    -0.15f,
                    -29.0f + i * 2.5f + avanceZ  // Z con desplazamiento temporal
                ));
                model5 = glm::scale(model5, glm::vec3(0.51f));
                shader.setMat4("model", model5);
                shader.setMat4("projection", projection);
                shader.setMat4("view", view);
                modelo5.Draw(shader);
            }
        }
		// Bomba
        glm::mat4 model6 = glm::mat4(1.0f);
        model6 = glm::translate(model6, glm::vec3(5.0f, bombaY, -40.0f)); // Usa la variable bombaY
        model6 = glm::rotate(model6, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 1.0f));
        model6 = glm::scale(model6, glm::vec3(0.50f));
        shader.setMat4("model", model6);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        modelo6.Draw(shader);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
