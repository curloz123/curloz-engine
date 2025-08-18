#ifndef SHADER_H
#define SHADER_H
#include<string>
#include "glm/glm.hpp"

class Shader
{
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setMat3(const std::string &name, glm::mat3 matrix) const;
    void setMat4(const std::string &name, glm::mat4 matrix) const;
    void setVec3(const std::string &name, glm::vec3 pos) const;
};



#endif

