#ifndef CAMERA_H
#define CAMERA_H
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>


#include "glm/glm.hpp"
enum Camera_Movement
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN,
    STOP
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.3f;
const float ZOOM = 45.0f;

class Camera
{
    public:
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;

        float Yaw;
        float Pitch;

        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;

        bool firstInput = true;
        float lastX;
        float lastY;
        float fov = ZOOM;
        Camera_Movement Direction;

        //Optional Jump Feature

        bool isOnGround = true;
        float jumpVelocity = 2.0f;
        float gravity = 2.0f;

        Camera(
                glm::vec3 position = glm::vec3(0.0f,0.0f,2.0f),
                glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f),
                float yaw = YAW,
                float pitch = PITCH
              ) ;
        Camera(
                float posX, float posY, float posZ,
                float upX, float upY, float upZ,
                float yaw,
                float pitch
              ) ;
        
        glm::mat4 GetViewMatrix();
        
        

        void ProcessKeyboard(GLFWwindow* window, float deltaTime);
        
        
        void ProcessMouseMovement(double xpos, double ypos);
        
        void ProcessMouseScroll(float yoffset);

        void processScrollBack(double yOffset);
        
    private:

        void updateCameraVectors();
        
        

};
#endif


/* Be sure to add following in your main function */

/*  
 y void processInput(GLFWwindow* window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  void mouseCallBack(GLFWwindow* , double xpos , double ypos);
  void scrollback(GLFWwindow* window , double xOffset , double yOffset);
  glfwSetCursorPosCallback(window, mouseCallBack);
  glfwSetScrollCallback(window , scrollback);


  Camera camera;
  glfwSetWindowUserPointer(window,&camera);
*/
