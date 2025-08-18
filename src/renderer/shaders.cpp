#include "renderer/shaders.h"
#include "glad/glad.h"
#include <fstream>
#include <sstream>
#include<iostream>
#include <string>
#include "glm/gtc//type_ptr.hpp"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    std::cout<<"vertex path "<<vertexPath<<"fragment path "<<fragmentPath<<std::endl;
    try
    {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream,fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // 2. compile shaders
    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    std::cout<<"Vertex Shader status: "<<success<<std::endl;
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };


    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    std::cout<<"fragment shader status: "<<success<<std::endl;
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    };


    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    // print linking errors if any
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    std::cout<<"Linking status: "<<success<<std::endl;
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }


    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}


void Shader::use()
{
    glUseProgram(ID);
    // std::cout<<"Using Shader\n";
    
}

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void Shader::setFloat(const std::string &name, float value) const
{ 
    int vertexColorLocation = glGetUniformLocation(ID,name.c_str());
    glUniform1f(vertexColorLocation, value); 
} 
void Shader::setMat3(const std::string &name, glm::mat3 matrix) const
{
    unsigned int nameLoc = glGetUniformLocation(ID,name.c_str());
    glUniformMatrix3fv(nameLoc,1,GL_FALSE,glm::value_ptr(matrix));
}
void Shader::setMat4(const std::string &name, glm::mat4 matrix) const
{
    unsigned int nameLoc = glGetUniformLocation(ID,name.c_str());
    glUniformMatrix4fv(nameLoc,1,GL_FALSE,glm::value_ptr(matrix));
}
void Shader::setVec3(const std::string &name, glm::vec3 pos) const
{
    float x = pos.x, y=pos.y , z = pos.z;
    int location = glGetUniformLocation(ID,name.c_str());
    glUniform3f(location,x,y,z);
}
