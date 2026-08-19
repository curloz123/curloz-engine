#include "renderer/pipelinedata/post_process.hpp"
#include "core/assert.hpp"
#include "core/logs.hpp"
#include "renderer/postprocess/pre_tonemap.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/postprocess/tonemap.hpp"

// assumes swapchain context has been initialized
namespace clz::renderer::post_process
{
	bool initializePostProcesses()
	{
		if (!createPreTonemapProcess())
		{
			clz::log::error("failed to create pre-tonemap resources");
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

		clz::log::info("Initialized post process resources");
		return true;
	}
	void destroyPostProcesses()
	{
		destroyPostTonemapProcess();
		destroyTonemapProcess();
		destroyPreTonemapProcess();
	}
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

	void applyPostProcessing(VkCommandBuffer commandBuffer)
	{
		applyPreTonemapProcess(commandBuffer);
		applyTonemapProcess(commandBuffer);
		applyPostTonemapProcess(commandBuffer);
	}

	void hintPostProcessStateChange()
	{
		updatePostProcessDescriptorSets();
	}


}
