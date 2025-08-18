#version 330 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
}; 


in vec3 FragPos;
in vec3 normalVector;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;

// function prototypes

void main()
{    
    vec3 result = vec3(texture(material.diffuse,TexCoords)); 
    
    FragColor = vec4(0.2f,0.5f,0.1f, 1.0f);
}


