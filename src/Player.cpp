#include "Player.h"

Player::Player(glm::vec3 position, glm::vec3 scale)
{
    mPosition = position;
    model = glm::translate(model, mPosition);
    model = glm::scale(model, scale); // a smaller cube

    // float temp_vertices[] = {
    //     -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
    //     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
    //     1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
    //     1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
    //     -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
    //     -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,

    //     -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    //     1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    //     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    //     1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    //     -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
    //     -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,

    //     -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
    //     -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
    //     -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
    //     -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
    //     -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
    //     -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,

    //     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
    //     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
    //     1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
    //     1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
    //     1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
    //     1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,

    //     -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
    //     1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
    //     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
    //     1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
    //     -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
    //     -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,

    //     -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    //     1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f,
    //     1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    //     1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    //     -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
    //     -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
    // };

    float cube_vertices[] = {
        // -Z face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f,

        // +Z face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f,

        // -X face
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f,

        // +X face
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
        1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,
        1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,

        // -Y face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f,
        1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f,

        // +Y face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f
    };


    std::copy(std::begin(cube_vertices), std::end(cube_vertices), vertices); // Copy values into the array
    
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


void Player::HandleInput(float dt, const glm::vec3& fwd, const glm::vec3& right)
{
    const Uint8* ks = SDL_GetKeyboardState(nullptr);
    float targetSpeed = (ks[SDL_SCANCODE_LCTRL] ? 25.0f : 10.0f);

    if (!falling)
    {
        glm::vec3 moveDir(0.0f);
        if (ks[SDL_SCANCODE_W]) moveDir -= glm::vec3(fwd.x, 0.0f, fwd.z);
        if (ks[SDL_SCANCODE_S]) moveDir += glm::vec3(fwd.x, 0.0f, fwd.z);
        if (ks[SDL_SCANCODE_A]) moveDir += glm::vec3(right.x, 0.0f, right.z);
        if (ks[SDL_SCANCODE_D]) moveDir -= glm::vec3(right.x, 0.0f, right.z);

        if (glm::length(moveDir) > 0.0f) {
            moveDir = glm::normalize(moveDir);
            mVelocity = moveDir * targetSpeed;
        }
        else {
            mVelocity.x *= 0.8f;
            mVelocity.z *= 0.8f;
        }
    }
}

void Player::Update(float deltaTime, TerrainMap& terrainMap)
{
    float terrainHeight = terrainMap.getTriHeightGlobal(mPosition.x, mPosition.z);
    float terrainDiff   = mPosition.y - terrainHeight;
    float slopeThreshold = 0.55f;

    falling = false;
    // Check slope
    glm::vec3 normal = terrainMap.getNormalGlobal(mPosition.x, mPosition.z);
    float slopeAngle = glm::degrees(acos(glm::dot(normal, glm::vec3(0,1,0))));

    float maxSlopeDeg = 50.0f;
    if (slopeAngle > maxSlopeDeg) {
        glm::vec3 accel = terrainMap.getDownhillAccelFromNormal(normal, gravityConstant);
        mVelocity += accel * deltaTime;

        // Project the velocity onto the tile/plane we're one so it doesnt explode or aggressively accumulate.
        mVelocity -= glm::dot(mVelocity, normal) * normal;    
        falling = true;
    }
    else{
        mVelocity.y = 0.0f;
    }
   
    mPosition.y = terrainMap.getTriHeightGlobal(mPosition.x, mPosition.z);
    mPosition += mVelocity * deltaTime;
}