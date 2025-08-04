#include "renderer/imageLoader.h"

GLuint loadTextureFromFile(const std::string &path, bool flip)
{
    int imageWidth = 0;
    int imageHeight= 0;
    int nChannels = 0;
    unsigned int texture;
    stbi_set_flip_vertically_on_load(flip);
    glGenTextures(1,&texture);
    glBindTexture(GL_TEXTURE_2D,texture);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    unsigned char *textureData = stbi_load(path.c_str(),&imageWidth,&imageHeight,&nChannels,0);

    GLenum format;
    if(nChannels == 1) format = GL_RED;
    if(nChannels == 3) format = GL_RGB;
    if(nChannels == 4) format = GL_RGBA;
    if(textureData)
    {
        glTexImage2D(GL_TEXTURE_2D,0,format,imageWidth,imageHeight,0,format,GL_UNSIGNED_BYTE,textureData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout<<"Could not load texture at path: "<<path<<std::endl;
        return -1;
    }
    stbi_image_free(textureData);
    return texture;

}
