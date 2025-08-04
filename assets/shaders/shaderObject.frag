#version 330 core

uniform struct Material
{
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};

uniform struct Lighting
{
    vec3 position;
    vec3 direction;
    float cutoff;
    float outerCutoff;

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
uniform vec3 lightColor;
uniform vec3 viewPos;
uniform float sinComponent;
uniform float getTime;
out vec4 fragColor;
uniform sampler2D tex;

void main()
{
    float ambientStrength = 0.1f;
    vec3 ambient = lighting.ambient * vec3(texture(material.diffuse, TexCoords));
   
    vec3 norm = normalize(normalVector);
    vec3 lightDir = normalize(lighting.position - fragPos); 


    float diff = max(dot(norm,lightDir),0.0);
    vec3 diffuse = lighting.diffuse * diff * vec3(texture(material.diffuse, TexCoords));

    float specularStrength = 1.0f;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDir,norm);
    float spec = pow(max(dot(viewDir,reflectDir),0.0) , material.shininess);
    vec3 specular = lighting.specular * spec * vec3(texture(material.specular,TexCoords) ) ;

    vec3 emission;
    if( texture(material.specular,TexCoords).x  == 0) 
    {
        emission = vec3(texture(material.emission,TexCoords + getTime*0.1)) * sinComponent;
    }
    else 
    {
        emission = vec3(0.0f,0.0f,0.0f);
    }

    float distance = length(fragPos - lighting.position);
    float attenuation = 1/(lighting.constant + lighting.linear*distance + lighting.quadratic*(distance*distance));
    
    float theta = dot(lightDir , -normalize(lighting.direction));
    float epsilon = lighting.cutoff -lighting.outerCutoff;
    float intensity = clamp((theta - lighting.outerCutoff)/epsilon,0.0,1.0f);
    vec3 result;

    specular *= intensity;
    diffuse *= intensity;
    /* if(theta > lighting.cutoff) */
    {
        result = ( ambient + specular + diffuse) * attenuation ;
    }
    /* else */
    /* { */
    /*     result = ambient ; */
    /* } */

    fragColor = vec4(result,1.0f);
}
