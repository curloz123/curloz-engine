#define GLM_ENABLE_EXPERIMENTAL
#include "renderer/bone.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include "renderer/assimp_glm_helpers.h"

Bone::Bone(const std::string &name, int ID, const aiNodeAnim *channel): m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
{
    m_NumPositions = channel->mNumPositionKeys;
    for(int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
    {
        aiVector3d aiPosition = channel->mPositionKeys[positionIndex].mValue;
        float timeStamp = channel->mPositionKeys[positionIndex].mTime;
        KeyPosition data;
        data.position = AssimpGLMHelpers::GetGLMVec(aiPosition);
        data.timeStamp = timeStamp;
        m_Positions.push_back(data);
    }

    m_NumRotations = channel->mNumRotationKeys;
    for(int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
    {
        aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
        float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
        KeyRotation data;
        data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
        data.timeStamp = timeStamp;
        m_Rotations.push_back(data);
    }

    m_NumScalings = channel->mNumScalingKeys;
    for(int scalingIndex = 0; scalingIndex < m_NumScalings; ++scalingIndex)
    {
        aiVector3d scale = channel->mScalingKeys[scalingIndex].mValue;
        float timeStamp = channel->mScalingKeys[scalingIndex].mTime;
        KeyScale data;
        data.scale = AssimpGLMHelpers::GetGLMVec(scale);
        data.timeStamp = timeStamp;
        m_Scale.push_back(data);
    }
}

void Bone::update(float animationTime)
{
    glm::mat4 translation = interpolatePosition(animationTime);
    glm::mat4 rotation = interpolateRotation(animationTime);
    glm::mat4 scale = interpolateScaling(animationTime);
    
    m_LocalTransform = translation * rotation * scale;
}

glm::mat4 Bone::getLocalTransform() const
{
    return m_LocalTransform;
}

std::string Bone::getBoneName() const 
{
    return m_Name;
}

int Bone::getBoneID() const 
{
    return m_ID;
}

int Bone::getPositionIndex(float animationTime)
{
    for(int index = 0; index < m_NumPositions - 1; ++index)
    {
        if(animationTime < m_Positions[index+1].timeStamp)
        {
            return index;
        }
    }
    assert(0);
}

int Bone::getRotationIndex(float animationTime)
{
    for(int index = 0; index < m_NumRotations; ++index)
    {
        if(animationTime < m_Rotations[index + 1].timeStamp)
        {
            return index;
        }
    }
    assert(0);
}

int Bone::getScaleIndex(float animationTime)
{
    for(int index = 0; index < m_NumScalings; ++index)
    {
        if(animationTime < m_Scale[index + 1].timeStamp)
        {
            return index;
        }
    }
    assert(0);
}


float Bone::getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
{
    float ScaleFactor = 0.0f;
    float midwayLength = animationTime - lastTimeStamp;
    float framesDiff = nextTimeStamp - lastTimeStamp;
    ScaleFactor = midwayLength / framesDiff;

    return ScaleFactor;
}


glm::mat4 Bone::interpolatePosition(float animationTime)
{
    if(m_NumPositions == 1) 
        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

    int p0Index = getPositionIndex(animationTime);
    int p1Index = p0Index + 1;
    float ScaleFactor = getScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);

    glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, ScaleFactor);

    return glm::translate(glm::mat4(1.0f), finalPosition);
}

glm::mat4 Bone::interpolateRotation(float animationTime)
{
    if(m_NumRotations == 1) 
    {
        auto rotation = glm::normalize(m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = getRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float ScaleFactor = getScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime); 
    glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, ScaleFactor);

    return glm::toMat4(finalRotation);
}

glm::mat4 Bone::interpolateScaling(float animationTime)
{
    if(m_NumScalings == 1)
    {
        return glm::scale(glm::mat4(1.0f), m_Scale[0].scale);
    }

    int p0Index = getScaleIndex(animationTime);
    int p1Index = p0Index + 1;
    float ScaleFactor = getScaleFactor(m_Scale[p0Index].timeStamp, m_Scale[p1Index].timeStamp, animationTime);
    glm::vec3 finalScale = glm::mix(m_Scale[p0Index].scale, m_Scale[p1Index].scale, ScaleFactor);
    
    return glm::scale(glm::mat4(1.0f), finalScale);
}
