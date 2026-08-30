/**
 * @file post_process.cpp
 * @author curl0z
 * @brief Main post process implementation file
 * Provides all the basic sub-system level functions
 * @note assumes swapchain context has been initialized
 */
#include "renderer/pipelinedata/post_process.hpp"
#include "core/logs.hpp"
#include "renderer/postprocess/bloom.hpp"
#include "renderer/postprocess/bloom_sample.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/postprocess/tonemap.hpp"

namespace clz::renderer::post_process
{
	/// @copydoc initializePostProcesses
	bool initializePostProcesses()
	{
		if (!createBloomSampleProcess())
		{
			clz::log::error("failed to create bloom sample process");
			return false;
		}
		
		if (!createBloomProcess())
		{
			clz::log::error("failed to create bloom process");
			return false;
		}

		if (!createTonemapProcess())
		{
			clz::log::error("failed to create tonemap pass");
			return false;
		}

		if (!createPostTonemapProcess())
		{
			clz::log::error("failed to create post-tonemap resources");
			return false;
		}

		updatePostProcessDescriptorSets();		
		clz::log::info("Initialized post process resources");
		return true;
	}

	/// @copydoc destroyPostProcesses
	void destroyPostProcesses()
	{
		destroyPostTonemapProcess();
		destroyTonemapProcess();
		destroyBloomProcess();
		destroyBloomSampleProcess();
	}

	/// @copydoc recreatePostProcesses
	bool recreatePostProcesses()
	{
		vkDeviceWaitIdle(r_deviceContext.device);

		destroyPostProcesses();

		if (!initializePostProcesses())
		{
			clz::log::error("failed to re-initialize post-process resources");
			return false;
		}

		updatePostProcessDescriptorSets();

		return true;
	}

	/// @copydoc applyPostProcessing
	void applyPostProcessing(VkCommandBuffer commandBuffer)
	{
		applyBloomSampleProcess(commandBuffer);
		applyBloomProcess(commandBuffer);
		applyTonemapProcess(commandBuffer);
		applyPostTonemapProcess(commandBuffer);
	}

	/// @copydoc hintPostProcessStateChange
	void hintPostProcessStateChange()
	{
		updatePostProcessDescriptorSets();
	}


}
