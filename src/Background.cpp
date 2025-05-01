#include "Background.h"

Background::Background(Shader& shader, Texture& texture) : shader(shader), texture(texture)
{
    vao.Bind();
    VBO vbo(vertices, 4 * 8 * sizeof(GLfloat));
    EBO ebo(indices, 6 * sizeof(GLuint));
    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)0);
    vao.LinkAttrib(vbo, 1, 2, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    vao.LinkAttrib(vbo, 2, 3, GL_FLOAT, 8 * sizeof(GLfloat), (void*)(5 * sizeof(GLfloat)));
    vao.Unbind();
    vbo.Unbind();
    ebo.Unbind();
}

void Background::Draw(const glm::mat4& rotation_matrix, const glm::vec3& transVec, const glm::vec3& scaleVec, bool rotate) {
    shader.Activate();
    texture.Bind();
    glm::mat4 model = glm::mat4(1.0f);
    if (rotate == 1) {
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    model = rotation_matrix * glm::translate(model, transVec);
    model = glm::scale(model, scaleVec);
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model));
    vao.Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    texture.Unbind();
}