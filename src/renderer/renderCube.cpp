#include "renderer/renderCube.h"




generalAttributes renderCube::attributes;




void renderCube::bookInAdvance()
{        
    attributes.setup();
}

renderCube::renderCube(const float &scale, glm::vec3 &color, glm::vec3 &position, Shader &originShader)
{
    projectionMatrix = generalInfo.projectionMatrix;
    viewMatrix = generalInfo.viewMatrix;
    attributes.shaderCube.use();
    attributes.shaderCube.setVec3("color", color);
    viewMatrix = generalInfo.viewMatrix;    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale));
    attributes.shaderCube.setMat4("transform", projectionMatrix * viewMatrix * model); 
    glBindVertexArray(attributes.VAO);
    glDrawArrays(GL_TRIANGLES,0,36);

    originShader.use();
}

    
