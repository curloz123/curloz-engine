#pragma once
#include "stb_image.h"
#include "glad/glad.h"
#include <iostream>
#include <string>

GLuint loadTextureFromFile(const std::string &path, bool flip);

