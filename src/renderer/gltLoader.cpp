#include "renderer/gltLoader.h"
#include "renderer/imageLoader.h"
#include "renderer/shaders.h"
#include <string>


std::map<std::string, glTexture> GLTloader::texturesLoaded;

GLTloader::GLTloader(const std::string &path)
{
    //retrieve path of models directory, to later use when accessing model's textures
    directory = path.substr(0, path.find_last_of('/'));

    int pos = path.find(".");
    std::string type = path.substr(pos+1);
    //loading model
    if(!loadModel(path, type))
    {
        std::cout<<"Could not load model"<<std::endl;
    }
}

bool GLTloader::loadModel(const std::string &path, const std::string &type)
{
    
    //Holds the whole scene
    tinygltf::Model model;

    //The file reader
    tinygltf::TinyGLTF loader;

    //Check for error and warnings
    std::string error, warning;

    //Check file type
    bool ret;
    if(type == "gltf")
    {
        //load file into memory
        ret = loader.LoadASCIIFromFile(&model, &error, &warning, path);
    }
    else if(type == "glb")
    {
        //load file into memory
        ret = loader.LoadBinaryFromFile(&model, &error, &warning, path);
    }
    else
    {
        std::cout<<"ERROR->invalid file type"<<std::endl;
        return false;
    }


    //Check for any errors and warnings
    if(!error.empty()) std::cout<<"ERROR-> "<<error<<std::endl;
    if(!warning.empty()) std::cout<<"WARNING-> "<<warning<<std::endl;

    //Check if file loaded properly
    if(!ret) return false;
    else std::cout<<"Loading model: "<<path<<std::endl;



    //looping through and working on all the meshes
    for(const auto &mesh : model.meshes)
    {
        //looping over all its primitives
        for(const auto &prim : mesh.primitives)
        {
            gltMesh glMesh;
            std::vector<gltVertex> vertices;
            std::vector<GLuint> indices;
            int attributesSize = 0;


            //Read the primitives
            
            //Position
            const float *posData = nullptr;
            if(prim.attributes.find("POSITION")!=prim.attributes.end())
            {
                //Accessor
                const auto &posAccesor = model.accessors[prim.attributes.find("POSITION")->second];
                attributesSize = posAccesor.count;
                //Buffer view
                const auto &posView = model.bufferViews[posAccesor.bufferView];
                //Buffer
                const auto &posBuffer = model.buffers[posView.buffer];

                //in a .gltf file:
                //This vertex data is stored deep inside a big byte blob.
                //There is no direct float* or std::vector<float> — it's just binary bytes.
                //So we must extract the required float* from it
                //So we must extract it and reinterpret it.

                //Think of it like this
                //buffer.data = the whole book
                //byteOffset = skip to chapter 3 (positions)
                //accessor.byteOffset = skip 20 words into the chapter
                //reinterpret_cast<const float*> = start reading those words as float numbers, not raw bytes

                //We perform it using this shit looking line
               
                posData = reinterpret_cast<const float*>(&posBuffer.data[posView.byteOffset + posAccesor.byteOffset]);

            }

            //Normal
            const float *normalData = nullptr;
            if(prim.attributes.find("NORMAL")!=prim.attributes.end())
            {
                //Accessor
                const auto &normalAccesor = model.accessors[prim.attributes.find("NORMAL")->second];
                //Buffer view
                const auto &normalView = model.bufferViews[normalAccesor.bufferView];
                //Buffer
                const auto &normalBuffer = model.buffers[normalView.buffer];

                normalData = reinterpret_cast<const float*>(&normalBuffer.data[normalView.byteOffset + normalAccesor.byteOffset]);

            }            

            //Texture coordinates
            const float *texCoordData = nullptr;
            if(prim.attributes.find("TEXCOORD_0")!=prim.attributes.end())
            {
                //Accessor
                const auto &texCoordAccesor = model.accessors[prim.attributes.find("TEXCOORD_0")->second];
                //Buffer view
                const auto &texCoordView = model.bufferViews[texCoordAccesor.bufferView];
                //Buffer
                const auto &texCoordBuffer = model.buffers[texCoordView.buffer];

                texCoordData = reinterpret_cast<const float*>(&texCoordBuffer.data[texCoordView.byteOffset + texCoordAccesor.byteOffset]);

            }            
            
            // std::cout<<"in gltLoader, attribute size is"<<attributesSize<<std::endl;
            for(int i=0; i< attributesSize; ++i)
            {
                gltVertex vertice;
                if(posData)
                {
                    vertice.Position = glm::vec3(posData[i*3 + 0],posData[i*3 + 1],posData[i*3 + 2]);
                }
                else
                {
                    std::cout<<"Could not load vertice Positions"<<std::endl;
                    return false;
                }
                if(normalData)
                {
                    vertice.Normal = glm::vec3(normalData[i*3 + 0],normalData[i*3 + 1],normalData[i*3 + 2]);
                }
                else
                {
                    std::cout<<"Could not load vertice Normals"<<std::endl;
                    return false;
                }
                if(texCoordData)
                {
                    vertice.TexCoords = glm::vec2(texCoordData[i*2 + 0],texCoordData[i*2 + 1]);
                }
                else
                {
                    std::cout<<"Could not load vertice Positions"<<std::endl;
                    return false;
                }
                vertices.push_back(vertice);
            }





            //Reading indices
            if (prim.indices >=0)
            {
                const auto &idAccesor = model.accessors[prim.indices];
                const auto &idView = model.bufferViews[idAccesor.bufferView];
                const auto &idBuffer = model.buffers[idView.buffer];

                //retrieving float data
                const unsigned char* data = idBuffer.data.data() + idView.byteOffset + idAccesor.byteOffset;
                
                //Type casting it to proper format
                switch(idAccesor.componentType)
                {
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE :
                        {
                            const uint8_t *idData = reinterpret_cast<const uint8_t*>(data);
                            for(int i=0; i<idAccesor.count; ++i)
                            {
                                indices.push_back(idData[i]);
                            }
                            break;
                        }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT :
                        {
                            const uint16_t *idData = reinterpret_cast<const uint16_t*>(data);
                            for(int i=0; i<idAccesor.count; ++i)
                            {
                                indices.push_back(idData[i]);
                            }
                            break;
                        }
                    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT :
                        {
                            const uint32_t *idData = reinterpret_cast<const uint32_t*>(data);
                            for(int i=0; i<idAccesor.count; ++i)
                            {
                                indices.push_back(idData[i]);
                            }
                            break;
                        }
                    default :
                        {
                            std::cout<<"Unsupported index type"<<std::endl;
                            return false;
                        }

                }

            }


            //Working on our textures now
            if(prim.material >= 0)
            {
                const auto &material = model.materials[prim.material];
                const auto &pbr = material.pbrMetallicRoughness;

                if(pbr.baseColorTexture.index >= 0)
                {
                    int texIndex = pbr.baseColorTexture.index;
                    int imgIndex = model.textures[texIndex].source;
                    glMesh.metallicFactor = pbr.metallicFactor;
                    glMesh.roughnessFactor = pbr.roughnessFactor;
                    std::cout<<"Mf is: "<<glMesh.metallicFactor<<std::endl;
                    std::cout<<"rf is: "<<glMesh.roughnessFactor<<std::endl;

                    const auto &image = model.images[imgIndex];
                    if(!image.uri.empty())
                    {
                        std::string imagePath = directory+ "/"+ image.uri;
                        std::cout<<"ImagePath: "<<imagePath<<std::endl;
                        auto it = texturesLoaded.find(imagePath);
                        if(it!=texturesLoaded.end())
                        {
                            glMesh.texture.push_back(it->second);
                        }
                        else
                        {
                            std::cout<<"Trying to load diffuse: "<<imagePath<<std::endl;
                            glTexture texture;
                            texture.ID = loadTextureFromFile(imagePath, false);
                            texture.type = "diffuse";
                            texture.path = imagePath;
                            glMesh.texture.push_back(texture);
                            texturesLoaded[imagePath] = texture;
                        }

                    }
                    else if(!image.image.empty())
                    {
                        std::cout << "Loading embedded diffuse from memory" << std::endl;
                        int imageWidth = image.width;
                        int imageHeight = image.height;
                        int numChannels = image.component;
                        glTexture texture;
                        texture.ID = loadTextureFromBinary(image.image.data(), imageWidth, imageHeight, numChannels);
                        texture.type = "diffuse";
                        glMesh.texture.push_back(texture);
                    }

                    else
                    {
                        std::cout<<"no diffuse texture"<<std::endl;
                    }
                }

                if(pbr.metallicRoughnessTexture.index >= 0)
                {
                    int texIndex = pbr.metallicRoughnessTexture.index;
                    int imgIndex = model.textures[texIndex].source;
                    const auto &image = model.images[imgIndex];

                    if(!image.uri.empty())
                    {
                        std::string imagePath = directory+ "/"+ image.uri;
                        auto it = texturesLoaded.find(imagePath);
                        if(it!=texturesLoaded.end())
                        {
                            glMesh.texture.push_back(it->second);
                        }
                        else
                        {
                            std::cout<<"Trying to load specular texture: "<<imagePath<<std::endl;
                            glTexture texture;
                            texture.ID = loadTextureFromFile(imagePath, false);
                            texture.type = "specular";
                            texture.path = imagePath;
                            glMesh.texture.push_back(texture);
                            texturesLoaded[imagePath] = texture;
                        }

                    }

                    else if(!image.image.empty())
                    {
                        std::cout << "Loading embedded specular from memory" << std::endl;
                        int imageWidth = image.width;
                        int imageHeight = image.height;
                        int numChannels = image.component;
                        glTexture texture;
                        texture.ID = loadTextureFromBinary(image.image.data(), imageWidth, imageHeight, numChannels);
                        texture.type = "specular";
                        glMesh.texture.push_back(texture);
                    }
                    else
                    {
                        std::cout<<"no specular texture"<<std::endl;
                    }

                    // glMesh.metallicRoughnessTexture = mrTexture; 
                }
                else std::cout<<"no metallicRoughnessTexture"<<std::endl;
            }



        

            //Setting up opengl buffers
            glGenVertexArrays(1,&glMesh.VAO);
            glGenBuffers(1, &glMesh.VBO);
            glGenBuffers(1, &glMesh.EBO);

            glBindVertexArray(glMesh.VAO);

            glBindBuffer(GL_ARRAY_BUFFER, glMesh.VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(gltVertex), &vertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glMesh.EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gltVertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gltVertex), (void*)offsetof(gltVertex,Normal));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gltVertex), (void*)offsetof(gltVertex,TexCoords));
            glEnableVertexAttribArray(2);


            glBindVertexArray(0);

            //Passing number of indices to our mesh struct
            glMesh.indexCount = indices.size();

            //pushing our final big mesh
            meshes.push_back(glMesh);

        }
    
    }

    std::cout<<"Loaded model properly"<<std::endl;
    return true;
}


void GLTloader::Draw(Shader &shader)
{
    //Drawing model
    shader.use();
    shader.setInt("material.diffuse0",0);
    shader.setInt("material.specular0",1);

    for(auto &mesh : meshes)
    {
        mesh.Draw(shader);
    }
}




