#include "Wheel.h"

Wheel::Wheel(float centerX, float centerY, float radius, float thickness, int segments, int numSpokes)
    : centerX(centerX), centerY(centerY), radius(radius), thickness(thickness), segments(segments), numSpokes(numSpokes) {
    GenerateWheelVertices();
    vao.Bind();
    VBO vbo(reinterpret_cast<const GLfloat*>(vertices.data()), vertices.size() * sizeof(GLfloat));
    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)0);
    vao.LinkAttrib(vbo, 1, 3, GL_FLOAT, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    vao.Unbind();
    vbo.Unbind();
}

void Wheel::GenerateWheelVertices() {
    vertices.clear();

    // Center of the wheel
    vertices.push_back(centerX);
    vertices.push_back(centerY);
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);
    vertices.push_back(1.0f);

    // Rim
    float angleStep = 2.0f * M_PI / segments;
    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);

        // Outer rim vertices
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(thickness);
        vertices.push_back(0.1f);
        vertices.push_back(0.1f);
        vertices.push_back(0.1f);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(-thickness);
        vertices.push_back(0.1f);
        vertices.push_back(0.1f);
        vertices.push_back(0.1f);
    }

    // Spokes
    for (int i = 0; i < numSpokes; ++i) {
        float angle = i * (2.0f * M_PI / numSpokes);
        float x = centerX + radius * cos(angle);
        float y = centerY + radius * sin(angle);

        // Spoke vertices, connecting the center
        vertices.push_back(centerX);
        vertices.push_back(centerY);
        vertices.push_back(thickness);
        vertices.push_back(0.8f);
        vertices.push_back(0.8f);
        vertices.push_back(0.8f);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(thickness);
        vertices.push_back(0.8f);
        vertices.push_back(0.8f);
        vertices.push_back(0.8f);

        vertices.push_back(centerX);
        vertices.push_back(centerY);
        vertices.push_back(-thickness);
        vertices.push_back(0.8f);
        vertices.push_back(0.8f);
        vertices.push_back(0.8f);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(-thickness);
        vertices.push_back(0.8f);
        vertices.push_back(0.8f);
        vertices.push_back(0.8f);
    }
}

void Wheel::Draw() {
    vao.Bind();
    // Rim
    glDrawArrays(GL_TRIANGLE_FAN, 1, segments * 2);
    // Spokes
    glDrawArrays(GL_LINES, segments * 2 + 1, vertices.size() / 6 - (segments * 2 + 1));
}