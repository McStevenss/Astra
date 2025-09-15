#include "Player.h"


Player::Player(glm::vec3 position, glm::vec3 scale, string const &modelPath)
{
    mPosition = position;
    mModelMatrix = glm::translate(mModelMatrix, mPosition);
    mModelMatrix = glm::scale(mModelMatrix, scale); // a smaller cube
    meshModel = new Model(modelPath,false,true);

    LoadAnimations();
    animator = new Animator(&animations["idle"]);
}

void Player::LoadAnimations()
{
    animations.emplace("idle", Animation("models/animations/Neutral Idle.dae",meshModel));
    animations.emplace("running", Animation("models/animations/Running.dae",meshModel));
    animations.emplace("left", Animation("models/animations/Left Strafe.dae",meshModel));
    animations.emplace("right", Animation("models/animations/Right Strafe.dae",meshModel));
    animations.emplace("backward", Animation("models/animations/Running Backward.dae",meshModel));
    animations.emplace("battlecry", Animation("models/animations/Standing Taunt Battlecry.dae",meshModel));

}
void Player::Render(Shader &shader, Camera &camera)
{
    auto transforms = animator->GetFinalBoneMatrices();
    for (int i = 0; i < transforms.size(); ++i)
        shader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
    
    meshModel->UpdateModelMatrix();
    meshModel->ModelMatrix = glm::rotate(meshModel->ModelMatrix, -camera.playerYaw - glm::radians(rotationOffset), glm::vec3(0.0f, 1.0f, 0.0f));
    shader.setMat4("model", meshModel->ModelMatrix);
    shader.setVec3("viewPos",camera.position());
    meshModel->Draw(shader);
}

void Player::HandleInput(float dt, const glm::vec3& fwd, const glm::vec3& right)
{
    const Uint8* ks = SDL_GetKeyboardState(nullptr);
    // float targetSpeed = (ks[SDL_SCANCODE_LCTRL] ? 25.0f : 10.0f);
    float targetSpeed = (ks[SDL_SCANCODE_LCTRL] ? 25.0f : 5.0f);

    if (!falling)
    {
        
        // --- Handle world movement ---
        glm::vec3 moveDir(0.0f);
        if (ks[SDL_SCANCODE_W]) moveDir -= glm::vec3(fwd.x, 0.0f, fwd.z);
        if (ks[SDL_SCANCODE_A]) moveDir += glm::vec3(right.x, 0.0f, right.z);
        if (ks[SDL_SCANCODE_S]) moveDir += glm::vec3(fwd.x, 0.0f, fwd.z);
        if (ks[SDL_SCANCODE_D]) moveDir -= glm::vec3(right.x, 0.0f, right.z);
        
        
        // --- Handle animations for resulting movement dir ---
        // if (ks[SDL_SCANCODE_W] && ks[SDL_SCANCODE_D] || ks[SDL_SCANCODE_W] && ks[SDL_SCANCODE_A]) animator->PlayAnimation(&animations["running"]);
        if (ks[SDL_SCANCODE_W] && ks[SDL_SCANCODE_D]) 
        {
            animator->PlayAnimation(&animations["right"]);
            rotationOffset = 45.0f;
        }

        else if (ks[SDL_SCANCODE_W] && ks[SDL_SCANCODE_A]) 
        {
            animator->PlayAnimation(&animations["left"]);
            rotationOffset = 135.0f;
        }
        
        else if (ks[SDL_SCANCODE_S] && ks[SDL_SCANCODE_D] || ks[SDL_SCANCODE_S] && ks[SDL_SCANCODE_A]) animator->PlayAnimation(&animations["backward"]);
        
        else {
            if (ks[SDL_SCANCODE_W]) animator->PlayAnimation(&animations["running"]);
            if (ks[SDL_SCANCODE_A]) animator->PlayAnimation(&animations["left"]);
            if (ks[SDL_SCANCODE_S]) animator->PlayAnimation(&animations["backward"]);
            if (ks[SDL_SCANCODE_D]) animator->PlayAnimation(&animations["right"]);

            rotationOffset = 90.0f;
        }
        
        // --- Default idle if no movement key is pressed ---
        if(!ks[SDL_SCANCODE_W] && !ks[SDL_SCANCODE_A] && !ks[SDL_SCANCODE_S] && !ks[SDL_SCANCODE_D]) 
        {
            animator->PlayAnimation(&animations["idle"]);
            rotationOffset = 90.0f;
        }

        if (glm::length(moveDir) > 0.0f) {
            moveDir = glm::normalize(moveDir);
            mVelocity = moveDir * targetSpeed;
        }
        else {
            
            // --- If the velocity is low just set it to zero to avoid unneccesary multiplications
            if (mVelocity.x < 0.1){
                mVelocity.x = 0;
            }
            else{
                mVelocity.x *= 0.5f;
            }
            
            if (mVelocity.z < 0.1){
                mVelocity.z = 0;
            }
            else{
                mVelocity.z *= 0.5f;
            }
        }
    }
}

void Player::Update(float deltaTime, TerrainMap& terrainMap)
{
    float terrainHeight = terrainMap.getTriHeightGlobal(mPosition.x, mPosition.z);
    float terrainDiff   = mPosition.y - terrainHeight;
    float slopeThreshold = 0.55f;
    meshModel->Position = mPosition;
    meshModel->UpdateModelMatrix();
    animator->UpdateAnimation(deltaTime);

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