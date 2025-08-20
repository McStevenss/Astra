#pragma once 
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SDL2/SDL.h>
#include "TerrainMap.h"

class Camera
{
    public:
        Camera(float fov, float nearPlane, float farPlane);
        Camera();
        glm::mat4 view() const;
        glm::mat4 proj(float aspect) const;
        glm::vec3 position() const;
        void recalculateViewport(SDL_Event e);
        glm::vec3 positionWithCollision(TerrainMap* terrain) const;
        glm::mat4 view(TerrainMap* terrain) const;
        glm::vec3 pos;
        glm::vec3* targetPos;
        float mFov=60.0f; 
        
        
        float yaw=1.00f;
        float pitch=-0.35f; // radians
        float nearP=0.1f; 
        float farP=2000.0f;
        float distance = 5.0f;

};


// struct Camera {
//     glm::vec3 pos;
//     float yaw=1.00f, pitch=-0.35f; // radians
//     float fov=60.0f; float nearP=0.1f, farP=2000.0f;
//     float distance = 5.0f;




//     glm::vec3 position() const {
//         // spherical coordinates → cartesian
//         glm::vec3 offset;
//         offset.x = distance * cosf(pitch) * cosf(yaw);
//         offset.y = distance * sinf(pitch);
//         offset.z = distance * cosf(pitch) * sinf(yaw);
//         return target + offset;
//     }

//     glm::mat4 view() const {
//         glm::vec3 f;
//         f.x = cosf(pitch)*cosf(yaw);
//         f.y = sinf(pitch);
//         f.z = cosf(pitch)*sinf(yaw);
//         return glm::lookAt(pos, pos + glm::normalize(f), glm::vec3(0,1,0));
//     }
//     glm::mat4 proj(float aspect) const {
//         return glm::perspective(glm::radians(fov), aspect, nearP, farP);
//     }

//     void recalculateViewport(SDL_Event e){
//         float aspect = e.window.data1 / (float)e.window.data2;
//         glViewport(0,0,e.window.data1,e.window.data2); 
//     }
// };