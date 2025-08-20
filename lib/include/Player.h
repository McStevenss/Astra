#pragma once 
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.hpp"

class Player
{
    public:
        Player(glm::vec3 position, glm::vec3 scale = glm::vec3(1.0f));
        Player(){mPosition = glm::vec3(1.0f);};
        void Render(Shader &shader);
        void Update();
        glm::mat4 model = glm::mat4(1.0f);
        glm::vec3 mPosition = glm::vec3(1.0f);
        //Move the model to the right coords
        float vertices[216]; 
        unsigned int VBO=0; 
        unsigned int VAO=0;
        unsigned int EBO=0;

};