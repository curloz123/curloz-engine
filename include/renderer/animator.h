#pragma once
#include "animation.h"

class Animator
{
    private:
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Animation *m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;

    public:
        Animator(Animation *animation);
        void updateAnimation(float dt);
        void playAnimation(Animation *animation);
        void calculateBonesTransform(const AssimpNodeData *node, glm::mat4 parentTransform);

        inline std::vector<glm::mat4> getFinalBonesMatrices(){ return m_FinalBoneMatrices; }
};
