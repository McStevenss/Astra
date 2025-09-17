#pragma once 
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.hpp"
#include "TerrainMap.h"
#include "Animator.h"
#include "Model.hpp"
#include "Animation.h"
#include "Camera.hpp"
class Player
{
    public:
        // Player(glm::vec3 position, glm::vec3 scale = glm::vec3(1.0f));
        Player(glm::vec3 position, glm::vec3 scale = glm::vec3(1.0f), string const &modelPath = "models/vampire_base/Vampire A Lusth.dae");
        Player(){mPosition = glm::vec3(1.0f);};
        // void Render(Shader &shader, float yaw);
        void Render(Shader &shader, Camera &camera);
        void Update(float deltaTime, TerrainMap& terrainMap);
        void HandleInput(float dt, const glm::vec3& fwd, const glm::vec3& right, bool rmb, bool lmb);
        void LoadAnimations();

        glm::mat4 mModelMatrix = glm::mat4(1.0f);
        glm::vec3 mPosition = glm::vec3(1.0f);
        glm::vec3 mVelocity = glm::vec3(0.0f);
        bool isGrounded = true;
        bool isSliding = false;

    private:
        float gravityConstant =-19.81f;
        float terrainSnapTreshhold = 0.05f;
        float jumpStrength = 8.0f;     // tweak for desired jump height

        float vertices[216]; 
        unsigned int VBO=0; 
        unsigned int VAO=0;
        unsigned int EBO=0;
        float rotationOffset = 90.0;
        Animator* animator;
        Model* meshModel;
        std::map<std::string, Animation> animations;
};