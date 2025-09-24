#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdio.h>
#include <iostream>
#include "Shader.hpp"
// #include <vector>

class Sphere{
    public:

        float vertices[108];
        Sphere();
        void Render();

        unsigned int texture;
        unsigned int VAO, VBO, EBO;

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;
        unsigned int indexCount;

    private:
        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
};

