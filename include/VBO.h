#pragma once
#include <glad/glad.h>

class VBO {
    public:
        GLuint ID;
        
        VBO(const GLfloat* vertices, GLsizeiptr size);
        void Bind();
        void Unbind();
        void Delete();  
};