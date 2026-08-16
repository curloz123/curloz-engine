/**
 * @file editor.cpp
 * @author curl0z
 *
 * @brief Sandbox editor implementation
 */

#include "../include/editor.hpp"
#include "../include/editor_types.hpp"
#include "../include/editorshortcuts.hpp"
#include "../include/inspector/inspector.hpp"
#include "../include/scenetable.hpp"
#include "../include/sceneview.hpp"
#include "../include/timemachine.hpp"
#include "../include/topbar.hpp"
#include "core/logs.hpp"
#include "entity/componentmanager.hpp"
#include "entity/corecomponents.hpp"
#include "include/offscreen/offscreentarget.hpp"
#include "math/angle.hpp"
#include "renderer/vk_types.hpp"
#include <cmath>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <window/window.hpp>
#include "renderer/cross_system_flags.hpp"

namespace clz::editor
{

/// @copydoc render
void render(VkCommandBuffer commandBuffer)
{
	/// @brief top menu bar
	showTopBar();
	setDockSpace();

	ImGui::PushFont(fontSansBold);
	ImGui::Begin("Scene");
	ImGui::PopFont();
	if (ImGui::BeginTabBar("Scene"))
	{
		showSceneTab();
		ImGui::EndTabBar();
	}
	drawSceneView();
	processShortcuts();
	showInspector(commandBuffer);
	timemachine::timeTravel();

	ImGui::End();
}

/// @copydoc init
bool init()
{
	/// Create ImGui's descriptor pool
	std::vector<VkDescriptorPoolSize> poolSizes = {
		{VK_DESCRIPTOR_TYPE_SAMPLER, 10},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10},
		{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 10},
		{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10},
		{VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 10},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 10},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 10},
		{VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 10}
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = 100 * poolSizes.size();
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();

	if (vkCreateDescriptorPool(
		    renderer::r_deviceContext.device,
		    &poolInfo,
		    nullptr,
		    &editorDescriptorPool
	    ) != VK_SUCCESS)
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
	// io.ConfigFlags |= ImGuiBackendFlags_RendererHasTextures;

	/// Colors
	setTheme();

	// Apply gamma correction
	for (auto& col : ImGui::GetStyle().Colors)
	{
		col.x = std::pow(col.x, 4.0f);
		col.y = std::pow(col.y, 4.0f);
		col.z = std::pow(col.z, 4.0f);
	}

	ImFontConfig config_default;

	// Load Nerd Fonts as independent fonts (MergeMode = false by default)
	fontSans = io.Fonts->AddFontFromFileTTF(
		"assets/fonts/NotoSansNerdFont-Black.ttf",
		18.0f,
		&config_default
	);

	fontSansBold = io.Fonts->AddFontFromFileTTF(
		"assets/fonts/NotoSansNerdFont-Bold.ttf",
		20.0f,
		&config_default
	);

	fontMono = io.Fonts->AddFontFromFileTTF(
		"assets/fonts/JetBrainsMonoNerdFont-Regular.ttf",
		17.0f,
		&config_default
	);

	fontMonoBold = io.Fonts->AddFontFromFileTTF(
		"assets/fonts/JetBrainsMonoNerdFont-Bold.ttf",
		19.0f,
		&config_default
	);

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
	pipelineRenderingInfo.depthAttachmentFormat =
		renderer::r_renderTargetContext.depthFormat;

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
	createOffscreenTargets();


	renderer::flagRendererEditorInitialized();
	clz::log::info("Initialized editor");

	return true;
}

void prepareEditor()
{
	for (const auto& entityId : ecs::getEntitiesWithComponent<ecs::TransformComponent>())
	{
		const auto editorTransform =
			ecs::getComponent<ecs::EditorTransformComponent>(entityId);

		ecs::setComponent<ecs::TransformComponent>(
			entityId,
			ecs::TransformComponent(
				math::quatFromEuler(math::radians(editorTransform.rotation)),
				editorTransform.position,
				editorTransform.scale
			)
		);
	}
}

void update(VkCommandBuffer commandBuffer)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	/// All Rendering functions go here
	render(commandBuffer);
	presentOffscreenWindows();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void shutdown()
{
	vkDeviceWaitIdle(clz::renderer::r_deviceContext.device);
	destroyOffscreenTargets();
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	vkDestroyDescriptorPool(
		renderer::r_deviceContext.device,
		editorDescriptorPool,
		nullptr
	);
}

void setTheme()
{
}
} // namespace clz::editor
