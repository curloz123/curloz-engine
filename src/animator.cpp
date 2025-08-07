#include "renderer/animator.h"

Animator::Animator(Animation *animation)
{
    m_CurrentTime = 0.0f;
    m_CurrentAnimation = animation;
    m_FinalBoneMatrices.reserve(100);
    for(int i=0; i<100; ++i)
        m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
}

void Animator::updateAnimation(float dt)
{
    m_DeltaTime = dt;
    if(m_CurrentAnimation)
    {
        m_CurrentTime += m_CurrentAnimation->getTicksPerSecond() * dt;
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->getTicksPerSecond());
        calculateBonesTransform(&m_CurrentAnimation->getRootNode(), glm::mat4(1.0f));
    }
}

void Animator::playAnimation(Animation *pAnimation)
{
    m_CurrentAnimation = pAnimation;
    m_CurrentTime = 0.0f;
}

void Animator::calculateBonesTransform(const AssimpNodeData *node, glm::mat4 parentTransform)
{
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    Bone *bone = m_CurrentAnimation->findBone(nodeName);
    if(bone)
    {
        bone->update(m_CurrentTime);     
        nodeTransform = bone->getLocalTransform();
    }

    glm::mat4 globalTransformation = parentTransform * nodeTransform;
    auto BoneInfoMap = m_CurrentAnimation->getBoneInfoMap();
    if(BoneInfoMap.find(nodeName) != BoneInfoMap.end())
    {
        int index = BoneInfoMap[nodeName].id;
        glm::mat4 offset = BoneInfoMap[nodeName].offset;
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    for(int i=0; i<node->childrenCount; ++i)
    {
        calculateBonesTransform(&node->children[i], globalTransformation);
    }
}


