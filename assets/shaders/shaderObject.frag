#version 330 core

uniform struct Material
{
    sampler2D texture_diffuse0;
    sampler2D texture_diffuse1;
    sampler2D texture_diffuse2;
    sampler2D texture_diffuse3;
    sampler2D texture_specular0;
    sampler2D texture_specular1;
    sampler2D texture_specular2;
    sampler2D texture_specular3;
    float metallicFactor;
    float roughnessFactor;
    float shininess ;
};

uniform struct Lighting
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec3 normalVector;
in vec3 fragPos;
in vec3 lightPos;
in vec2 TexCoords;
uniform Material material;
uniform Lighting lighting;
uniform vec3 objectColor;
uniform vec3 viewPos;
uniform float sinComponent;
uniform float getTime;
out vec4 fragColor;
uniform sampler2D tex;

void main()
{
    vec3 ambient = lighting.ambient * vec3(texture(material.texture_diffuse0, TexCoords));
   
    vec3 norm = normalize(normalVector);
    vec3 lightDir = normalize(lighting.position - fragPos); 


    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = lighting.diffuse * diff * vec3(texture(material.texture_diffuse0, TexCoords));
    float specularStrength = 1.0f;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir,norm);
    float spec = pow(max(dot(viewDir,reflectDir),0.0) , material.shininess);
    /* float shininess = mix(256.0f, 8.0f, roughness); */
    vec3 specular = lighting.specular * spec * vec3(texture(material.texture_specular0,TexCoords) ) ;


    float distance = length(fragPos - lighting.position);
    float attenuation = 1/(lighting.constant + lighting.linear*distance + lighting.quadratic*(distance*distance));
    
    vec3 result = (ambient + diffuse + specular) * attenuation ;

    fragColor = vec4(result, 1.0f);
}
