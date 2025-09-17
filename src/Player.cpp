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
    animations.emplace("Jump", Animation("models/animations/Jumping Up.dae",meshModel));
    animations.emplace("Falling", Animation("models/animations/Falling Idle.dae",meshModel));

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
    float targetSpeed = (ks[SDL_SCANCODE_LCTRL] ? 25.0f : 5.0f);

    // --- Jump input ---
    if (ks[SDL_SCANCODE_SPACE] && isGrounded && !isSliding) {
        mVelocity.y = jumpStrength; 
        isGrounded = false;
        // animator->PlayAnimation(&animations["jump"]);
    }

    // --- Ground movement only if grounded and not sliding ---
    if (isGrounded)
    {
        
        if(!isSliding)
        {
            glm::vec3 moveDir(0.0f);
            if (ks[SDL_SCANCODE_W]) moveDir -= glm::vec3(fwd.x, 0.0f, fwd.z);
            if (ks[SDL_SCANCODE_A]) moveDir += glm::vec3(right.x, 0.0f, right.z);
            if (ks[SDL_SCANCODE_S]) moveDir += glm::vec3(fwd.x, 0.0f, fwd.z);
            if (ks[SDL_SCANCODE_D]) moveDir -= glm::vec3(right.x, 0.0f, right.z);
            
            
            // animations (your logic kept as-is)
            if (ks[SDL_SCANCODE_W] && ks[SDL_SCANCODE_D]) {
                animator->PlayAnimation(&animations["right"]);
                rotationOffset = 45.0f;
            }
            else if (ks[SDL_SCANCODE_W] && ks[SDL_SCANCODE_A]) {
                animator->PlayAnimation(&animations["left"]);
                rotationOffset = 135.0f;
            }
            else if (ks[SDL_SCANCODE_S] && (ks[SDL_SCANCODE_D] || ks[SDL_SCANCODE_A])) {
                animator->PlayAnimation(&animations["backward"]);
            }
            else {
                if (ks[SDL_SCANCODE_W]) animator->PlayAnimation(&animations["running"]);
                if (ks[SDL_SCANCODE_A]) animator->PlayAnimation(&animations["left"]);
                if (ks[SDL_SCANCODE_S]) animator->PlayAnimation(&animations["backward"]);
                if (ks[SDL_SCANCODE_D]) animator->PlayAnimation(&animations["right"]);
                rotationOffset = 90.0f;
            }
        
            if (!ks[SDL_SCANCODE_W] && !ks[SDL_SCANCODE_A] && !ks[SDL_SCANCODE_S] && !ks[SDL_SCANCODE_D]) 
            {
                animator->PlayAnimation(&animations["idle"]);
                rotationOffset = 90.0f;
            }
            
            // --- Apply ground movement ---
            if (glm::length(moveDir) > 0.0f) {
                moveDir = glm::normalize(moveDir);
                mVelocity.x = moveDir.x * targetSpeed;
                mVelocity.z = moveDir.z * targetSpeed;
            } else {
                if (fabs(mVelocity.x) < 0.01f) mVelocity.x = 0;
                else mVelocity.x *= 0.8f;
                if (fabs(mVelocity.z) < 0.01f) mVelocity.z = 0;
                else mVelocity.z *= 0.8f;
            }
        }
    }
}


void Player::Update(float deltaTime, TerrainMap& terrainMap)
{
    float terrainHeight = terrainMap.getTriHeightGlobal(mPosition.x, mPosition.z);
    float terrainDiff   = mPosition.y - terrainHeight;
    glm::vec3 normal = terrainMap.getNormalGlobal(mPosition.x, mPosition.z);
    
    float slopeAngle = glm::degrees(acos(glm::dot(normal, glm::vec3(0,1,0))));
    float maxSlopeDeg = 50.0f;

    // --- Apply airborne gravity ---
    if (!isGrounded) {
        mVelocity.y += gravityConstant * deltaTime;
    }
    
    // if (isGrounded && slopeAngle > maxSlopeDeg) {
    if (slopeAngle > maxSlopeDeg) {
        glm::vec3 accel = terrainMap.getDownhillAccelFromNormal(normal, gravityConstant);
        mVelocity += accel * deltaTime;
        mVelocity -= glm::dot(mVelocity, normal) * normal;  
        isSliding = true;
        // Project the velocity onto the tile/plane we're one so it doesnt explode or aggressively accumulate.
    }
    else if (isGrounded && slopeAngle < maxSlopeDeg){
        mVelocity.y = 0.0f;
        isSliding = false;
    }


    // --- Default terrain snap if in air and underground
    if (terrainDiff <= 0.0f && !isGrounded){
        isGrounded = true;
    } 

    // --- Snap player to terrain if terraindiff is not that much to avoid bounching or flying downhill
    if (terrainDiff <= terrainSnapTreshhold) 
    {
        mPosition.y = terrainHeight;
        isGrounded = true;
    }
    else 
    {
        isGrounded = false;
    }

    // --- Apply velocity and update visuals ---
    mPosition += mVelocity * deltaTime;
    meshModel->Position = mPosition;

    if(mVelocity.y > 0){
        // animator->PlayAnimation(&animations["Jump"]);
        animator->PlayAnimation(&animations["Falling"]);
    }
    else if(mVelocity.y <= -2){
        animator->PlayAnimation(&animations["Falling"]);
    }


    meshModel->UpdateModelMatrix();
    animator->UpdateAnimation(deltaTime);

}
