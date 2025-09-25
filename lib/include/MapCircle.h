#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <stdio.h>
#include <iostream>
#include "Shader.hpp"
#include "TerrainMap.h"
#include "TerrainChunk.hpp"
// #include <vector>

class MapCircle{
    public:

        // MapCircle(float radius=0.5f, int segments=24);
        MapCircle() = default;
        void Init(float radius=0.5f, int segments=24);
        void Render(Shader &shader, glm::mat4 VP, glm::vec3 position);
        void RenderProjectedCircle(Shader &shader, glm::vec3 hit, glm::mat4 VP, float radius, TerrainMap& terrainMap);
        void buildCircle(std::vector<glm::vec3>& out, float radius, int segments);
        void SetColor(glm::vec3 color);
       
    private:
        std::vector<glm::vec3> ring;
        glm::mat4 Mring = glm::mat4(1.0f); // identity
        glm::vec4 pColor = glm::vec4(0.0f,0.0f,0.5f,1.0f);
        float pRadius=0.5;
        int pSegments=24;
        GLuint VBO=0;
        GLuint VAO=0;
        std::vector<glm::vec3> Vertices;
};

