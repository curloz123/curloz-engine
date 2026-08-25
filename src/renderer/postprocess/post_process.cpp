#include "renderer/pipelinedata/post_process.hpp"
#include "core/logs.hpp"
#include "renderer/postprocess/bloom.hpp"
#include "renderer/postprocess/bloom_sample.hpp"
#include "renderer/postprocess/post_tonemap.hpp"
#include "renderer/postprocess/tonemap.hpp"

// assumes swapchain context has been initialized
namespace clz::renderer::post_process
{
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
	void destroyPostProcesses()
	{
		destroyPostTonemapProcess();
		destroyTonemapProcess();
		destroyBloomProcess();
		destroyBloomSampleProcess();
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
		applyBloomSampleProcess(commandBuffer);
		applyBloomProcess(commandBuffer);
		applyTonemapProcess(commandBuffer);
		applyPostTonemapProcess(commandBuffer);
	}

	void hintPostProcessStateChange()
	{
		updatePostProcessDescriptorSets();
	}


}
