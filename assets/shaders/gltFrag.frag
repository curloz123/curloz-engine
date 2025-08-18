#version 330 core

out vec4 FragColor;

struct Material 
{
    sampler2D diffuse;
    sampler2D specular;
    float roughness;

    float metallicFactor;
    float roughnessFactor;
}; 

uniform struct Lighting
{
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;
in vec3 normalVector;
in vec2 TexCoords;
in vec3 lightPos;
in vec3 lightDir;
in vec3 viewPos;

uniform Material material;
uniform Lighting lighting;


void main()
{    
    vec3 norm = normalize(normalVector);
    vec3 lightDir = normalize(lighting.position - FragPos); 

    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = lighting.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

    vec3 viewDir = normalize(viewPos - FragPos);
    vec4 mrSample = texture(material.specular, TexCoords);
    float metallic = mrSample.b * material.metallicFactor;
    float roughness = mrSample.g * material.roughnessFactor;
    /* float shininess = mix(256.0f, 8.0f, roughness); */
    float shininess = 128.0f;
    float specStrength = metallic;
    vec3 reflectDir = reflect(-lightDir,norm);
    float spec = pow(max(dot(viewDir, reflectDir),0.0f),shininess);
    vec3 specular = lighting.specular * spec * specStrength;



    vec3 ambient = lighting.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0f);
}


