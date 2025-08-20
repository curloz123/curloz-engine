#version 330 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform mat4 transform;
uniform mat3 normalMatrix;

out vec3 FragPos;
out vec3 normalVector;
out vec2 TexCoords;

void main()
{
    gl_Position = transform * vec4(Position,1.0f);
    FragPos = vec3(model * vec4(Position,1.0f));
    normalVector = normalMatrix * Normal; /*highly performance expensive*/
    TexCoords = TexCoord;
}
