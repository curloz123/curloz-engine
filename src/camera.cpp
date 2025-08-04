#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "renderer/camera.h"




Camera::Camera(
                glm::vec3 position ,
                glm::vec3 up ,
                float yaw ,
                float pitch                 
              ) :
                Front(glm::vec3(0.0f,0.0f,-1.0f)),
                MovementSpeed(SPEED),
                MouseSensitivity(SENSITIVITY),
                Zoom(ZOOM)
        {
            Position = position;
            WorldUp = up;
            Yaw = yaw;
            Pitch = pitch;
            updateCameraVectors();
        }
Camera::Camera(
                float posX, float posY, float posZ,
                float upX, float upY, float upZ,
                float yaw,
                float pitch
               ) : 
                Front(glm::vec3(0.0f, 0.0f, -1.0f)), 
                MovementSpeed(SPEED), 
                MouseSensitivity(SENSITIVITY), 
                Zoom(ZOOM)
        {
             Position = glm::vec3(posX, posY, posZ);
             WorldUp = glm::vec3(upX, upY, upZ);
             Yaw = yaw;
             Pitch = pitch;
             updateCameraVectors();
        }

        
glm::mat4 Camera::GetViewMatrix()
        {
            return glm::lookAt(Position,Position + Front , Up);
        }

void Camera::ProcessKeyboard(GLFWwindow* window, float deltaTime)
        {
            // Direction = Camera_Movement::STOP;
            float velocity = MovementSpeed * deltaTime;
            if(glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS) Position += Front * velocity;
            if(glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS) Position -= Front * velocity;
            if(glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS) Position -= Right * velocity;
            if(glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS) Position += Right * velocity;
            if(glfwGetKey(window,GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) Position -= WorldUp * velocity;
            if(glfwGetKey(window,GLFW_KEY_SPACE) == GLFW_PRESS) Position += WorldUp * velocity;

            // if(Position.y <= 0.8f || Position.y >= 0.8f)
            // {
            //     Position.y = 0.8f;
            // }
          //  else {camera.Direction = Camera_Movement::STOP;}

            // if(Direction == Camera_Movement::FORWARD) Position += Front * velocity;
            // if(Direction == Camera_Movement::BACKWARD) Position -= Front * velocity;
            // if(Direction == Camera_Movement::RIGHT) Position += Right * velocity;
            // if(Direction == Camera_Movement::LEFT) Position -= Right * velocity;
            // if(Direction == Camera_Movement::UP) Position += WorldUp * velocity;
            // if(Direction == Camera_Movement::DOWN) Position -= WorldUp * velocity;
        }

        
void Camera::ProcessMouseMovement(double xpos, double ypos)
        {
            float xPos = static_cast<float>(xpos);
            float yPos = static_cast<float>(ypos);
 
            if(firstInput)
            {
              lastX = xPos;
              lastY = yPos;
              firstInput = false;
            }

         
            float xoffset = xPos - lastX;
            float yoffset = lastY - yPos;
 
            lastX = xPos;
            lastY = yPos;

            xoffset *= MouseSensitivity;
            yoffset *= MouseSensitivity;

            Yaw += xoffset;
            Pitch += yoffset;

            if(Pitch >= 60.0f) Pitch = 60.0f;
            if(Pitch <= -60.0f) Pitch = -60.0f;
            
            updateCameraVectors();
        }

void Camera::ProcessMouseScroll(float yoffset)
        {
            Zoom -= yoffset;
            if(Zoom <= 1.0f) Zoom = 1;
            if(Zoom >= 89.0f) Zoom = 89.0f;
        }

void Camera::updateCameraVectors()
  
        {
            glm::vec3 front;
            front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
            front.y = sin(glm::radians(Pitch));
            front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));

            Front = glm::normalize(front);
            // printf("Camera.Front detes are: %f,%f,%f\n",Front.x,Front.y,Front.z);

            Right = glm::normalize(glm::cross(Front,WorldUp));
            Up = glm::normalize(glm::cross(Right,Front));
        }

void Camera::processScrollBack(double yOffset)
        {
          fov -= yOffset;
          if(fov <= 1) fov = 1;
          if(fov >= 90) fov = 90;
        }




/* Be sure to add following in your main function */

/*  
  void processInput(GLFWwindow* window);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  void mouseCallBack(GLFWwindow* , double xpos , double ypos);
  void scrollback(GLFWwindow* window , double xOffset , double yOffset);
  glfwSetCursorPosCallback(window, mouseCallBack);
  glfwSetScrollCallback(window , scrollback);


  Camera camera;
  glfwSetWindowUserPointer(window,&camera);
*/
