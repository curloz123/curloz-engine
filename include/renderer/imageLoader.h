#pragma once
#include "stb_image.h"
#include "glad/glad.h"
#include <iostream>
#include <string>

GLuint loadTextureFromFile(const std::string &path, bool flip);
GLuint loadTextureFromBinary(const unsigned char *pixelData, int imageWidth, int imageHeight, int numChannels);
