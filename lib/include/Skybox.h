#ifndef SKYBOX_H
#define SKYBOX_H
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdio.h>
#include <iostream>
#include "Shader.hpp"
// #include <vector>

class SkyBox{
    public:

        float vertices[108];
        SkyBox() = default;
        void Init();
        void Render(Shader& shader, const glm::mat4& projection, const glm::mat4& view);
        // SkyBox();
        unsigned int loadCubemap(std::vector<std::string> faces);

        unsigned int cubemapTexture;
        unsigned int VAO, VBO;
    private:
};


#endif