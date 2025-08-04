#version 330 core

out vec4 FragColor;

struct Material {
    sampler2D diffuse;
    /* sampler2D specular; */
    /* float shininess; */
}; 



in vec3 FragPos;
in vec3 normalVector;
in vec2 TexCoords;

uniform Material material;


void main()
{    
       
    FragColor = texture(material.diffuse,TexCoords);
}


