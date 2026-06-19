#pragma once 

#include <string>
#include "AL/al.h"

namespace clz::audio 
{
	ALuint loadBuffer(const std::string& filePath);
	ALuint getBufferID(const char* fileName);
	void deleteBuffer(const ALuint buffer);
	void deleteAllBuffers();
}
