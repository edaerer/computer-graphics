#pragma once
#include <vector>
#include <glad/glad.h>
#include <cmath>
#include "VAO.h"
#include "VBO.h"

class Wheel {
    public:
        Wheel(float centerX, float centerY, float radius, float thickness, int segments, int numSpokes);
        void GenerateWheelVertices();
        void Draw();
    private:
        float centerX, centerY, radius, thickness;
        int segments, numSpokes;
        std::vector<float> vertices;
        VAO vao;
};