/**
 * @file editor.cpp
 * @author curl0z
 *
 * @brief Sandbox editor implementation
 *
 */
#include "renderer/editor/editor.hpp"
#include "core/logs.hpp"
#include "renderer/editor/editor_types.hpp"
#include "renderer/editor/inspector.hpp"
#include "renderer/editor/playertable.hpp"
#include "renderer/vk_types.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <cmath>
#include <vector>
#include <vulkan/vulkan.h>
#include <window/window.hpp>

namespace clz::editor
{
	void render()
	{
		ImGui::DockSpaceOverViewport(
			ImGui::GetMainViewport()->ID,
			ImGui::GetMainViewport(),
			ImGuiDockNodeFlags_PassthruCentralNode
		);

		ImGui::PushFont(fontSansBold);
		ImGui::Begin("Players");
		ImGui::PopFont();
		if (ImGui::BeginTabBar("Players"))
		{
			showEntityTab();
			ImGui::EndTabBar();
		}
		ImGui::End();

		showInspector();
	}
	bool init()
	{
		/// Create ImGui's descriptor pool
		std::vector<VkDescriptorPoolSize> poolSizes = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 100 * poolSizes.size();
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		if (vkCreateDescriptorPool(renderer::r_deviceContext.device, &poolInfo,
			nullptr, &editorDescriptorPool) != VK_SUCCESS)
		{
			clz::log::error("Editor's vkCreateDescriptorPool failed");
			return false;
		}


		/// Initialize ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		/// Colors
		ImGui::StyleColorsDark();

		// Apply gamma correction
		for (auto& col: ImGui::GetStyle().Colors)
		{
			col.x = std::pow(col.x, 2.2f);
			col.y = std::pow(col.y, 2.2f);
			col.z = std::pow(col.z, 2.2f);
		}

		/// Fonts
		fontSans     = io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansNerdFont-Black.ttf", 18.0f);
		fontSansBold = io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansNerdFont-Bold.ttf", 20.0f);
		fontMono     = io.Fonts->AddFontFromFileTTF("assets/fonts/JetBrainsMonoNerdFont-Regular.ttf", 17.0f);
		fontMonoBold = io.Fonts->AddFontFromFileTTF("assets/fonts/JetBrainsMonoNerdFont-Bold.ttf", 19.0f);

		if (!fontSans || !fontSansBold || !fontMono || !fontMonoBold)
		{
			clz::log::error("Failed to load one or more fonts");
			return false;
		}

		ImGui_ImplGlfw_InitForVulkan(window::getWindowHandle(), true);

		// Dynamic rendering: tell ImGui which color format it'll render into
		VkPipelineRenderingCreateInfo pipelineRenderingInfo{};
		pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingInfo.colorAttachmentCount = 1;
		pipelineRenderingInfo.pColorAttachmentFormats = &renderer::r_swapchainContext.format.format;
		pipelineRenderingInfo.depthAttachmentFormat = renderer::r_swapchainContext.depthFormat;


		/// initInfo
		ImGui_ImplVulkan_InitInfo initInfo{};
		initInfo.ApiVersion = VK_API_VERSION_1_3;
		initInfo.Instance = renderer::r_deviceContext.instance;
		initInfo.PhysicalDevice = renderer::r_deviceContext.physicalDevice;
		initInfo.Device = renderer::r_deviceContext.device;
		initInfo.QueueFamily = renderer::r_deviceContext.graphicsFamily.value();
		initInfo.Queue = renderer::r_deviceContext.graphicsQueue;
		initInfo.DescriptorPool = editorDescriptorPool;
		initInfo.MinImageCount = renderer::r_swapchainContext.images.size();
		initInfo.ImageCount = renderer::r_swapchainContext.images.size();
		initInfo.UseDynamicRendering = true;
		initInfo.PipelineCache = VK_NULL_HANDLE;
		initInfo.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.PipelineInfoMain.PipelineRenderingCreateInfo = pipelineRenderingInfo;

		ImGui_ImplVulkan_Init(&initInfo);

		clz::log::info("Initialized editor");

		return true;
	}

	void update(VkCommandBuffer commandBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		/// All Rendering functions go here
		render();

		ImGui::Render();
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
	}

	void shutdown()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDestroyDescriptorPool(renderer::r_deviceContext.device,
					editorDescriptorPool, nullptr);
	}
}