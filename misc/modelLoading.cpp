#include "glm/detail/qualifier.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/quaternion_transform.hpp"
#include "glm/ext/quaternion_trigonometric.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/trigonometric.hpp"
#include "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <ostream>
#include <string>
#include "glm/glm.hpp"
#include "shaders.h"
#include "stb_image.h"
#include "camera.h"
#include "model.h"


int screenWidth = 1920;
int screenHeight = 1080;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);

    GLFWwindow* window = glfwCreateWindow(screenWidth,screenHeight,"lighting window",NULL,NULL);
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

    Shader shaderObject = Shader("shaders.d/assimpShader.vert","shaders.d/assimpShader.frag");
    stbi_set_flip_vertically_on_load(true);
    stbi_set_flip_vertically_on_load(false);
    Model ourModel("models/schoolGirl/D0208059.obj");

    glm::vec3 pointLightPositions[] =
    {
        glm::vec3( 0.7f,  0.2f,  2.0f),
        glm::vec3( 2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3( 0.0f,  0.0f, -3.0f)
    };
    int nPointLights = sizeof(pointLightPositions)/sizeof(pointLightPositions[0]);



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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        camera.ProcessKeyboard(window,deltaTime);

        printf("Using shader object\n");
        shaderObject.use();


        printf("setting up light\n");
        shaderObject.setFloat("material.shininess",32);

        shaderObject.setVec3("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        shaderObject.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
        shaderObject.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
        shaderObject.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));

        for(int j=0;j<nPointLights;++j)
        {
            shaderObject.setVec3("pointLights["+ std::to_string(j)+ "].position", pointLightPositions[j]);
            shaderObject.setVec3("pointLights["+ std::to_string(j)+ "].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
            shaderObject.setVec3("pointLights["+ std::to_string(j)+ "].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
            shaderObject.setVec3("pointLights["+ std::to_string(j)+ "].specular", glm::vec3(1.0f, 1.0f, 1.0f));
            shaderObject.setFloat("pointLights["+ std::to_string(j)+ "].constant", 1.0f);
            shaderObject.setFloat("pointLights["+ std::to_string(j)+ "].linear", 0.09f);
            shaderObject.setFloat("pointLights["+ std::to_string(j)+ "].quadratic", 0.032f);
        }

        shaderObject.setVec3("spotLight.position", camera.Position);
        shaderObject.setVec3("spotLight.direction", camera.Front);
        shaderObject.setVec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
        shaderObject.setVec3("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        shaderObject.setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        shaderObject.setFloat("spotLight.constant", 1.0f);
        shaderObject.setFloat("spotLight.linear", 0.09f);
        shaderObject.setFloat("spotLight.quadratic", 0.032f);
        shaderObject.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shaderObject.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));



        printf("Setting up view matrix\n");
        glm::mat4 view = camera.GetViewMatrix();
        shaderObject.setMat4("view",view);
        shaderObject.setVec3("viewPos",camera.Position);

        printf("Setting up projection matrix\n");
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(camera.fov),aspectRatio,0.1f,100.0f);
        shaderObject.setMat4("projection",projection);

        printf("Setting up model matrix\n");
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,glm::vec3(0.0f,-1.0f,-3.0f));
        model = glm::scale(model,glm::vec3(0.05f));
        shaderObject.setMat4("model",model);
        ourModel.Draw(shaderObject);


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
