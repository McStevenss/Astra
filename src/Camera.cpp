#include "Camera.hpp"


Camera::Camera(float fov, float nearPlane, float farPlane)
{
    mFov = fov;
    float nearP=nearPlane; 
    float farP=farPlane;
}

Camera::Camera()
{
}

// glm::vec3 Camera::positionWithCollision(TerrainMap* terrain) const {
//     // Desired orbit offset
//     glm::vec3 offset;
//     offset.x = distance * cosf(pitch) * cosf(yaw);
//     // offset.y = distance * sinf(pitch);
//     offset.y = glm::max(distance * sinf(pitch), 1.7f); // <--
//     offset.z = distance * cosf(pitch) * sinf(yaw);

//     glm::vec3 desired = *targetPos + offset;

//     // --- Collision check ---
//     glm::vec3 from = *targetPos + glm::vec3(0, 1.7f, 0); // player eye/center height
//     glm::vec3 to   = desired;

//     // Step along the ray to see if terrain blocks the line
//     int steps = 100; // resolution
//     glm::vec3 dir = (to - from) / (float)steps;
//     glm::vec3 pos = from;
//     for (int i = 0; i < steps; i++) {
//         pos += dir;
//         float terrainY = terrain->getHeightGlobal(pos.x, pos.z);
//         if (pos.y < terrainY + 0.5f) {  // hit!
//             return pos + glm::vec3(0, 0.5f, 0); // snap just above terrain
//         }
//     }

//     // No collision → use full orbit position
//     return desired;
// }


glm::vec3 Camera::positionWithCollision(TerrainMap* terrain) const {
    // Direction from target to camera based on pitch/yaw
    glm::vec3 dir;
    dir.x = cosf(pitch) * cosf(yaw);
    dir.y = sinf(pitch);
    dir.z = cosf(pitch) * sinf(yaw);
    dir = glm::normalize(dir);

    // glm::vec3 desired = *targetPos + dir * distance;
    // desired.y = *targetPos.y + 1.7f + distance * sinf(pitch);


    glm::vec3 desired = *targetPos + dir * distance;

    // Apply vertical offset based on pitch
    desired.y = (*targetPos).y + 1.7f + distance * sinf(pitch);

    // Collision ray from player eye height
    glm::vec3 from = *targetPos + glm::vec3(0, 1.7f, 0);
    glm::vec3 to   = desired;

    int steps = 100;
    glm::vec3 step = (to - from) / (float)steps;
    glm::vec3 pos = from;

    for (int i = 0; i < steps; i++) {
        pos += step;
        float terrainY = terrain->getHeightGlobal(pos.x, pos.z);
        if (pos.y < terrainY + 0.5f) {
            return pos + glm::vec3(0, 0.5f, 0); // collision adjustment
        }
    }

    return desired;
}

glm::mat4 Camera::view() const {
    return glm::lookAt(position(), *targetPos, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::view(TerrainMap* terrain) const {
    return glm::lookAt(positionWithCollision(terrain), *targetPos + glm::vec3(0, 1.7f, 0), glm::vec3(0, 1, 0));
    // return glm::lookAt(positionWithCollision(terrain), *targetPos, glm::vec3(0, 1, 0));
}

glm::mat4 Camera::proj(float aspect) const
{
    return glm::perspective(glm::radians(mFov), aspect, nearP, farP);
}

glm::vec3 Camera::position() const
{
    //   spherical coordinates → cartesian
        glm::vec3 offset;
        offset.x = distance * cosf(pitch) * cosf(yaw);
        // offset.y = distance * sinf(pitch);
        offset.z = distance * cosf(pitch) * sinf(yaw);
        return *targetPos + offset;
}


// glm::vec3 Camera::position() const
// {
//     // direction vector from target to camera at current pitch/yaw
//     glm::vec3 dir;
//     dir.x = cosf(pitch) * cosf(yaw);
//     dir.y = sinf(pitch);
//     dir.z = cosf(pitch) * sinf(yaw);
//     dir = glm::normalize(dir);

//     return *targetPos + dir * distance; // move along same direction
// }

void Camera::recalculateViewport(SDL_Event e)
{
    float aspect = e.window.data1 / (float)e.window.data2;
    glViewport(0,0,e.window.data1,e.window.data2); 
}
