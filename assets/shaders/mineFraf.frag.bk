#version 330 core
out vec4 FragColor;
in vec2 TexCoord;
in float Side;

uniform sampler2D texture1;
uniform sampler2D texture2;
uniform sampler2D texture3;

void main()
{ 
    if(Side == 0)
    {
        FragColor = texture(texture1,TexCoord);
    }
    if(Side == 1)
    {
        FragColor = texture(texture2,TexCoord);
    }
    if(Side == -1)
    {
        FragColor = texture(texture3,TexCoord);
    }

}
