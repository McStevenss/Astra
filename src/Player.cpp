#include "Player.h"

Player::Player(glm::vec3 position, glm::vec3 scale)
{
    mPosition = position;
    model = glm::translate(model, mPosition);
    model = glm::scale(model, scale); // a smaller cube

      float temp_vertices[] = {
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,

        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,

        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,

        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
        1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,

        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    };

    std::copy(std::begin(temp_vertices), std::end(temp_vertices), vertices); // Copy values into the array
    
    unsigned int indices[] = {
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4,
    8, 9, 10, 10, 11, 8,
    12, 13, 14, 14, 15, 12,
    16, 17, 18, 18, 19, 16,
    20, 21, 22, 22, 23, 20
    };


    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);


    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    //Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

}

void Player::Render(Shader &shader, float yaw)
{
    // shader.setMat4("model", model);
    model = glm::mat4(1.0f);
    glm::vec3 test = mPosition + glm::vec3(0.0f,1.0f,0.0f);
    model = glm::translate(model, test);   // move to current player position
    model = glm::scale(model, glm::vec3(1.0f)); // optional, if you want to scale
    model = glm::rotate(model, -yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setMat4("model",model);
    glBindVertexArray(VAO); 
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// void Player::Update(float deltaTime, TerrainMap& terrainMap)
// {
//     float terrainHeight = terrainMap.getHeightGlobal(mPosition.x,mPosition.z);
//     float terrainDiff = mPosition.y - terrainHeight;
//     float slopeThreshold = 0.15f;
//     float cellSize = terrainMap.getCellSize();

//      // --- slope handling ---
//     float maxSlopeDeg = 45.0f;
//     if (terrainMap.isTooSteep(mPosition.x, mPosition.z, maxSlopeDeg)) {
//         glm::vec3 slideDir = terrainMap.getSlideDirection(mPosition.x, mPosition.z);

//         // scale slide speed based on steepness (more steep = faster slide)
//         glm::vec2 grad = terrainMap.getGradient(mPosition.x, mPosition.z, cellSize);
//         float slopeStrength = glm::length(grad); // ~tan(angle)
//         // float slideSpeed = slopeStrength * 5.0f; // tweak multiplier
//         float slideSpeed = 25; // tweak multiplier
        
//         // Apply slide force only along XZ
//         mVelocity.x += slideDir.x * slideSpeed * deltaTime;
//         mVelocity.z += slideDir.z * slideSpeed * deltaTime;
//         if(terrainDiff<= slopeThreshold){
//             mPosition.y = terrainHeight;
//         }
//     }
//     else{
//         // mVelocity.y = 0;
//         // mVelocity.x = 0;
//         // mVelocity.z = 0;
//         mVelocity = glm::vec3(0.0f);
//         mPosition.y = terrainHeight;
//     }
    
//     mPosition += mVelocity * deltaTime;
// }

void Player::Update(float deltaTime, TerrainMap& terrainMap)
{
    // float terrainHeight = terrainMap.getHeightGlobal(mPosition.x, mPosition.z);
    float terrainHeight = terrainMap.getTriHeightGlobal(mPosition.x, mPosition.z);
    float terrainDiff   = mPosition.y - terrainHeight;
    float slopeThreshold = 0.55f;

    // if (terrainDiff > slopeThreshold) {
    //     // Free fall
    //     mVelocity.y += gravityConstant * deltaTime;
    // }
    // else {
        // Snap to terrain height
        mPosition.y = terrainHeight;
        mVelocity.y = 0.0f;
        falling = false;
        // Check slope
        glm::vec3 normal = terrainMap.getNormalGlobal(mPosition.x, mPosition.z);
        float slopeAngle = glm::degrees(acos(glm::dot(normal, glm::vec3(0,1,0))));

  
        // std::cout << "Slope Angle:" <<slopeAngle << " TerrainDiff:" << terrainDiff << std::endl;
        // printf("normal: (%.1f,%.1f,%.1f)", normal.x,normal.y,normal.z);

        float maxSlopeDeg = 50.0f;
        if (slopeAngle > maxSlopeDeg) {
            glm::vec3 accel = terrainMap.getDownhillAccelFromNormal(normal, gravityConstant);
            mVelocity += accel * deltaTime;
            falling = true;
        }
        else {
            // Ground friction if slope is walkable
            mVelocity.x *= 0.8f;
            mVelocity.z *= 0.8f;
            falling = false;
        }

        // Always correct y to terrain
        mPosition.y = terrainMap.getTriHeightGlobal(mPosition.x, mPosition.z);
    // }

    // Apply velocity
    mPosition += mVelocity * deltaTime;
}