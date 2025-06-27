#include "Helpers.h"

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;
bool firstPerson = false;
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

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fish Swimming", NULL, NULL);
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

    // Load shaders
    unsigned int shaderProgram = createShaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    unsigned int textShaderProgram = createShaderProgram("shaders/text_vertex.glsl", "shaders/text_fragment.glsl");

    // Load models
    MeshData fishData = loadFBX("model/fishClown.fbx");
    MeshData octopusData = loadFBX("model/Octopus_Whimsy_radish_0429041236_texture.fbx");
    MeshData shipData = loadFBX("model/cartoonic_pirates_ship.fbx");

    // Load textures
    unsigned int oceanTexture = loadTexture("textures/ocean.jpg"); // or .png

    // Lighting
    glm::vec3 lightPos(0.0f, 5.0f, 5.0f);

    // Fish position and swimming parameters
    float swimTime = 0.0f;

    // Debug cube data (for orientation cues)
    float debugCubeVertices[] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f
    };
    
    unsigned int debugCubeIndices[] = {
        0,1,2, 2,3,0,
        4,5,6, 6,7,4,
        0,1,5, 5,4,0,
        2,3,7, 7,6,2,
        0,3,7, 7,4,0,
        1,2,6, 6,5,1
    };

    std::vector<glm::vec3> cubePositions = {
        glm::vec3(0.0f, 0.0f, 0.0f),   // Origin (yellow)
        glm::vec3(5.0f, 0.0f, 0.0f),   // +X
        glm::vec3(-5.0f, 0.0f, 0.0f),  // -X
        glm::vec3(0.0f, 0.0f, 5.0f),   // +Z
        glm::vec3(0.0f, 0.0f, -5.0f),  // -Z
        glm::vec3(0.0f, 5.0f, 0.0f),   // +Y
        glm::vec3(0.0f, -5.0f, 0.0f)   // -Y
    };

    std::vector<glm::vec3> cubeColors = {
        glm::vec3(1.0f, 1.0f, 0.0f),   // Yellow (Origin)
        glm::vec3(1.0f, 0.0f, 0.0f),   // Red (+X)
        glm::vec3(0.0f, 1.0f, 0.0f),   // Green (-X)
        glm::vec3(0.0f, 0.0f, 1.0f),   // Blue (+Z)
        glm::vec3(1.0f, 0.0f, 1.0f),   // Magenta (-Z)
        glm::vec3(0.0f, 1.0f, 1.0f),   // Cyan (+Y)
        glm::vec3(1.0f, 0.5f, 0.0f)    // Orange (-Y)
};

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

    unsigned int debugVAO, debugVBO, debugEBO;
    glGenVertexArrays(1, &debugVAO);
    glGenBuffers(1, &debugVBO);
    glGenBuffers(1, &debugEBO);

    glBindVertexArray(debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(debugCubeVertices), debugCubeVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, debugEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(debugCubeIndices), debugCubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    unsigned int roomVAO, roomVBO, roomEBO;
    glGenVertexArrays(1, &roomVAO);
    glGenBuffers(1, &roomVBO);
    glGenBuffers(1, &roomEBO);

    glBindVertexArray(roomVAO);
    glBindBuffer(GL_ARRAY_BUFFER, roomVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(roomVertices), roomVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, roomEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(roomIndices), roomIndices, GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {
        // 1. Time calculations
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        swimTime += deltaTime;

        processInput(window);

        // 2. Clear screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 3. Use main shader program
        glUseProgram(shaderProgram);

        // 4. Setup projection matrix (perspective)
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // 5. Calculate fish position and direction (swimming in circle with bobbing)
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

        // 6. Build fish model matrix (translate, rotate, scale)
        glm::mat4 fishModel = glm::mat4(1.0f);
        fishModel = glm::translate(fishModel, fishPos);

        float fishRotationAngle = std::atan2(fishDir.x, fishDir.z);
        fishModel = glm::rotate(fishModel, fishRotationAngle, glm::vec3(1, 1, 1));
        fishModel = glm::scale(fishModel, glm::vec3(0.1f));

        // Octopus
        float bobAmplitude_octopus = 0.1f;       // smaller than fish
        float swayAmplitude = 0.05f;     // subtle horizontal sway
        float swaySpeed = 4.f;
        float rotationAmplitude = 0.1f;  // gentle turning
        float bobOffset = std::sin(swimTime * 2.0f) * bobAmplitude_octopus;
        float swayOffset = std::sin(swimTime * swaySpeed) * swayAmplitude;
        float rotationOffset = std::sin(swimTime * 0.5f) * rotationAmplitude;
        float xOffset = 10.0f; // adjust this value to move further right

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

        // 7. Setup view matrix depending on camera mode
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

        // 8. Upload camera uniforms
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
        unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");
 
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

        // 9. Upload lighting uniform
        unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

        // Define uniforms for drawing
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        unsigned int hasTextureLoc = glGetUniformLocation(shaderProgram, "hasTexture");
        unsigned int textureLoc = glGetUniformLocation(shaderProgram, "textureSampler");

        // 10. Draw ocean room (walls)
        glm::mat4 roomModel = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(roomModel));
        glUniform3fv(colorLoc, 1, glm::value_ptr(glm::vec3(0.0f, 0.2f, 0.4f))); // ocean blue
        glUniform1i(hasTextureLoc, 1); // use texture

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, oceanTexture); // ocean texture
        glUniform1i(textureLoc, 0);

        glBindVertexArray(roomVAO);
        glDrawElements(GL_TRIANGLES, 36 * 6, GL_UNSIGNED_INT, 0);

        // 12. Draw fish
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

        // 13. Draw octopus
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

        // 13. Draw ship
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

        // 13. Draw debug cubes (optional)
        glUniform1i(hasTextureLoc, 0); // disable texture for cubes

        for (size_t i = 0; i < cubePositions.size(); ++i) {
            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, cubePositions[i]);
            cubeModel = glm::scale(cubeModel, glm::vec3(0.5f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel));
            glUniform3fv(colorLoc, 1, glm::value_ptr(cubeColors[i]));

            glBindVertexArray(debugVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // 14. Draw UI text (on top)
        // Disable depth test and enable blending
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Use text shader
        glUseProgram(textShaderProgram);

        // Setup orthographic projection with flipped Y axis for text coords
        glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f);
        unsigned int projLocText = glGetUniformLocation(textShaderProgram, "projection");
        glUniformMatrix4fv(projLocText, 1, GL_FALSE, glm::value_ptr(ortho));

        // Set text color to white (or whatever you want)
        unsigned int colorLocText = glGetUniformLocation(textShaderProgram, "textColor");
        glUniform3f(colorLocText, 1.0f, 1.0f, 1.0f);

        // Draw text at (10,10)
        for (int y = 10; y < 200; y += 20) {
            renderText("Test", 10.0f, (float)y, glm::vec3(1.0f, 0.0f, 0.0f), textShaderProgram, SCR_WIDTH, SCR_HEIGHT);
        }

        // Re-enable depth test for other draws
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(0);

        // 15. Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &fishData.mesh.VAO);
    glDeleteBuffers(1, &fishData.mesh.VBO);
    glDeleteBuffers(1, &fishData.mesh.EBO);

    glDeleteProgram(shaderProgram);

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