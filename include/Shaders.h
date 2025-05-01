#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <glad/glad.h>

class Shader {
    public:
        GLuint ID;

        Shader(const std::string& vertexFile, const std::string& fragmentFile);
        void Activate();
        void Delete();
};