#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

void renderText(const char* text, float x, float y, glm::vec3 color, unsigned int shaderProgram)
{
    static char buffer[99999]; // vertex buffer for stb_easy_font

    int num_quads = stb_easy_font_print(x, y, (char*)text, NULL, buffer, sizeof(buffer));

    static unsigned int vao = 0, vbo = 0;
    if (vao == 0) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, num_quads * 4 * 2 * sizeof(float), buffer, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, (void*)0);

    glUseProgram(shaderProgram);

    // Send uniforms
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "textColor");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

    glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(ortho));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    // Draw quads
    glDrawArrays(GL_QUADS, 0, num_quads * 4);

    // Cleanup
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Globals for camera mode
bool firstPerson = false;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Camera parameters
glm::vec3 cameraPos, cameraFront, cameraUp;

// Load shaders from file helper (simplified)
std::string loadShaderSource(const char* filepath)
{
    std::string code;
    std::ifstream file(filepath, std::ios::in);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << filepath << std::endl;
        return "";
    }
    std::string line;
    while (getline(file, line)) code += line + "\n";
    file.close();
    return code;
}

// Texture loading function
unsigned int loadTexture(const std::string& path)
{
    std::cout << "[DEBUG] loadTexture called with path: " << path << std::endl;
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        std::cout << "Loading texture from: " << path << std::endl;
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Compile and link shaders
unsigned int createShaderProgram(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode = loadShaderSource(vertexPath);
    std::string fragmentCode = loadShaderSource(fragmentPath);

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation error:\n" << infoLog << std::endl;
    }

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation error:\n" << infoLog << std::endl;
    }

    // Shader Program
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Shader linking error:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

// Data structures to hold mesh data
struct Mesh {
    unsigned int VAO, VBO, EBO;
    size_t indexCount;
    unsigned int textureID;
    bool hasTexture;
};

struct MeshData {
    Mesh mesh;
    glm::vec3 color;
};

MeshData loadOBJ(const std::string& filename)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    // Get the directory of the OBJ file for relative texture paths
    std::string objDir = filename.substr(0, filename.find_last_of('/') + 1);

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str(), objDir.c_str());

    if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
    if (!err.empty()) std::cerr << "ERR: " << err << std::endl;
    if (!ret) exit(1);

    std::cout << "Materials loaded: " << materials.size() << std::endl;
    for (size_t i = 0; i < materials.size(); ++i) {
        std::cout << "Material #" << i << ": " << materials[i].name << std::endl;
        std::cout << " Diffuse texture: " << materials[i].diffuse_texname << std::endl;
    }

    glm::vec3 matColor(1.0f, 0.0f, 0.0f); // Default red
    unsigned int textureID = 0;
    bool hasTexture = false;

    // Get the material ID of the first shape
    if (!shapes.empty() && !shapes[0].mesh.material_ids.empty()) {
        int mat_id = shapes[0].mesh.material_ids[0];
        if (mat_id >= 0 && mat_id < static_cast<int>(materials.size())) {
            const auto& mat = materials[mat_id];
            matColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

            if (!mat.diffuse_texname.empty()) {
                std::string texturePath = mat.diffuse_texname;
                std::string altTexturePath = objDir + mat.diffuse_texname;

                textureID = loadTexture(texturePath);
                if (textureID != 0) {
                    hasTexture = true;
                    std::cout << "Loaded texture from: " << texturePath << std::endl;
                } else {
                    textureID = loadTexture(altTexturePath);
                    if (textureID != 0) {
                        hasTexture = true;
                        std::cout << "Loaded texture from: " << altTexturePath << std::endl;
                    } else {
                        std::cout << "Failed to load texture from: " << texturePath
                                  << " or " << altTexturePath << std::endl;
                    }
                }
            }
        } else {
            std::cout << "Material ID out of range or invalid for shape.\n";
        }
    } else {
        std::cout << "No material ID available for shape.\n";
    }

    std::vector<float> vertices; // pos + normal + texcoord
    std::vector<unsigned int> indices;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            float vx = attrib.vertices[3 * index.vertex_index + 0];
            float vy = attrib.vertices[3 * index.vertex_index + 1];
            float vz = attrib.vertices[3 * index.vertex_index + 2];

            float nx = 0.f, ny = 0.f, nz = 0.f;
            if (index.normal_index >= 0) {
                nx = attrib.normals[3 * index.normal_index + 0];
                ny = attrib.normals[3 * index.normal_index + 1];
                nz = attrib.normals[3 * index.normal_index + 2];
            }

            float tx = 0.f, ty = 0.f;
            if (index.texcoord_index >= 0) {
                tx = attrib.texcoords[2 * index.texcoord_index + 0];
                ty = attrib.texcoords[2 * index.texcoord_index + 1];
            }

            vertices.insert(vertices.end(), { vx, vy, vz, nx, ny, nz, tx, ty });
            indices.push_back((unsigned int)(indices.size()));
        }
    }

    Mesh mesh;
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texcoord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    mesh.indexCount = indices.size();
    mesh.textureID = textureID;
    mesh.hasTexture = hasTexture;

    MeshData result;
    result.mesh = mesh;
    result.color = matColor;

    return result;
}

// Input processing
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Callback to switch camera on pressing 'C'
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
        firstPerson = !firstPerson;
}

int main()
{
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to init GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Fish Swimming", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, keyCallback);

    // Load OpenGL with GLAD
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
    MeshData fishData = loadOBJ("fishClown.obj");
    MeshData seaData = loadOBJ("Ocean.obj");

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

    // Main loop
    while (!glfwWindowShouldClose(window))
    {

        // Time calculation
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        swimTime += deltaTime;

        processInput(window);

        glClearColor(0.68f, 0.85f, 0.90f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);

        // --- CHANGE: fish moves CLOCKWISE around origin on circle, with vertical bob ---
        float radius = 2.0f;
        float speed = 1.0f;             // radians per second
        float baseHeight = -1.0f;       // Y plane of fish swimming circle
        float bobAmplitude = 0.3f;      // vertical bobbing

        float angle = swimTime * speed;

        // Clockwise movement: x = r*cos(angle), z = r*sin(angle)
        // For clockwise rotation on XZ plane, fish faces tangent: (-sin(angle), 0, cos(angle))
        float fishX = radius * std::cos(angle);
        float fishZ = radius * std::sin(angle);
        float fishY = baseHeight + std::sin(swimTime * 3.0f) * bobAmplitude;

        glm::vec3 fishPos = glm::vec3(fishX, fishY, fishZ);

        // Fish direction (tangent vector) for rotation and camera
        glm::vec3 fishDir = glm::normalize(glm::vec3(-std::sin(angle), 0.0f, std::cos(angle)));

        // Build fish model matrix
        glm::mat4 fishModel = glm::mat4(1.0f);
        fishModel = glm::translate(fishModel, fishPos);

        // Rotate fish to face swimming direction around Y axis
        float fishRotationAngle = std::atan2(fishDir.x, fishDir.z); // note: atan2(x,z) gives Y-rotation
        fishModel = glm::rotate(fishModel, fishRotationAngle, glm::vec3(0, 1, 0));

        // Optional: slight scale of fish model
        fishModel = glm::scale(fishModel, glm::vec3(0.1f));

        

        // Camera setup
        if (firstPerson) {
            float camHeight = 0.5f;
            float camDistanceBehind = 1.0f;

            glm::vec3 cameraPos = fishPos + fishDir * camDistanceBehind + glm::vec3(0.0f, camHeight, 0.0f);
            glm::vec3 cameraFront = fishDir;
            glm::vec3 cameraUp = glm::vec3(0, 1, 0);

            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

            // Set uniforms
            unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
            unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
            unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

            cameraPos = cameraPos;       // update global if needed
            cameraFront = cameraFront;
            cameraUp = cameraUp;
        }
        else {
            // Third person: fixed camera looking at fish
            glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 10.0f);
            glm::vec3 cameraFront = glm::normalize(fishPos - cameraPos);
            glm::vec3 cameraUp = glm::vec3(0, 1, 0);

            glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

            unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
            unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");
            unsigned int viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

            glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3fv(viewPosLoc, 1, glm::value_ptr(cameraPos));

            cameraPos = cameraPos;       // update global if needed
            cameraFront = cameraFront;
            cameraUp = cameraUp;
        }

        // Set uniforms for lighting and model
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int colorLoc = glGetUniformLocation(shaderProgram, "objectColor");
        unsigned int lightPosLoc = glGetUniformLocation(shaderProgram, "lightPos");
        unsigned int hasTextureLoc = glGetUniformLocation(shaderProgram, "hasTexture");
        unsigned int textureLoc = glGetUniformLocation(shaderProgram, "textureSampler");

        glUniform3fv(lightPosLoc, 1, glm::value_ptr(lightPos));

        // Draw the sea
        glm::mat4 seaModel = glm::mat4(1.0f);
        seaModel = glm::translate(seaModel, glm::vec3(0.0f, -2.0f, 0.0f));  // Move below
        seaModel = glm::scale(seaModel, glm::vec3(10.0f, 1.0f, 10.0f));     // Make large

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(seaModel));
        glUniform3fv(colorLoc, 1, glm::value_ptr(seaData.color));
        glUniform1i(hasTextureLoc, seaData.mesh.hasTexture ? 1 : 0);

        if (seaData.mesh.hasTexture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, seaData.mesh.textureID);
            glUniform1i(textureLoc, 0);
        }

        glBindVertexArray(seaData.mesh.VAO);
        glDrawElements(GL_TRIANGLES, (GLsizei)seaData.mesh.indexCount, GL_UNSIGNED_INT, 0);

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

        // World-space debug cubes
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

        glUniform1i(hasTextureLoc, 0); // Debug cubes don't use textures

        for (size_t i = 0; i < cubePositions.size(); ++i) {
            glm::mat4 cubeModel = glm::mat4(1.0f);
            cubeModel = glm::translate(cubeModel, cubePositions[i]);
            cubeModel = glm::scale(cubeModel, glm::vec3(0.5f)); // half size

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel));
            glUniform3fv(colorLoc, 1, glm::value_ptr(cubeColors[i]));

            glBindVertexArray(debugVAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        }

        // Disable depth test so text always draws on top
        glDisable(GL_DEPTH_TEST);

        // Enable blending for transparency (important for text)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Use your text shader
        glUseProgram(textShaderProgram);

        // Set orthographic projection uniform once
        glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
        unsigned int projLoc = glGetUniformLocation(textShaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(ortho));

        // Set text color uniform (e.g., black)
        unsigned int colorLoc2 = glGetUniformLocation(textShaderProgram, "textColor");
        glUniform3f(colorLoc2, 0.0f, 0.0f, 0.0f);

        // Draw your text (implement your renderText function, see below)
        renderText("Hello, OpenGL!", 10.0f, 10.0f, glm::vec3(1.0f, 1.0f, 1.0f), textShaderProgram);

        // Re-enable depth test for next frame
        glEnable(GL_DEPTH_TEST);

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &fishData.mesh.VAO);
    glDeleteBuffers(1, &fishData.mesh.VBO);
    glDeleteBuffers(1, &fishData.mesh.EBO);

    glDeleteVertexArrays(1, &seaData.mesh.VAO);
    glDeleteBuffers(1, &seaData.mesh.VBO);
    glDeleteBuffers(1, &seaData.mesh.EBO);

    glDeleteProgram(shaderProgram);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}