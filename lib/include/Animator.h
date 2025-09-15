#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <glm/glm.hpp>
#include <map>
#include <vector>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include "Animation.h"
#include "Bone.h"
#include <glm/gtx/matrix_decompose.hpp>

struct BlendState {
    Animation* from = nullptr;
    Animation* to = nullptr;
    float duration = 0.2f;   // seconds
    float time = 0.0f;
    bool active = false;
};

class Animator
{
    public:
        BlendState m_Blend;
        Animation* m_CurrentAnimation;
        float m_CurrentTime;

        Animator(Animation* animation)
        {
            m_CurrentAnimation = animation;
            m_CurrentTime = 0.0;

            m_Blend.from = animation;
            m_Blend.to = animation;
            m_Blend.active = false;

            m_FinalBoneMatrices.reserve(100);

            for (int i = 0; i < 100; i++)
                m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
        }

        Animation* GetCurrentAnimation()
        {
            return m_CurrentAnimation;
        }

        void UpdateAnimation(float dt)
        {
            m_DeltaTime = dt;

            if (m_Blend.active)
            {
                m_Blend.time += dt;
                float alpha = glm::clamp(m_Blend.time / m_Blend.duration, 0.0f, 1.0f);

                // During blend, "from" uses its current time,
                CalculateBlendedBoneTransform(&m_Blend.from->GetRootNode(),&m_Blend.to->GetRootNode(),glm::mat4(1.0f), alpha);

                if (alpha >= 1.0f)
                {
                    m_Blend.active = false;
                    m_CurrentAnimation = m_Blend.to;
                    m_CurrentTime = 0.0f; // start playing it forward
                }
            }
            else if (m_CurrentAnimation)
            {
                // advance normally
                m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());

                CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
            }
        }

        void PlayAnimation(Animation* pAnimation, float blendDuration = 0.15f)
        {

            if (m_CurrentAnimation != pAnimation)
            {
                m_Blend.from = m_CurrentAnimation;
                m_Blend.to = pAnimation;
                m_Blend.duration = blendDuration;
                m_Blend.time = 0.0f;
                m_Blend.active = true;
                
                // Optionally, still switch the current animation for time tracking
                m_CurrentAnimation = pAnimation;
                m_CurrentTime = 0.0f;
                CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
            }
          
        }

        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
        {
            std::string nodeName = node->name;
            glm::mat4 nodeTransform = node->transformation;

            Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

            if (Bone)
            {
                Bone->Update(m_CurrentTime);
                nodeTransform = Bone->GetLocalTransform();
            }

            glm::mat4 globalTransformation = parentTransform * nodeTransform;

            auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end())
            {
                int index = boneInfoMap[nodeName].id;
                glm::mat4 offset = boneInfoMap[nodeName].offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }

            for (int i = 0; i < node->childrenCount; i++)
                CalculateBoneTransform(&node->children[i], globalTransformation);
        }

        void CalculateBlendedBoneTransform(const AssimpNodeData* fromNode,const AssimpNodeData* toNode,glm::mat4 parentTransform,float alpha)
        {
            std::string nodeName = fromNode->name;

            // Start with default transforms
            glm::mat4 fromTransform = fromNode->transformation;
            glm::mat4 toTransform   = toNode->transformation;

            // Get bone data from "from" animation
            Bone* fromBone = m_Blend.from ? m_Blend.from->FindBone(nodeName) : nullptr;
            if (fromBone)
            {
                fromBone->Update(m_CurrentTime); // use current time of the old animation
                fromTransform = fromBone->GetLocalTransform();
            }

            // Get bone data from "to" animation
            Bone* toBone = m_Blend.to ? m_Blend.to->FindBone(nodeName) : nullptr;
            if (toBone)
            {
                // 🚀 Freeze at time=0.0f during blend
                toBone->Update(0.0f);
                toTransform = toBone->GetLocalTransform();
            }

            // Decompose both transforms
            // NOTE: Decompose is we take a part the matrix and do calculations component wise so we can actually interpolate 
            glm::vec3 fromPos, toPos, fromScale, toScale;
            glm::quat fromRot, toRot;
            DecomposeTransform(fromTransform, fromPos, fromRot, fromScale);
            DecomposeTransform(toTransform, toPos, toRot, toScale);

            // Blend them
            glm::vec3 pos = glm::mix(fromPos, toPos, alpha);
            glm::quat rot = glm::slerp(fromRot, toRot, alpha);
            glm::vec3 scale = glm::mix(fromScale, toScale, alpha);

            // Rebuild local transform
            glm::mat4 localTransform =
                glm::translate(glm::mat4(1.0f), pos) *
                glm::toMat4(rot) *
                glm::scale(glm::mat4(1.0f), scale);

            // Global transform
            glm::mat4 globalTransformation = parentTransform * localTransform;

            // Write into final bone matrix if this node is a bone
            auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end())
            {
                int index = boneInfoMap[nodeName].id;
                glm::mat4 offset = boneInfoMap[nodeName].offset;
                m_FinalBoneMatrices[index] = globalTransformation * offset;
            }

            // Recurse through children
            for (int i = 0; i < fromNode->childrenCount; i++)
            {
                // Both nodes should have the same hierarchy structure
                CalculateBlendedBoneTransform(&fromNode->children[i],
                                            &toNode->children[i],
                                            globalTransformation,
                                            alpha);
            }
        }


        void DecomposeTransform(const glm::mat4& transform, glm::vec3& outPos, glm::quat& outRot, glm::vec3& outScale)
        {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transform, outScale, outRot, outPos, skew, perspective);
            outRot = glm::normalize(outRot);
        }

        std::vector<glm::mat4> GetFinalBoneMatrices()
        {
            return m_FinalBoneMatrices;
        }

        glm::vec3 GetBoneGlobalPosition(const std::string& boneName)
        {
            auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
            if (!m_CurrentAnimation) return glm::vec3(-1.0f);

            glm::mat4 globalTransform = GetBoneGlobalTransform(boneName);
            return glm::vec3(globalTransform[3]);
        }

        glm::mat4 GetBoneGlobalTransform(const std::string& boneName)
        {
            glm::mat4 result = glm::mat4(1.0f);
            CalculateBoneGlobalTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f), boneName, result);
            return result;
        }

        void CalculateBoneGlobalTransform(const AssimpNodeData* node, glm::mat4 parentTransform, 
                                        const std::string& targetBone, glm::mat4& outTransform)
        {
            glm::mat4 nodeTransform = node->transformation;

            Bone* bone = m_CurrentAnimation->FindBone(node->name);
            if (bone)
            {
                bone->Update(m_CurrentTime);
                nodeTransform = bone->GetLocalTransform();
            }

            glm::mat4 globalTransform = parentTransform * nodeTransform;

            if (node->name == targetBone)
                outTransform = globalTransform;

            for (int i = 0; i < node->childrenCount; i++)
                CalculateBoneGlobalTransform(&node->children[i], globalTransform, targetBone, outTransform);
        }

    private:
        std::vector<glm::mat4> m_FinalBoneMatrices;        
        float m_DeltaTime;

};
#endif