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
#include "Quad.h"
// #include <vector>

class HDRIBox{
    public:
        HDRIBox() = default;
        void Init(float ScreenWidth, float ScreenHeight);
        void Render(glm::mat4 View, glm::mat4 Projection, bool showIrradianceMap);

        GLuint envCubemap;
        GLuint irradianceMap;
        GLuint prefilterMap;
        GLuint brdfLUTTexture;
        
        private:
        void LoadHDRITexture();
        void CreateFramebuffers();
        void RenderProjections();
        void CreateIrradianceMap();
        void CreatePrefilterMap();
        void CreateLUTTexture();

        void CalculatePrefilter();
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
        Shader* brdfShader;

        Cube* envBox;
        Quad* envQuad;
        
        int cubemapSize = 512; //512
        int irradianceSize = 32;
        // int prefilterSize = 128;
        int prefilterSize = 32;
        int HDRImageColorCap = 150;

        GLuint hdrTexture;  

        
        // --- Framebuffers/render buffers ---
        GLuint captureFBO;
        GLuint captureRBO;

};

