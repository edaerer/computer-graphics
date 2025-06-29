#ifndef HELPER_H
#define HELPER_H

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>
#include <iomanip>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>

const unsigned int SCR_WIDTH = 1300;
const unsigned int SCR_HEIGHT = 1200;
bool firstPerson = false;
unsigned int textShaderProgram;
unsigned int textVAO, textVBO;

const char* textVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // pos.xy, texCoord.xy

out vec2 TexCoords;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

const char* textFragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    float alpha = texture(text, TexCoords).r;
    FragColor = vec4(textColor, alpha);
}
)";

struct Character {
    GLuint TextureID;   // Glyph texture
    glm::ivec2 Size;    // Glyph size
    glm::ivec2 Bearing; // Offset from baseline
    GLuint Advance;     // Horizontal offset to next glyph
};

std::map<GLchar, Character> Characters;

void loadFont(const std::string& fontPath) {
    FT_Library ft;
    FT_Face face;
    FT_Init_FreeType(&ft);
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "ERROR::FREETYPE: Failed to load font\n";
        FT_Done_FreeType(ft);
        return; // or handle error properly
    }
    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

    for (GLubyte c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "ERROR::FREETYPE: Failed to load Glyph " << c << '\n';
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void initTextRendering()
{
    // Compile text shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, vertexShader);
    glAttachShader(textShaderProgram, fragmentShader);
    glLinkProgram(textShaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up text rendering VAO/VBO
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color)
{
    glUseProgram(textShaderProgram);
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.x, color.y, color.z);

    glm::mat4 projection = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(textShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    for (char c : text) {
        Character ch = Characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale; // bitshift by 6 to get pixel value
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}


void renderUI(float swimTime, glm::vec3 fishPos, glm::vec3 octopusPos, glm::vec3 shipPos)
{
    float yOffset = SCR_HEIGHT - 40;
    float lineHeight = 25;
    
    // Title
    renderText("UNDERWATER ANIMATION", 20, yOffset, 1.2f, glm::vec3(0.0f, 0.8f, 1.0f));
    yOffset -= lineHeight * 1.5f;
    
    // Animation time
    std::stringstream timeStr;
    timeStr << "Time: " << std::fixed << std::setprecision(1) << swimTime << "s";
    renderText(timeStr.str(), 20, yOffset, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));
    yOffset -= lineHeight;
    
    // Camera mode
    std::string cameraMode = firstPerson ? "Camera: First Person" : "Camera: Third Person";
    renderText(cameraMode, 20, yOffset, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));
    yOffset -= lineHeight * 1.5f;
    
    // Object positions
    renderText("POSITIONS:", 20, yOffset, 1.0f, glm::vec3(1.0f, 0.8f, 0.0f));
    yOffset -= lineHeight;
    
    // Fish position
    std::stringstream fishStr;
    fishStr << "Fish: (" << std::fixed << std::setprecision(1) 
            << fishPos.x << ", " << fishPos.y << ", " << fishPos.z << ")";
    renderText(fishStr.str(), 20, yOffset, 0.7f, glm::vec3(0.0f, 1.0f, 0.5f));
    yOffset -= lineHeight * 0.8f;
    
    // Octopus position
    std::stringstream octStr;
    octStr << "Octopus: (" << std::fixed << std::setprecision(1) 
           << octopusPos.x << ", " << octopusPos.y << ", " << octopusPos.z << ")";
    renderText(octStr.str(), 20, yOffset, 0.7f, glm::vec3(1.0f, 0.5f, 1.0f));
    yOffset -= lineHeight * 0.8f;
    
    // Ship position
    std::stringstream shipStr;
    shipStr << "Ship: (" << std::fixed << std::setprecision(1) 
            << shipPos.x << ", " << shipPos.y << ", " << shipPos.z << ")";
    renderText(shipStr.str(), 20, yOffset, 0.7f, glm::vec3(0.8f, 0.8f, 0.8f));
    yOffset -= lineHeight * 1.5f;
    
    // Animation status
    renderText("ANIMATIONS:", 20, yOffset, 1.0f, glm::vec3(1.0f, 0.8f, 0.0f));
    yOffset -= lineHeight;
    
    renderText("Fish: Circular swimming + bobbing", 20, yOffset, 0.7f, glm::vec3(0.8f, 0.8f, 0.8f));
    yOffset -= lineHeight * 0.8f;
    
    renderText("Octopus: Gentle swaying + rotation", 20, yOffset, 0.7f, glm::vec3(0.8f, 0.8f, 0.8f));
    yOffset -= lineHeight * 0.8f;
    
    renderText("Ship: Ocean-like movement", 20, yOffset, 0.7f, glm::vec3(0.8f, 0.8f, 0.8f));
    yOffset -= lineHeight * 1.5f;
    
    // Controls
    renderText("CONTROLS:", 20, yOffset, 1.0f, glm::vec3(1.0f, 0.8f, 0.0f));
    yOffset -= lineHeight;
    
    renderText("C - Toggle Camera | ESC - Exit", 20, yOffset, 0.7f, glm::vec3(0.8f, 0.8f, 0.8f));
    yOffset -= lineHeight * 0.8f;
    
    renderText("WASD - Move Camera | Mouse - Look Around", 20, yOffset, 0.7f, glm::vec3(0.8f, 0.8f, 0.8f));
    
    // Animation indicator (bottom right)
    int animFrame = (int)(swimTime * 4) % 4;
    std::string animChars[] = {"|", "/", "-", "\\"};
    renderText("Running " + animChars[animFrame], SCR_WIDTH - 100, 20, 0.8f, glm::vec3(0.0f, 1.0f, 0.0f));
}

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

unsigned int loadTexture(const std::string& path)
{
    std::cout << "[DEBUG] loadTexture called with path: " << path << std::endl;
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        std::cout << "[DEBUG] Loading texture from: " << path << std::endl;
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
        std::cerr << "[DEBUG] Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

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

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

MeshData loadFBX(const std::string& filepath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath,
        aiProcess_Triangulate |
        aiProcess_FlipUVs |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Assimp error: " << importer.GetErrorString() << std::endl;
        exit(1);
    }

    // For simplicity, take the first mesh only:
    aiMesh* mesh = scene->mMeshes[0];

    std::vector<float> vertices; // pos(3), normal(3), texcoord(2)
    std::vector<unsigned int> indices;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        // Positions
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        // Normals
        if (mesh->HasNormals()) {
            vertices.push_back(mesh->mNormals[i].x);
            vertices.push_back(mesh->mNormals[i].y);
            vertices.push_back(mesh->mNormals[i].z);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }

        // Texture coords (only first set)
        if (mesh->HasTextureCoords(0)) {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }

    // Indices
    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // Load material & texture
    glm::vec3 matColor(1.0f, 1.0f, 1.0f); // default white
    unsigned int textureID = 0;
    bool hasTexture = false;

    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        aiColor3D diffuseColor(1.f, 1.f, 1.f);
        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == AI_SUCCESS) {
            matColor = glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
        }

        aiString texPath;
        if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
            std::string texturePath = texPath.C_Str();

            std::replace(texturePath.begin(), texturePath.end(), '\\', '/');

            while (texturePath.find("../") == 0) {
                texturePath = texturePath.substr(3);
            }
            size_t imagesPos = texturePath.find("images/");
            if (imagesPos != std::string::npos) {
                texturePath.erase(imagesPos, 7);
            }
            size_t tempPos = texturePath.find("temp.fbm/");
            if (tempPos != std::string::npos) {
                texturePath.erase(tempPos, 9);
            }
            size_t cartoonicPos = texturePath.find("cartoonic_pirates_ship3.fbm/");
            if (cartoonicPos != std::string::npos) {
                texturePath.erase(cartoonicPos, 28);
            }
            std::cout<<texturePath<<std::endl;

            texturePath = "model/" + texturePath;

            textureID = loadTexture(texturePath);

            if (textureID != 0) {
                hasTexture = true;
            }
        }
    }

    Mesh meshGL;
    glGenVertexArrays(1, &meshGL.VAO);
    glGenBuffers(1, &meshGL.VBO);
    glGenBuffers(1, &meshGL.EBO);

    glBindVertexArray(meshGL.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, meshGL.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshGL.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texcoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    meshGL.indexCount = indices.size();
    meshGL.textureID = textureID;
    meshGL.hasTexture = hasTexture;

    MeshData result;
    result.mesh = meshGL;
    result.color = matColor;

    return result;
}

#endif