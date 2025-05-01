#include "Car.h"

Car::Car(Shader& shader) : shader(shader) {}

void Car::Draw(const glm::mat4& rotation_matrix) {
    Window window(shader, vertices_window, indices_window);
    Body body(shader, vertices_car, indices_car);
    Wheel wheel(0.0f, 0.0f, 0.2f, 0.12f, 100, 6);

    // right-rear window
    glm::mat4 model1 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(-0.25f, -1.1f, 1.0f));
    model1 = glm::scale(model1, glm::vec3(0.15f, 0.15f, 0.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model1));
    window.Draw();

    // right-front window
    glm::mat4 model2 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.55f, -1.1f, 1.0f));
    model2 = glm::scale(model2, glm::vec3(0.15f, 0.15f, 0.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model2));
    window.Draw();

    // left-rear window
    glm::mat4 model3 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(-0.25f, -1.1f, 0.1f));
    model3 = glm::scale(model3, glm::vec3(0.15f, 0.15f, 0.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model3));
    window.Draw();

    // left-front window
    glm::mat4 model4 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.55f, -1.1f, 0.1f));
    model4 = glm::scale(model4, glm::vec3(0.15f, 0.15f, 0.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model4));
    window.Draw();

    // front window
    glm::mat4 model5 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model5 = rotation_matrix * glm::translate(model5, glm::vec3(-0.8f, -1.1f, 0.45f));
    model5 = glm::scale(model5, glm::vec3(0.2f, 0.15f, 0.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model5));
    window.Draw();

    // rear window
    glm::mat4 model6 = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model6 = rotation_matrix * glm::translate(model6, glm::vec3(-0.8f, -1.1f, -1.35f));
    model6 = glm::scale(model6, glm::vec3(0.2f, 0.15f, 0.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model6));
    window.Draw();

    // bottom body
    glm::mat4 model7 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 1.0f));
    model7 = glm::scale(model7, glm::vec3(0.2f, 0.2f, 0.2f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model7));
    body.Draw();

    // top body
    glm::mat4 model8 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.6f, 1.0f));
    model8 = glm::scale(model8, glm::vec3(0.15f, 0.15f, 0.15f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model8));
    body.Draw();

    // right-front wheel
    glm::mat4 model9 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.6f, 1.53f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model9));
    wheel.Draw();

    // left-front wheel
    glm::mat4 model10 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -1.6f, 0.45f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model10));
    wheel.Draw();

    // right-rear wheel
    glm::mat4 model11 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -1.6f, 1.53f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model11));
    wheel.Draw();

    // left-rear wheel
    glm::mat4 model12 = rotation_matrix * glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -1.6f, 0.45f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(model12));
    wheel.Draw();

}