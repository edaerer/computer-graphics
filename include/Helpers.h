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