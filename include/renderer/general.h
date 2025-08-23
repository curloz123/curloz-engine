#pragma once 
#include "camera.h"
#include "glm/gtc/matrix_transform.hpp"

struct info 
{
    int width = 1920;
    int height = 1080;
    float aspectRatio = (float)width / (float)height;
    Camera camera;

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width/(float)height, 0.1f, 100.0f);
    glm::mat4 viewMatrix = camera.GetViewMatrix();
    void update()
    {
        viewMatrix = camera.GetViewMatrix();
    }
};
extern info generalInfo;
