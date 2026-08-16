#pragma once

#include <cstdint>

namespace clz::renderer
{
	bool initRenderTargetContext(uint32_t width, uint32_t height);
	void destroyRenderTargetContext();
	bool recreateRenderTargetContext(uint32_t width, uint32_t height);
}