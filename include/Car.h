#pragma once
#include "Shaders.h"
#include "Window.h"
#include "Body.h"
#include "Wheel.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Car {
    public:
        Car(Shader& shader);
        void Draw(const glm::mat4& rotation_matrix);
    private:
        Shader& shader;
        GLfloat vertices_window[24] = {
            -3.0f, 1.0f,  3.0f,  0.6f, 0.7f, 0.8f,
            -3.0f, 3.0f,  3.0f,  0.6f, 0.7f, 0.8f,
            1.0f, 3.0f,  3.0f,  0.6f, 0.7f, 0.8f,
            1.0f, 1.0f,  3.0f,  0.6f, 0.7f, 0.8f
        };
        GLuint indices_window[6] = {
            0, 1, 2,
            2, 3, 0
        };
        GLfloat vertices_car[48] = {
            -6.f, -3.0f,  3.0f,   1.0f, 0.4f, 0.4f,
            6.0f, -3.0f,  3.0f,   1.0f, 0.4f, 0.4f,
            -6.0f,  0.0f,  3.0f,   1.0f, 0.4f, 0.4f,
            6.0f,  0.0f,  3.0f,   1.0f, 0.4f, 0.4f,

            -6.f, -3.0f, -3.0f,   1.0f, 0.4f, 0.4f,
            6.0f, -3.0f, -3.0f,   1.0f, 0.4f, 0.4f,
            -6.0f,  0.0f, -3.0f,   1.0f, 0.4f, 0.4f,
            6.0f,  0.0f, -3.0f,   1.0f, 0.4f, 0.4f
        };
        GLuint indices_car[36] = {
            0, 1, 2,  1, 2, 3,
            1, 3, 5,  3, 5, 7,
            4, 5, 6,  5, 6, 7,
            0, 2, 4,  2, 4, 6,
            2, 3, 6,  3, 6, 7,
            0, 1, 4,  1, 4, 5
        };
};