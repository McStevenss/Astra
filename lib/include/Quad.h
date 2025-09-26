#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdio.h>
#include <iostream>
#include "Shader.hpp"
// #include <vector>

class Quad{
    public:
        Quad();
        void Render();

    private:
       GLuint VAO = 0;
       GLuint VBO = 0;
};

