#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include "glm/glm.hpp"
#include "shaders.h"
#include "stb_image.h"
#include "camera.h"

int screenWidth = 1920;
int screenHeight = 1080;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);

    GLFWwindow* window = glfwCreateWindow(screenHeight,screenWidth,"minecraft",NULL,NULL);
    if(window == NULL)
    {
        std::cout<<"GLFW window could not be created";
    }

    glfwMakeContextCurrent(window);
    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout<<"Could not initialize glad\n";
        glfwTerminate();
        return -1;
    }
    glfwSetWindowSize(window,1920,1080);
    glViewport(0,0,1920,1080);
    glEnable(GL_DEPTH_TEST); 

    Shader shader1("/home/curloz/dev/project/opengl/shaders.d/mineVert.vert",
                    "/home/curloz/dev/project/opengl/shaders.d/mineFraf.frag");
    float vertices[] = {
        -0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.0f,
         0.5f,-0.5f,-0.5f, 1.0f,0.0f, 0.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f, 0.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f, 0.0f,
    -0.5f, 0.5f,-0.5f, 0.0f,1.0f, 0.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.0f,

    -0.5f,-0.5f, 0.5f, 0.0f,0.0f, 0.0f,
     0.5f,-0.5f, 0.5f, 1.0f,0.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 0.0f,1.0f, 0.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,0.0f, 0.0f,

    -0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.0f,
    -0.5f, 0.5f,-0.5f, 0.0f,1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.0f,
    -0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.0f,
    -0.5f,-0.5f, 0.5f, 1.0f,0.0f, 0.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.0f,

     0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.0f,
     0.5f, 0.5f,-0.5f, 0.0f,1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.0f,
     0.5f, 0.5f, 0.5f, 1.0f,1.0f, 0.0f,
     0.5f,-0.5f, 0.5f, 1.0f,0.0f, 0.0f,
     0.5f,-0.5f,-0.5f, 0.0f,0.0f, 0.0f,

    -0.5f,-0.5f,-0.5f, 0.0f,1.0f, -1.0f,
     0.5f,-0.5f,-0.5f, 1.0f,1.0f, -1.0f,
     0.5f,-0.5f, 0.5f, 1.0f,0.0f, -1.0f,
     0.5f,-0.5f, 0.5f, 1.0f,0.0f, -1.0f,
    -0.5f,-0.5f, 0.5f, 0.0f,0.0f, -1.0f,
    -0.5f,-0.5f,-0.5f, 0.0f,1.0f, -1.0f,

    // -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,1.0f,
    //  0.5f,  0.5f, -0.5f,  1.0f, 0.0f,1.0f,
    //  0.5f,  0.5f,  0.5f,  1.0f, 1.0f,1.0f,
    //  0.5f,  0.5f,  0.5f,  1.0f, 1.0f,1.0f,
    // -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,1.0f,
    // -0.5f,  0.5f, -0.5f,  0.0f, 0.0f,1.0f 

    -0.5f, 0.5f, 0.5f, 0.0f,0.0f, 1.0f,
    -0.5f, 0.5f,-0.5f, 0.0f,1.0f, 1.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f, 1.0f,
     0.5f, 0.5f,-0.5f, 1.0f,1.0f, 1.0f,
     0.5f, 0.5f, 0.5f, 1.0f,0.0f, 1.0f,
    -0.5f, 0.5f, 0.5f, 0.0f,0.0f, 1.0f

};

int imageWidth = 0;
int imageHeight= 0;
int nChannels = 0;
unsigned int texture1;
stbi_set_flip_vertically_on_load(true);
glGenTextures(1,&texture1);
glBindTexture(GL_TEXTURE_2D,texture1);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

unsigned char *textureData = stbi_load("textures.d/blockSide.jpg",&imageWidth,&imageHeight,&nChannels,0);

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
    std::cout<<"Could not load side texture\n";
    return -1;
}
shader1.use();
shader1.setInt("texture1",0);
std::cout<<"Generated Texture for side part\n";
shader1.use();
shader1.setInt("texture1",0);
stbi_image_free(textureData);

unsigned int texture2;
glGenTextures(1,&texture2);
glBindTexture(GL_TEXTURE_2D,texture2);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

textureData = stbi_load("textures.d/blockTop.png",&imageWidth,&imageHeight,&nChannels,0);

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
    std::cout<<"Could not load top texture\n";
    return -1;
}
std::cout<<"Generated Texture for top part\n";
shader1.use();
shader1.setInt("texture2",1);
stbi_image_free(textureData);

unsigned int texture3;
glGenTextures(1,&texture3);
glBindTexture(GL_TEXTURE_2D,texture3);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

textureData = stbi_load("textures.d/blockBottom.jpg",&imageWidth,&imageHeight,&nChannels,0);

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
    std::cout<<"Could not load bottom texture\n";
    return -1;
}
std::cout<<"Generated Texture for bottom part\n";
shader1.use();
shader1.setInt("texture3",2);

stbi_image_free(textureData);





unsigned int VAO;
glGenVertexArrays(1,&VAO);
glBindVertexArray(VAO);

unsigned int VBO;
glGenBuffers(1,&VBO);
glBindBuffer(GL_ARRAY_BUFFER,VBO);
glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
glEnableVertexAttribArray(0);
glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
glEnableVertexAttribArray(1);
glVertexAttribPointer(2,1,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(5*sizeof(float)));
glEnableVertexAttribArray(2);


glm::vec3 blockPositions[10000]; ;
int currentBlockPosition = 0;
for(int i=0;i<100;++i)
{
    for(int k=0;k<100;++k)
        {
           blockPositions[currentBlockPosition] = glm::vec3((float)i,0.0f,(float)k); 
           ++currentBlockPosition;
        }
    }

    float deltaTime = 0.0f;
    float timeTakenThisFrame = 0.0f;
    float timeTakenLastFrame = 0.0f;

    void processInput(GLFWwindow* window);
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);

    void mouseCallBack(GLFWwindow* , double xpos , double ypos);
    void scrollCallBack(GLFWwindow*, double xOffset , double yOffset);
    glfwSetCursorPosCallback(window, mouseCallBack);
    glfwSetScrollCallback(window, scrollCallBack);

    Camera camera;
    glfwSetWindowUserPointer(window,&camera);

    while(!glfwWindowShouldClose(window))
    {
        int pixelWidth,pixelHeight;
        glfwGetFramebufferSize(window,&pixelWidth,&pixelHeight);
        float aspectRatio = (float)pixelWidth/(float)pixelHeight;
        timeTakenThisFrame = glfwGetTime();
        deltaTime = timeTakenThisFrame - timeTakenLastFrame;
        timeTakenLastFrame = timeTakenThisFrame;

        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glClearColor(0.5f, 0.8f, 0.9f, 1.0f);
        camera.ProcessKeyboard(window,deltaTime);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D,texture2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D,texture3);

        shader1.use();
        
        glBindVertexArray(VAO);
        
        glm::mat4 view = camera.GetViewMatrix();
        unsigned int viewLoc = glGetUniformLocation(shader1.ID,"view");
        glUniformMatrix4fv(viewLoc,1,GL_FALSE,glm::value_ptr(view));

        glm::mat4 model = glm::mat4(1.0f); 
        for(int i=0;i<10000;++i)
        {
           model = glm::mat4(1.0f);
           model = glm::translate(model,blockPositions[i]); 

           unsigned int modelLoc = glGetUniformLocation(shader1.ID,"model");
           glUniformMatrix4fv(modelLoc,1,GL_FALSE,glm::value_ptr(model));

           glDrawArrays(GL_TRIANGLES,0,36);
        }
        

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.fov),aspectRatio,0.1f,100.0f);
        unsigned int projectionLoc = glGetUniformLocation(shader1.ID,"projection");
        glUniformMatrix4fv(projectionLoc,1,GL_FALSE,glm::value_ptr(projection));
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

}

void processInput(GLFWwindow* window)
{
    if(glfwGetKey(window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window,true);
    }
}

void scrollCallBack(GLFWwindow* window, double xOffset, double yOffset)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->processScrollBack(yOffset);
}
void mouseCallBack(GLFWwindow* window , double xpos , double ypos)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->ProcessMouseMovement(xpos , ypos);
}
