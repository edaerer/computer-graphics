#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Texture.h"
#include "Camera.h"
#include "Car.h"
#include "Background.h"

// window size
const unsigned int width = 1920;
const unsigned int height = 1000;
// mouse input
float rotationX = 0.0f;
float rotationY = 0.0f;
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

void mouse_callback(GLFWwindow* window, double xpos, double ypos);

int main() {
    if (glfwInit() != GLFW_TRUE) {
        std::cout << "Failed to initialize GLFW." << std::endl;
        exit(EXIT_FAILURE);
    }
    GLFWwindow* window = glfwCreateWindow(width, height, "Eda OpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD." << std::endl;
        exit(EXIT_FAILURE);
    }
    glfwSetCursorPosCallback(window, mouse_callback);

    Shader carShader("./shaders/car.vert", "./shaders/car.frag");
    Shader backgroundShader("./shaders/background.vert", "./shaders/background.frag");

    Texture skyTexture("./textures/sky.jpg", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
    skyTexture.texUnit(backgroundShader, "tex0", 0);

    Texture roadTexture("./textures/road.jpg", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGB, GL_UNSIGNED_BYTE);
    roadTexture.texUnit(backgroundShader, "tex0", 0);

    Car car(carShader);
    Background sky(backgroundShader, skyTexture);
    Background road(backgroundShader, roadTexture);
    
    Camera camera(width, height, glm::vec3(0.0f, -0.5f, 6.0f));
    glm::vec3 lightPos = glm::vec3(0.0f, 100.0f, 0.0f);
    glm::vec3 cameraPos = camera.Position;

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        camera.updateMatrix(45.0f, 0.1f, 100.0f);
        camera.Inputs(window);
        glm::mat4 rotation = glm::mat4(1.0f);
        rotation = glm::rotate(rotation, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));
        rotation = glm::rotate(rotation, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));

        carShader.Activate();
        camera.Matrix(carShader, "camMatrix");
        car.Draw(rotation);

        backgroundShader.Activate();
        camera.Matrix(backgroundShader, "camMatrix");
        glUniform3fv(glGetUniformLocation(backgroundShader.ID, "lightPos"), 1, glm::value_ptr(lightPos));
        glUniform3fv(glGetUniformLocation(backgroundShader.ID, "viewPos"), 1, glm::value_ptr(cameraPos));

        for (int i = -2; i < 3; ++i) {
            glm::vec3 transVecRoad = glm::vec3(i * 10.0f, 3.4f, 1.8f);
            glm::vec3 scaleVecRoad = glm::vec3(5.0f, 6.5f, 4.0f);
            road.Draw(rotation, transVecRoad, scaleVecRoad, 1);
        }
        for (int i = -2; i < 3; ++i) {
            glm::vec3 transVecSky = glm::vec3(i * 10.0f, 3.4f, -3.1f);
            glm::vec3 scaleVecSky = glm::vec3(5.0f, 5.2f, 0.0f);
            sky.Draw(rotation, transVecSky, scaleVecSky);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    skyTexture.Delete();
    roadTexture.Delete();
    carShader.Delete();
    backgroundShader.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static const float sensitivity = 0.3f;

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (firstMouse) {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        float offsetX = xpos - lastX;
        float offsetY = ypos - lastY;

        lastX = xpos;
        lastY = ypos;

        rotationY += offsetX * sensitivity;
        rotationX += offsetY * sensitivity;
    }
}