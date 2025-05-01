#include "Body.h"

Body::Body(Shader& shader, GLfloat* vertices, GLuint* indices) : shader(shader)
{
    vao.Bind();
    VBO vbo(vertices, 8 * 6 * sizeof(GLfloat));
    EBO ebo(indices, 36 * sizeof(GLuint));
    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)0);
    vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    vao.Unbind();
    vbo.Unbind();
    ebo.Unbind();
}

void Body::Draw() {
    shader.Activate();
    vao.Bind();
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}