#include "Window.h"

Window::Window(Shader& shader, GLfloat* vertices, GLuint* indices) : shader(shader)
{
    vao.Bind();
    VBO vbo(vertices, 4 * 6 * sizeof(GLfloat));
    EBO ebo(indices, 6 * sizeof(GLuint));
    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)0);
    vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    vao.Unbind();
    vbo.Unbind();
    ebo.Unbind();
}

void Window::Draw() {
    shader.Activate();
    vao.Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}