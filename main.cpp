#include "Helpers.h"

float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::vec3 cameraPos, cameraFront, cameraUp;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
glm::vec3 thirdPersonCameraPos(0.0f, 6.0f, 13.0f);
glm::vec3 thirdPersonCameraFront(0.0f, 0.0f, -1.0f);
glm::vec3 thirdPersonCameraUp(0.0f, 1.0f, 0.0f);
bool showUI = true;
float uiUpdateTimer = 0.0f;
const float UI_UPDATE_INTERVAL = 0.1f;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

int main()
{
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fish Swimming - Press H for UI Toggle", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    loadFont("fonts/Arial.ttf");

    // Load shaders
    unsigned int shaderProgram = createShaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");

    // Initialize text rendering
    initTextRendering();

    // Load models
    MeshData fishData = loadFBX("model/fishClown.fbx");
    MeshData octopusData = loadFBX("model/octopus.fbx");
    MeshData shipData = loadFBX("model/ship.fbx");

    // Load textures
    unsigned int oceanTexture = loadTexture("textures/ocean.jpg");

    // Lighting
    glm::vec3 lightPos(0.0f, 5.0f, 5.0f);

    // Fish position and swimming parameters
    float swimTime = 0.0f;

    // Room (walls)
    float roomVertices[] = {
        // Bottom (-Y)
        -20.0f, -20.0f, -20.0f,  0, 1, 0,  0, 0,
        20.0f, -20.0f, -20.0f,  0, 1, 0,  1, 0,
        20.0f, -20.0f,  20.0f,  0, 1, 0,  1, 1,
        -20.0f, -20.0f,  20.0f,  0, 1, 0,  0, 1,

        // Top (+Y)
        -20.0f,  20.0f, -20.0f,  0, -1, 0,  0, 0,
        20.0f,  20.0f, -20.0f,  0, -1, 0,  1, 0,
        20.0f,  20.0f,  20.0f,  0, -1, 0,  1, 1,
        -20.0f,  20.0f,  20.0f,  0, -1, 0,  0, 1,

        // Left (-X)
        -20.0f, -20.0f, -20.0f,  1, 0, 0,  0, 0,
        -20.0f,  20.0f, -20.0f,  1, 0, 0,  1, 0,
        -20.0f,  20.0f,  20.0f,  1, 0, 0,  1, 1,
        -20.0f, -20.0f,  20.0f,  1, 0, 0,  0, 1,

        // Right (+X)
        20.0f, -20.0f, -20.0f, -1, 0, 0,  0, 0,
        20.0f,  20.0f, -20.0f, -1, 0, 0,  1, 0,
        20.0f,  20.0f,  20.0f, -1, 0, 0,  1, 1,
        20.0f, -20.0f,  20.0f, -1, 0, 0,  0, 1,

        // Front (+Z)
        -20.0f, -20.0f,  20.0f,  0, 0, -1, 0, 0,
        20.0f, -20.0f,  20.0f,  0, 0, -1, 1, 0,
        20.0f,  20.0f,  20.0f,  0, 0, -1, 1, 1,
        -20.0f,  20.0f,  20.0f,  0, 0, -1, 0, 1,

        // Back (-Z)
        -20.0f, -20.0f, -20.0f,  0, 0, 1,  0, 0,
        20.0f, -20.0f, -20.0f,  0, 0, 1,  1, 0,
        20.0f,  20.0f, -20.0f,  0, 0, 1,  1, 1,
        -20.0f,  20.0f, -20.0f,  0, 0, 1,  0, 1,
    };

    unsigned int roomIndices[] = {
        // Bottom
        0, 1, 2, 2, 3, 0,
        // Top
        4, 5, 6, 6, 7, 4,
        // Left
        8, 9,10,10,11, 8,
        // Right
        12,13,14,14,15,12,
        // Front
        16,17,18,18,19,16,
        // Back
        20,21,22,22,23,20
    };

    unsigned int roomVAO, roomVBO, roomEBO;
    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);
    glGenBuffers(1, &roomEBO);
    glBindVertexArray(roomVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), roomVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roomEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(roomIndices), roomIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        // Time calculations
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        swimTime += deltaTime;
        uiUpdateTimer += deltaTime;

        processInput(window);

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use main shader program
        glUseProgram(shaderProgram);
        unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // Fish
        float radius = 2.0f;
        float speed = 1.0f;             
        float baseHeight = -1.0f;       
        float bobAmplitude_fish = 0.3f;      
        float angle = swimTime * speed;
        float fishX = radius * std::cos(angle);
        float fishZ = radius * std::sin(angle);
        float fishY = baseHeight + std::sin(swimTime * 3.0f) * bobAmplitude_fish;
        glm::vec3 fishPos = glm::vec3(fishX, fishY, fishZ);
        glm::vec3 fishDir = glm::normalize(glm::vec3(-std::sin(angle), 0.0f, std::cos(angle)));
        glm::mat4 fishModel = glm::mat4(1.0f);
        fishModel = glm::translate(fishModel, fishPos);
        float fishRotationAngle = std::atan2(fishDir.x, fishDir.z);
        fishModel = glm::rotate(fishModel, fishRotationAngle, glm::vec3(1, 1, 1));
        fishModel = glm::scale(fishModel, glm::vec3(0.1f));

        // Variables for movement
        float bobAmplitude_octopus = 0.1f;
        float swayAmplitude = 0.05f;
        float swaySpeed = 4.f;
        float rotationAmplitude = 0.1f;
        float bobOffset = std::sin(swimTime * 2.0f) * bobAmplitude_octopus;
        float swayOffset = std::sin(swimTime * swaySpeed) * swayAmplitude;
        float rotationOffset = std::sin(swimTime * 0.5f) * rotationAmplitude;
        float xOffset = 10.0f;

        // Octopus
        glm::vec3 octopusPos = glm::vec3(xOffset + swayOffset, baseHeight + bobOffset, 0.0f);
        glm::mat4 octopusModel = glm::mat4(1.0f);
        octopusModel = glm::translate(octopusModel, octopusPos);
        octopusModel = glm::rotate(octopusModel, rotationOffset, glm::vec3(0.0f, 1.0f, 0.0f));
        octopusModel = glm::scale(octopusModel, glm::vec3(2.f));

        // Ship
        glm::vec3 shipPos = glm::vec3(-xOffset + swayOffset, baseHeight + bobOffset, 0.0f);
        glm::mat4 shipModel = glm::mat4(1.0f);
        shipModel = glm::translate(shipModel, shipPos);
        shipModel = glm::rotate(shipModel, rotationOffset, glm::vec3(0.0f, 1.0f, 0.0f));
        shipModel = glm::scale(shipModel, glm::vec3(0.1f));

        // Camera
        glm::mat4 view;
        glm::vec3 cameraPos;
        glm::vec3 cameraFront;
        glm::vec3 cameraUp;

        if (firstPerson) {
            float camHeight = 0.5f;
            float camDistanceBehind = 1.0f;

            cameraPos = fishPos + fishDir * camDistanceBehind + glm::vec3(0.0f, camHeight, 0.0f);
            cameraFront = fishDir;
            cameraUp = glm::vec3(0, 1, 0);

            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }
        else {
            // Use the controllable third person camera
            cameraPos = thirdPersonCameraPos;
            cameraFront = thirdPersonCameraFront;
            cameraUp = thirdPersonCameraUp;

            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }

        // Upload camera uniforms
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

        // Define uniforms for drawing
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        unsigned int hasTextureLoc = glGetUniformLocation(shaderProgram, "hasTexture");
        unsigned int textureLoc = glGetUniformLocation(shaderProgram, "textureSampler");

        // Draw ocean room (walls) 
        glm::mat4 roomModel = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(roomModel));
        glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(0.0f, 0.2f, 0.4f)));
        glUniform1i(hasTextureLoc, 1);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, oceanTexture);
        glUniform1i(textureLoc, 0);
        glBindVertexArray(roomVAO);
        glDrawElements(GL_TRIANGLES, 36 * 6, GL_UNSIGNED_INT, 0);

        // Draw fish
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(fishModel));
        glUniform3fv(colorLoc, 1, glm::value_ptr(fishData.color));
        glUniform1i(hasTextureLoc, fishData.mesh.hasTexture ? 1 : 0);
        if (fishData.mesh.hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fishData.mesh.textureID);
            glUniform1i(textureLoc, 0);
        }
        glBindVertexArray(fishData.mesh.VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)fishData.mesh.indexCount, GL_UNSIGNED_INT, 0);

        // Draw octopus
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(octopusModel));
        glUniform3fv(colorLoc, 1, glm::value_ptr(octopusData.color));
        glUniform1i(hasTextureLoc, octopusData.mesh.hasTexture ? 1 : 0);
        if (octopusData.mesh.hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, octopusData.mesh.textureID);
            glUniform1i(textureLoc, 0);
        }
        glBindVertexArray(octopusData.mesh.VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)octopusData.mesh.indexCount, GL_UNSIGNED_INT, 0);

        // Draw ship
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(shipModel));
        glUniform3fv(colorLoc, 1, glm::value_ptr(shipData.color));
        glUniform1i(hasTextureLoc, shipData.mesh.hasTexture ? 1 : 0);
        if (shipData.mesh.hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, shipData.mesh.textureID);
            glUniform1i(textureLoc, 0);
        }
        glBindVertexArray(shipData.mesh.VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)shipData.mesh.indexCount, GL_UNSIGNED_INT, 0);

        // Render UI overlay
        if (showUI) {
            renderUI(swimTime, fishPos, octopusPos, shipPos);
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &fishData.mesh.VAO);
    glDeleteBuffers(1, &fishData.mesh.VBO);
    glDeleteBuffers(1, &fishData.mesh.EBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(textShaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    
    // Only handle WASD movement in third person mode
    if (!firstPerson) {
        float cameraSpeed = 5.0f * deltaTime; // adjust accordingly
        
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            thirdPersonCameraPos += cameraSpeed * thirdPersonCameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            thirdPersonCameraPos -= cameraSpeed * thirdPersonCameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            thirdPersonCameraPos -= glm::normalize(glm::cross(thirdPersonCameraFront, thirdPersonCameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            thirdPersonCameraPos += glm::normalize(glm::cross(thirdPersonCameraFront, thirdPersonCameraUp)) * cameraSpeed;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        firstPerson = !firstPerson;
        
        // Reset mouse state when switching to third person
        if (!firstPerson) {
            firstMouse = true;
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    // Only handle mouse in third person mode
    if (firstPerson) return;
    
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    // Update third person camera front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    thirdPersonCameraFront = glm::normalize(front);
}