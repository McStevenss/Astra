#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdio.h>
#include <iostream>
#include "Shader.hpp"
#include "TerrainMap.h"
#include "TerrainChunk.hpp"
#include "stb_image.h"
#include "Shader.hpp"
#include "Cube.h"
// #include <vector>

class HDRIBox{
    public:
        HDRIBox() = default;
        void Init(float ScreenWidth, float ScreenHeight);
        void Render(glm::mat4 View, glm::mat4 Projection, bool showIrradianceMap);

        GLuint envCubemap;
        GLuint irradianceMap;
        
        private:
        void LoadHDRITexture();
        void CreateFramebuffers();
        void RenderProjections();
        void CreateIrradianceMap();
        // void CalculateIrradiance();
        void CalculateIrradiance(float ScreenWidth, float ScreenHeight);
        
        glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
        glm::mat4 captureViews[6] = {
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
            glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
        };
        Shader* equirectangularToCubemapShader;
        Shader* irradianceShader;
        Shader* backgroundShader;
        Shader* prefilterShader;

        Cube* envBox;
        
        int cubemapSize = 2048; //512
        int irradianceSize = 64;
        int HDRImageColorCap = 150;

        GLuint hdrTexture;  

        
        // --- Framebuffers/render buffers ---
        GLuint captureFBO;
        GLuint captureRBO;

};

