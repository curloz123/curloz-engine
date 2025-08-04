#version 330 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;
uniform vec3 lightPosition;

out vec3 fragPos;
out vec3 normalVector;
out vec3 lightPos;
out vec2 TexCoords;

void main()
{
    gl_Position = projection * view * model * vec4(Position,1.0f);
    fragPos = vec3(model * vec4(Position,1.0f));
    normalVector = mat3(transpose(inverse(model)))*Normal; /*highly performance expensive*/
    lightPos = lightPosition;
    TexCoords = TexCoord;
}
