#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> 
#include <assimp/anim.h>
#include <vector>
#include <string>

struct KeyPosition
{
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation
{
    glm::quat orientation;
    float timeStamp;
};

struct KeyScale
{
    glm::vec3 scale;
    float timeStamp;
};

class Bone
{
    private:
        std::vector<KeyPosition> m_Positions;
        std::vector<KeyRotation> m_Rotations;
        std::vector<KeyScale> m_Scale;
        int m_NumPositions;
        int m_NumRotations;
        int m_NumScalings;

        std::string m_Name;
        int m_ID;
        glm::mat4 m_LocalTransform;

    public:
        Bone(const std::string &name, int ID, const aiNodeAnim* channel);
        void update(float animationTime);
        glm::mat4 getLocalTransform() const;
        std::string getBoneName() const;
        int getBoneID() const;
        int getPositionIndex(float animationTime);
        int getRotationIndex(float animationTime);
        int getScaleIndex(float animationTime);

    private:
        float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
        glm::mat4 interpolatePosition(float animationTime);
        glm::mat4 interpolateRotation(float animationTime);
        glm::mat4 interpolateScaling(float animationTime);
};
