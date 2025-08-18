// #include "glm/common.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "physics/core.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "renderer/gltLoader.h"
#include "renderer/shaders.h"
#include "renderer/stb_image.h"
#include "renderer/camera.h"
#include "renderer/gltLoader.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "renderer/imageLoader.h"
#include "physics/particle.h"
// #include "physics/forceGen.h"
#include <imgui/imgui_impl_opengl3.h>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

int screenWidth = 1920;
int screenHeight = 1080;

void processInput(GLFWwindow* window);
void processMouse(GLFWwindow *window, Camera &camera);
void ImGuiEvent();

struct model
{
    GLTloader model;
    physics::Vector3 position;
    physics::Vector3 velocity;
    Particle particle;
};


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
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    //Working in ImGUI stuff
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard;  

    ImGui::StyleColorsDark();

    if(!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
        std::cout<<"Could not initialize ImplGlfw_InitForOpenGL"<<std::endl;
        return -1;
    }
    if(!ImGui_ImplOpenGL3_Init("#version 330"))
    {
        std::cout<<"Could not initialize ImplOpenGL3_Init"<<std::endl;
        return -1;
    }
   
    Shader shaderObject = Shader("assets/shaders/shaderObject.vert","assets/shaders/shaderObject.frag");
    Shader platformShader = Shader("assets/shaders/platform.vert","assets/shaders/platform.frag");
    Shader simpleShader = Shader("assets/shaders/vertexShader.vert", "assets/shaders/fragmentShader.frag");

    float verticesPlatform[] = 
    {
        -1.0f,0.0f,-1.0f, 0.0f,1.0f,0.0f, 0.0f,0.0f,
        -1.0f,0.0f, 1.0f, 0.0f,1.0f,0.0f, 0.0f,100.0f,
         1.0f,0.0f, 1.0f, 0.0f,1.0f,0.0f, 100.0f,100.0f,
         1.0f,0.0f,-1.0f, 0.0f,1.0f,0.0f, 100.0f,0.0f
    };
    unsigned int indicesPlatform [] = 
    {
        0,1,2,
        2,3,0
    };

    unsigned int VAO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    unsigned int VBO;
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verticesPlatform),verticesPlatform,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    unsigned int EBO;
    glGenBuffers(1,&EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indicesPlatform),indicesPlatform,GL_STATIC_DRAW);

    
    platformShader.use();
    GLuint groundDiffuse = loadTextureFromFile("assets/textures/sexyAhhGround/GroundDirtWeedsPatchy004_COL_1K.jpg",false);
    platformShader.setInt("material.diffuse0",0);
    GLuint groundSpecular = loadTextureFromFile("assets/textures/sexyAhhGround/GroundDirtWeedsPatchy004_COL_1K.jpg",false);
    platformShader.setInt("material.specular0",1);




    float verticesBox[] = {
        // positions         
        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f, 
        -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 

        -0.5f, -0.5f,  0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 
        -0.5f, -0.5f,  0.5f, 

        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 
        
         0.5f,  0.5f,  0.5f, 
         0.5f,  0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 

        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f, -0.5f, 

        -0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f, 
    };

    unsigned int VAO2;
    glGenVertexArrays(1,&VAO2);
    glBindVertexArray(VAO2);

    unsigned int VBO2;
    glGenBuffers(1,&VBO2);
    glBindBuffer(GL_ARRAY_BUFFER,VBO2);
    glBufferData(GL_ARRAY_BUFFER,sizeof(verticesBox),verticesBox,GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);

    
    float deltaTime = 0.0f;
    float timeTakenThisFrame = 0.0f;
    float timeTakenLastFrame = 0.0f;

    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);

    Camera camera;
    glfwSetWindowUserPointer(window, &camera);
    glm::vec3 lightPos = glm::vec3(0.0f,1.0f,-5.0f);
    float lightPosArr[3] = {lightPos.x, lightPos.y, lightPos.z};
    glm::vec3 modelPos = glm::vec3(0.0f,0.0f,0.0f);
    float modelPosArr[3] = {modelPos.x, modelPos.y, modelPos.z};

    std::vector<model> models; 
     
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int pixelWidth,pixelHeight;
        glfwGetFramebufferSize(window,&pixelWidth,&pixelHeight);
        float aspectRatio = (float)pixelWidth/(float)pixelHeight;

        timeTakenThisFrame = glfwGetTime();
        deltaTime = timeTakenThisFrame - timeTakenLastFrame;
        timeTakenLastFrame = timeTakenThisFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f,0.0f,0.0f, 1.0f);

        processInput(window);
        camera.ProcessKeyboard(window,deltaTime);
        if(camera.shouldMove) processMouse(window, camera);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.fov), aspectRatio, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

       
        platformShader.use();

        platformShader.setVec3("lighting.position",lightPos);
        platformShader.setVec3("lighting.ambient",glm::vec3(0.2f));
        platformShader.setVec3("lighting.diffuse",glm::vec3(0.8f,0.8f,0.8f));
        platformShader.setVec3("lighting.specular",glm::vec3(1.0f,1.0f,1.0f));
        platformShader.setFloat("lighting.constant",1);
        platformShader.setFloat("lighting.linear",0.009f);
        platformShader.setFloat("lighting.quadratic",0.0032f);
        platformShader.setVec3("viewPos",camera.Position);
        // platformShader.setFloat("material.shininess",256);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, groundDiffuse);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, groundSpecular);
        platformShader.setInt("material.diffuse0",0);
        platformShader.setInt("material.specular0",1);


        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(100.0f));
        platformShader.setMat4("model", model);
        platformShader.setMat4("transform", projection*view*model);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);



        simpleShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.01f));

        ImGui::Begin("Sliders");
        ImGui::SliderFloat3("Light Position", lightPosArr, -10.0f, 10.0f);
        lightPos = glm::vec3(lightPosArr[0],lightPosArr[1],lightPosArr[2]);
        ImGui::End();
        glm::mat4 transform = projection * view * model;
        simpleShader.setMat4("transform",transform);
        glBindVertexArray(VAO2);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        for(auto &it : models)
        {
            
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    Camera* camera = static_cast<Camera*>(glfwGetWindowUserPointer(window));
    if(glfwGetKey(window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window,true);
    }
    if(glfwGetKey(window,GLFW_KEY_TAB) == GLFW_PRESS && camera->shouldMove)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        camera->shouldMove = false;
    }
    if(glfwGetKey(window,GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window,GLFW_KEY_G) == GLFW_PRESS && !camera->shouldMove)
    {
        camera->firstInput = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        camera->shouldMove = true;
    }

}

void processMouse(GLFWwindow *window, Camera &camera)
{
    double MOUSE_POS_X, MOUSE_POS_Y;  
    glfwGetCursorPos(window, &MOUSE_POS_X, &MOUSE_POS_Y);
    camera.ProcessMouseMovement(MOUSE_POS_X, MOUSE_POS_Y);
}
