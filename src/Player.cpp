#include "Player.h"


Player::Player(glm::vec3 position, glm::vec3 scale, string const &modelPath)
{
    mPosition = position;
    mModelMatrix = glm::translate(mModelMatrix, mPosition);
    mModelMatrix = glm::scale(mModelMatrix, scale); // a smaller cube
    meshModel = new Model(modelPath,false,true);

    LoadAnimations();
    animator = new Animator(&animations[currentAnimationState]);

    circleShader = new Shader("shaders/circle.vs","shaders/circle.fs");

    GenCircleGL();
}

void Player::LoadAnimations()
{
    animations.emplace(AnimationState::Idle,       Animation("models/animations/Neutral Idle.dae", meshModel));
    animations.emplace(AnimationState::Running,    Animation("models/animations/Running.dae", meshModel));
    animations.emplace(AnimationState::StrafeLeft, Animation("models/animations/Left Strafe.dae", meshModel));
    animations.emplace(AnimationState::StrafeRight,Animation("models/animations/Right Strafe.dae", meshModel));
    animations.emplace(AnimationState::Backward,   Animation("models/animations/Running Backward.dae", meshModel));
    animations.emplace(AnimationState::Battlecry,  Animation("models/animations/Standing Taunt Battlecry.dae", meshModel));
    animations.emplace(AnimationState::Jump,       Animation("models/animations/Jumping Up.dae", meshModel));
    animations.emplace(AnimationState::Falling,    Animation("models/animations/Falling Idle.dae", meshModel));
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

void Player::HandleInput(float dt, const glm::vec3& fwd, const glm::vec3& right, bool rmb, bool lmb)
{
    const Uint8* ks = SDL_GetKeyboardState(nullptr);
    float targetSpeed = (ks[SDL_SCANCODE_LCTRL] ? 25.0f : 5.0f);

    glm::vec3 moveDir(0.0f);
  
        
    // --- Ground movement only if grounded and not sliding ---
    if (isGrounded && !isSliding)
    {
        AnimationState newState = AnimationState::Idle;
     
        if (ks[SDL_SCANCODE_SPACE]) {
            mVelocity.y = jumpStrength; 
            isGrounded = false;
        }
        
        // glm::vec3 moveDir(0.0f);
        if (ks[SDL_SCANCODE_W] || (lmb && rmb)){
            moveDir -= glm::vec3(fwd.x, 0.0f, fwd.z);
            newState = AnimationState::Running;
        } 
        if (ks[SDL_SCANCODE_S]){
            moveDir += glm::vec3(fwd.x, 0.0f, fwd.z);
            newState = AnimationState::Backward;
        }
        if (ks[SDL_SCANCODE_A] && rmb) {
            moveDir += glm::vec3(right.x, 0.0f, right.z);
            newState = AnimationState::StrafeLeft;
        }
        if (ks[SDL_SCANCODE_D] && rmb) {
            moveDir -= glm::vec3(right.x, 0.0f, right.z);
            newState = AnimationState::StrafeRight;
        }
        
        
        if (ks[SDL_SCANCODE_W] && ks[SDL_SCANCODE_D] && rmb) {
            rotationOffset = 45.0f;
        }
        else if (ks[SDL_SCANCODE_W] && ks[SDL_SCANCODE_A] && rmb) {
            rotationOffset = 135.0f;
        }
        else{
            rotationOffset = 90.0f;
        }
      
    
        // --- Apply ground movement ---
        if (glm::length(moveDir) > 0.0f) 
        {
            moveDir = glm::normalize(moveDir);
            mVelocity.x = moveDir.x * targetSpeed;
            mVelocity.z = moveDir.z * targetSpeed;
        } 
        else 
        {
            if (fabs(mVelocity.x) < 0.01f) mVelocity.x = 0;
            else mVelocity.x *= 0.8f;
            if (fabs(mVelocity.z) < 0.01f) mVelocity.z = 0;
            else mVelocity.z *= 0.8f;
        }
        
        // --- Animation selection ---
        if (newState != currentAnimationState) {
            currentAnimationState = newState;
            animator->PlayAnimation(&animations[newState]);
        }
    }
}



void Player::Update(float deltaTime, TerrainMap& terrainMap)
{
    float terrainHeight = terrainMap.getTriHeightGlobal(mPosition.x, mPosition.z);
    currentTerrainHeight = terrainHeight;
    float terrainDiff   = mPosition.y - terrainHeight;
    glm::vec3 normal = terrainMap.getNormalGlobal(mPosition.x, mPosition.z);
    
    float slopeAngle = glm::degrees(acos(glm::dot(normal, glm::vec3(0,1,0))));
    float maxSlopeDeg = 50.0f;
    
    // --- Apply airborne gravity ---
    if (!isGrounded) {
        mVelocity.y += gravityConstant * deltaTime;
    }
    
    // if (isGrounded && slopeAngle > maxSlopeDeg) {
        if (terrainDiff <= terrainSnapTreshhold && slopeAngle > maxSlopeDeg) {
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
            currentAnimationState = AnimationState::Falling;
            animator->PlayAnimation(&animations[currentAnimationState]);
        }
        else if(mVelocity.y <= -2){
            currentAnimationState = AnimationState::Falling;
            animator->PlayAnimation(&animations[currentAnimationState]);
        }
        
        
        meshModel->UpdateModelMatrix();
        animator->UpdateAnimation(deltaTime);
        
}

void Player::GenCircleGL()
{
    glGenVertexArrays(1, &ringVAO); glGenBuffers(1, &ringVBO);
    glBindVertexArray(ringVAO); glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    glBufferData(GL_ARRAY_BUFFER, ringVerts.size()*sizeof(glm::vec3), ringVerts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void*)0); glBindVertexArray(0);

}

void Player::DrawPosCircle(glm::mat4 VP)
{
    std::vector<glm::vec3> ring; buildCircle(ring, 0.5f, 24);    
    glm::mat4 Mring = glm::mat4(1.0f); // identity

    for(auto& v : ring){ v.y = 0.0f; }
    Mring = glm::translate(glm::mat4(1.0f), glm::vec3(mPosition.x, currentTerrainHeight + 0.05f, mPosition.z));
  
    glBindBuffer(GL_ARRAY_BUFFER, ringVBO);
    glBufferData(GL_ARRAY_BUFFER, ring.size()*sizeof(glm::vec3), ring.data(), GL_DYNAMIC_DRAW);

    circleShader->use();
    circleShader->setMat4("uVP", VP);
    circleShader->setMat4("uM", Mring);
    circleShader->setVec4("uColor", glm::vec4(0.0f,0.0f,0.5f,1.0f));
    
    glBindVertexArray(ringVAO);
    glDrawArrays(GL_LINE_LOOP, 0, (GLint)ring.size());
    glBindVertexArray(0);
}


void Player::buildCircle(std::vector<glm::vec3>& out, float radius, int segments)
{
    out.clear(); out.reserve(segments);
    for(int i=0;i<segments;++i){
        float a = (i/(float)segments)*6.2831853f;
        out.emplace_back(radius*cosf(a), 0.0f, radius*sinf(a));
    }
}

    