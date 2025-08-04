#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in ivec4 boneIDs;
layout(location = 4) in vec4 weight;


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;
out vec3 normalVector;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    for(int i=0; i< MAX_BONE_INFLUENCE; ++i)
    {
        if(boneIDs[i] == -1) continue;
        if(boneIDs[i] >= MAX_BONES)
        {
            totalPosition = vec4(pos, 1.0f);
            break;
        }

        vec4 localPosition = finalBonesMatrices[boneIDs[i]] * vec4(pos, 1.0f);
        totalPosition += localPosition * weight[i];
        vec3 localNormal = mat3(finalBonesMatrices[boneIDs[i]]) * norm;
        totalNormal += localNormal;

    }
    TexCoords = tex;
    normalVector = norm;
    gl_Position = projection * view * model * totalPosition;
}
