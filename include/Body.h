#pragma once
#include "Shaders.h"
#include "VAO.h"
#include "EBO.h"
#include <glad/glad.h>

class Body {
    public:
        Body(Shader& shader, GLfloat* vertices, GLuint* indices);
        void Draw();
    private:
        Shader& shader;
        VAO vao;
};