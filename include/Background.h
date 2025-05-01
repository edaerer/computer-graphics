#pragma once
#include "Shaders.h"
#include "VAO.h"
#include "EBO.h"
#include "Texture.h"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Background {
    public:
        Background(Shader& shader, Texture& texture);
        void Draw(const glm::mat4& rotation_matrix, const glm::vec3& transVec, const glm::vec3& scaleVec, bool rotate=0);
    private:
        Shader& shader;
        Texture& texture;
        VAO vao;
        const GLfloat vertices[32] = {
            -1.0f, -1.0f, 0.0f,   0.0f, 0.0f,               0.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 0.0f,    1.0f, 0.0f,               0.0f, 0.0f, 1.0f,
            1.0f,  0.0f, 0.0f,    1.0f, 1.0f,               0.0f, 0.0f, 1.0f,
            -1.0f,  0.0f, 0.0f,   0.0f, 1.0f,               0.0f, 0.0f, 1.0f
        };
        const GLuint indices[6] = {
            0, 1, 2,
            2, 3, 0
        };
};