#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "renderer/general.h"
#include <glm/glm.hpp>
// #include "renderer/gltLoader.h"
#include "renderer/shaders.h"
#include "renderer/stb_image.h"
#include "renderer/camera.h"
#include "renderer/imageLoader.h"
// #include "physics/particle.h"
// #include "physics/forceGen.h"
#include "renderer/lighting.h"
#include "renderer/entity.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include <iostream>
#include <string>

int screenWidth = generalInfo.width;
int screenHeight = generalInfo.height;

void processInput(GLFWwindow* window);
void processMouse(GLFWwindow *window, Camera &camera);
void ImGuiEvent();


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

    renderCube::bookInAdvance();


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
   
    Shader shaderObject = Shader("assets/shaders/assimpShader.vert","assets/shaders/assimpShader.frag");
    Shader platformShader = Shader("assets/shaders/platform.vert","assets/shaders/platform.frag");
    Shader simpleShader = Shader("assets/shaders/vertexShader.vert", "assets/shaders/fragmentShader.frag");


    DirectionalLight dirLight(glm::vec3(-0.2f, -1.0f, -0.3f), glm::vec3(0.01f), glm::vec3(0.1f), glm::vec3(0.01f));
    glm::vec3 ptLightPosition [3] =
    {
        glm::vec3( 0.0f, 5.0f, 5.0f),
        glm::vec3( 7.0f, 5.0f,-7.0f),
        glm::vec3(-7.0f, 5.0f,-7.0f)
    };
    PointLight ptLight1 = PointLight(ptLightPosition[0],glm::vec3(0.0f, 0.0f, 0.1f), glm::vec3(0.0f,0.0f,0.8f), glm::vec3(0.0f,0.0f,1.0f));
    PointLight ptLight2 = PointLight(ptLightPosition[1],glm::vec3(0.0f, 0.1f, 0.0f), glm::vec3(0.0f,0.8f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
    PointLight ptLight3 = PointLight(ptLightPosition[2],glm::vec3(0.1f,0.0f,0.0f), glm::vec3(0.8f,0.0f,0.0f), glm::vec3(1.0f,0.0f,0.0f));
    Entity::PointLights.push_back(ptLight1);
    Entity::PointLights.push_back(ptLight2);
    Entity::PointLights.push_back(ptLight3);

    Entity::camera = &generalInfo.camera;
    Entity::viewMatrix = generalInfo.viewMatrix;
    Entity::projectionMatrix = generalInfo.projectionMatrix;
    Entity::directionLight = dirLight;

    Entity statue("assets/models/statue-chiriqui-3d-model/scene.gltf", shaderObject);
    Entity mic("assets/models/microphone/microphone_gxl_066_bafhcteks.glb", shaderObject);

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

    
    float deltaTime = 0.0f;
    float timeTakenThisFrame = 0.0f;
    float timeTakenLastFrame = 0.0f;

    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_NORMAL);

    glfwSetWindowUserPointer(window, &generalInfo.camera);
    glm::vec3 lightPos = glm::vec3(0.0f,1.0f,-5.0f);
    float lightPosArr[3] = {lightPos.x, lightPos.y, lightPos.z};
    glm::vec3 modelPos = glm::vec3(0.0f,0.0f,0.0f);
    float modelPosArr[3] = {modelPos.x, modelPos.y, modelPos.z};



     
    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        timeTakenThisFrame = glfwGetTime();
        deltaTime = timeTakenThisFrame - timeTakenLastFrame;
        timeTakenLastFrame = timeTakenThisFrame;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f,0.0f,0.0f, 1.0f);

        processInput(window);
        generalInfo.camera.ProcessKeyboard(window,deltaTime);
        if(generalInfo.camera.shouldMove) processMouse(window, generalInfo.camera);
        generalInfo.update();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        glm::mat4 model = glm::mat4(1.0f);
       
        platformShader.use();
        platformShader.setVec3("lighting.position",glm::vec3(0.0f,25.0f,0.0f));
        platformShader.setVec3("lighting.ambient",glm::vec3(1.0f));
        platformShader.setVec3("lighting.diffuse",glm::vec3(1.0f));
        platformShader.setVec3("lighting.specular",glm::vec3(1.0f));
        platformShader.setFloat("lighting.constant",1);
        platformShader.setFloat("lighting.linear",0.009f);
        platformShader.setFloat("lighting.quadratic",0.0032f);
        platformShader.setVec3("viewPos",generalInfo.camera.Position);

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
        platformShader.setMat4("transform", generalInfo.projectionMatrix * generalInfo.viewMatrix * model);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        if(!generalInfo.camera.shouldMove) 
        {
        for(int i=0; i<Entity::entities.size(); ++i)
        {
            ImGui::PushID(i);
            ImGui::Begin(Entity::entities[i]->name.c_str());

            //setting position
            glm::vec3 objectPosArr = Entity::entities[i]->getPosition();
            ImGui::SliderFloat3("Object Position", &objectPosArr.x, -50.0f, 50.0f);
            Entity::entities[i]->setPosition(objectPosArr);

            //setting rotation
            glm::vec3 objectRotArr = Entity::entities[i]->getRotation();
            ImGui::SliderFloat3("Object Rotation", &objectRotArr.x, -180.0f, 180.0f);
            Entity::entities[i]->setRotation(glm::vec3(1.0f,0.0f,0.0f), objectRotArr.x);
            Entity::entities[i]->setRotation(glm::vec3(0.0f,1.0f,0.0f), objectRotArr.y);
            Entity::entities[i]->setRotation(glm::vec3(0.0f,0.0f,1.0f), objectRotArr.z);

            //setting scale
            float scale = Entity::entities[i]->getScale();
            ImGui::SliderFloat("Object Scale", &scale, 0.0f,100.0f);
            Entity::entities[i]->setScale(scale);
            ImGui::End();
            ImGui::PopID();
        }
        }
    
        for(auto it : Entity::entities)
        {
            it->update(deltaTime);
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
