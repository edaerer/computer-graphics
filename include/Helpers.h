#ifndef HELPER_H
#define HELPER_H

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_EASY_FONT_IMPLEMENTATION
#include "stb_easy_font.h"
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

void renderText(const char* text, float x, float y, glm::vec3 color, unsigned int shaderProgram, const unsigned int SCR_WIDTH, const unsigned int SCR_HEIGHT)
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);


    glUseProgram(shaderProgram);

    // Send uniforms
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "textColor");
    unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

    glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_WIDTH, (float)SCR_HEIGHT, 0.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(ortho));
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));

    // Draw quads
    glDrawArrays(GL_TRIANGLES, 0, num_quads * 6);


    // Cleanup
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

#endif