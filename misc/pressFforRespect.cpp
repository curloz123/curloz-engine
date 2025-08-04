#include  "include/glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

void processInput(GLFWwindow* window);
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";


const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
      "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0" ;

const char *vertexShaderSource2 = "#version 330 core\n"

    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";


const char *fragmentShaderSource2 = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
      "FragColor = vec4(0.3f, 0.1f, 0.7f, 1.0f);\n"
    "}\0" ;


int main()
{




  //Initialization
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3); 
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
  glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
  





  //Making a window

  GLFWwindow* window = glfwCreateWindow(800,600,"GLFW window",NULL,NULL);
  if(window == NULL){
    std::cout<<"Could not initialize window";
    glfwTerminate();
    return -1;
  }
  // glViewport(0,0,800,600);
  glfwMakeContextCurrent( window );
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout<<"Failed to initialize glad\n";
    return -1;
  }
  glfwSetWindowSize(window, 800, 600);
  glViewport(0,0,800,600);






  //Compiling Shaders and stuff
  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader,1,&vertexShaderSource,NULL);
  glCompileShader(vertexShader);

  int success ;
  char infoLog[512];
  glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&success);
      
  if(!success)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader,1,&fragmentShaderSource,NULL); 
  glCompileShader(fragmentShader);

  int success2 ;
  char infoLog2[512];
  glGetShaderiv(fragmentShader,GL_COMPILE_STATUS,&success2);
      
  if(!success2)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog2 << std::endl;
  }


  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram,vertexShader);
  glAttachShader(shaderProgram,fragmentShader);
  glLinkProgram(shaderProgram);

  unsigned int vertexShader2;
  vertexShader2 = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader2,1,&vertexShaderSource2,NULL);
  glCompileShader(vertexShader2);

  glGetShaderiv(vertexShader2,GL_COMPILE_STATUS,&success);
      
  if(!success)
  {
    glGetShaderInfoLog(vertexShader2, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
  }

  unsigned int fragmentShader2;
  fragmentShader2 = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader2,1,&fragmentShaderSource2,NULL); 
  glCompileShader(fragmentShader2);

  glGetShaderiv(fragmentShader2,GL_COMPILE_STATUS,&success2);
      
  if(!success2)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog2 << std::endl;
  }


  unsigned int shaderProgram2;
  shaderProgram2 = glCreateProgram();
  glAttachShader(shaderProgram2,vertexShader2);
  glAttachShader(shaderProgram2,fragmentShader2);
  glLinkProgram(shaderProgram2);






  //wokring on our rectangle now
  // float vertices[] = 
  // {
  //    0.5f, 0.5f,0.0f,   //top right
  //    0.5f,-0.5f,0.0f,  //bottom right
  //   -0.5f,-0.5f,0.0f,   //top left
  //   -0.5f,0.5f,0.0f //bottom left
  // };
  float vertices2[] = 
  {
    //The main big Rectangle
     0.08f, 0.8f,0.0f,  //0
     0.08f,-0.8f,0.0f,  //1
     -0.08f,-0.8f,0.0f, //2
    -0.08f,0.8f,0.0f,   //3
    
    //Top extrusion
     0.8f ,0.8f ,0.0f,  //4
     0.8f, 0.6f ,0.0f,  //5
     0.08f,0.6f ,0.0f,  //6

     //bottom extrusion
     0.6f,0.1f,0.0f,   //7
     0.6f,-0.1f,0.0f, //8
     0.08f,0.1f,0.0f, //9
     0.08f,-0.1f,0.0f //10

  };
  unsigned int indices[] =
  {
    0,1,3,
    1,2,3
  };

  unsigned int indices2[] =
  {
    0,4,6,
    4,5,6
  };

  unsigned int  indices3[] = 
  {
    9,7,10,
    7,10,8
  };
  unsigned int VAO,VBO,EBO;
  glGenVertexArrays(1,&VAO);
  glGenBuffers(1,&VBO);
  glGenBuffers(1,&EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER,VBO);
  glBufferData(GL_ARRAY_BUFFER,sizeof(vertices2),vertices2,GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);


  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER,0);

  glBindVertexArray(0);

  unsigned int VAO2,VBO2,EBO2;
  glGenVertexArrays(1,&VAO2);
  glGenBuffers(1,&VBO2);
  glGenBuffers(1,&EBO2);

  glBindVertexArray(VAO2);

  glBindBuffer(GL_ARRAY_BUFFER,VBO2);
  glBufferData(GL_ARRAY_BUFFER,sizeof(vertices2),vertices2,GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO2);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices2),indices2,GL_STATIC_DRAW);
  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
  glEnableVertexAttribArray(0);


  unsigned int VAO3,VBO3,EBO3;
  glGenVertexArrays(1,&VAO3);
  glGenBuffers(1,&VBO3);
  glGenBuffers(1,&EBO3);

  glBindVertexArray(VAO3);

  glBindBuffer(GL_ARRAY_BUFFER,VBO3);
  glBufferData(GL_ARRAY_BUFFER,sizeof(vertices2),vertices2,GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO3);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices3),indices3,GL_STATIC_DRAW);

  glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
  glEnableVertexAttribArray(0);



    
  // glBindVertexArray(VAO);
  // glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); 
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  while(!glfwWindowShouldClose(window))
  {
    processInput(window);

    glClearColor(0.2f,0.3f,0.3f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
  // glDrawArrays(GL_TRIANGLES,0,3);
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);

    glUseProgram(shaderProgram2);
    glBindVertexArray(VAO2);
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
  
    glUseProgram(shaderProgram2);
    glBindVertexArray(VAO3);
    glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
  
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
    // glDeleteShader(vertexShader);
    // glDeleteShader(fragmentShader);

  glfwTerminate();
  return 0;
}


void processInput(GLFWwindow *window)
{
  if(glfwGetKey(window,GLFW_KEY_ESCAPE) == GLFW_PRESS)
  {
    glfwSetWindowShouldClose(window,true);
  }
}
