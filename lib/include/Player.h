#pragma once 
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.hpp"
#include "TerrainMap.h"
class Player
{
    public:
        Player(glm::vec3 position, glm::vec3 scale = glm::vec3(1.0f));
        Player(){mPosition = glm::vec3(1.0f);};
        void Render(Shader &shader, float yaw);
        void Update(float deltaTime, TerrainMap& terrainMap);
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 mPosition = glm::vec3(1.0f);
        glm::vec3 mVelocity = glm::vec3(0.0f);
        bool onGround = true;
        bool falling = false;
        int jumpVelocity = 5;
    private:
        float gravityConstant =-19.81f;
        float vertices[216]; 
        unsigned int VBO=0; 
        unsigned int VAO=0;
        unsigned int EBO=0;

};