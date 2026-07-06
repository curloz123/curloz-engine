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
#include "core/logs.hpp"
#include "../include/gizmo.hpp"
#include "renderer/vk_types.hpp"
#include "../include/scenetable.hpp"
#include <cmath>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <vector>
#include <vulkan/vulkan.h>
#include <window/window.hpp>

namespace clz::editor
{
	void render()
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
		ImGui::PushFont(fontSansBold);

		ImGui::Begin("Scene");
		ImGui::PopFont();
		if (ImGui::BeginTabBar("Scene"))
		{
			showSceneTab();
			ImGui::EndTabBar();
		}
		processShortcuts();
		drawGizmo();
		showInspector();

		ImGui::End();

	}
	bool init()
	{
		/// Create ImGui's descriptor pool
		std::vector<VkDescriptorPoolSize> poolSizes = {{VK_DESCRIPTOR_TYPE_SAMPLER, 100},
							       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100},
							       {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100},
							       {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100},
							       {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100},
							       {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100},
							       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100},
							       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100},
							       {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100},
							       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100},
							       {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100}};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolInfo.maxSets = 100 * poolSizes.size();
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		if (vkCreateDescriptorPool(renderer::r_deviceContext.device, &poolInfo, nullptr, &editorDescriptorPool) != VK_SUCCESS)
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
		setTheme();

		// Apply gamma correction
		for (auto& col : ImGui::GetStyle().Colors)
		{
			col.x = std::pow(col.x, 2.2f);
			col.y = std::pow(col.y, 2.2f);
			col.z = std::pow(col.z, 2.2f);
		}

		/// Fonts
		fontSans = io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansNerdFont-Black.ttf", 18.0f);
		fontSansBold = io.Fonts->AddFontFromFileTTF("assets/fonts/NotoSansNerdFont-Bold.ttf", 20.0f);
		fontMono = io.Fonts->AddFontFromFileTTF("assets/fonts/JetBrainsMonoNerdFont-Regular.ttf", 17.0f);
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
		vkDeviceWaitIdle(clz::renderer::r_deviceContext.device);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		vkDestroyDescriptorPool(renderer::r_deviceContext.device, editorDescriptorPool, nullptr);
	}

	void setTheme()
	{
		// Rest style by AaronBeardless from ImThemes (Thank you AaronBeardless)
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 0.8f;
		style.DisabledAlpha = 0.5f;
		style.WindowPadding = ImVec2(13.0f, 10.0f);
		style.WindowRounding = 0.0f;
		style.WindowBorderSize = 1.0f;
		style.WindowMinSize = ImVec2(32.0f, 32.0f);
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.WindowMenuButtonPosition = ImGuiDir_Right;
		style.ChildRounding = 3.0f;
		style.ChildBorderSize = 1.0f;
		style.PopupRounding = 5.0f;
		style.PopupBorderSize = 1.0f;
		style.FramePadding = ImVec2(20.0f, 8.1f);
		style.FrameRounding = 2.0f;
		style.FrameBorderSize = 0.0f;
		style.ItemSpacing = ImVec2(3.0f, 3.0f);
		style.ItemInnerSpacing = ImVec2(3.0f, 8.0f);
		style.CellPadding = ImVec2(6.0f, 14.1f);
		style.IndentSpacing = 0.0f;
		style.ColumnsMinSpacing = 10.0f;
		style.ScrollbarSize = 10.0f;
		style.ScrollbarRounding = 2.0f;
		style.GrabMinSize = 12.1f;
		style.GrabRounding = 1.0f;
		style.TabRounding = 2.0f;
		style.TabBorderSize = 0.0f;
		style.ColorButtonPosition = ImGuiDir_Right;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

		style.Colors[ImGuiCol_Text] = ImVec4(0.98039216f, 0.98039216f, 0.98039216f, 1.0f);
		style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.49803922f, 0.49803922f, 0.49803922f, 1.0f);
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
		style.Colors[ImGuiCol_ChildBg] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
		style.Colors[ImGuiCol_PopupBg] = ImVec4(0.09411765f, 0.09411765f, 0.09411765f, 1.0f);
		style.Colors[ImGuiCol_Border] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
		style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.047058824f);
		style.Colors[ImGuiCol_TitleBg] = ImVec4(0.11764706f, 0.11764706f, 0.11764706f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.11764706f, 0.11764706f, 0.11764706f, 1.0f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.10980392f);
		style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(1.0f, 1.0f, 1.0f, 0.39215687f);
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.47058824f);
		style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.09803922f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 1.0f, 1.0f, 0.39215687f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3137255f);
		style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.047058824f);
		style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.047058824f);
		style.Colors[ImGuiCol_Separator] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
		style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
		style.Colors[ImGuiCol_SeparatorActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
		style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
		style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
		style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
		style.Colors[ImGuiCol_Tab] = ImVec4(1.0f, 1.0f, 1.0f, 0.09803922f);
		style.Colors[ImGuiCol_TabHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.15686275f);
		style.Colors[ImGuiCol_TabActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3137255f);
		style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.0f, 0.0f, 0.0f, 0.15686275f);
		style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.23529412f);
		style.Colors[ImGuiCol_PlotLines] = ImVec4(1.0f, 1.0f, 1.0f, 0.3529412f);
		style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_PlotHistogram] = ImVec4(1.0f, 1.0f, 1.0f, 0.3529412f);
		style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.15686275f, 0.15686275f, 0.15686275f, 1.0f);
		style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(1.0f, 1.0f, 1.0f, 0.3137255f);
		style.Colors[ImGuiCol_TableBorderLight] = ImVec4(1.0f, 1.0f, 1.0f, 0.19607843f);
		style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.019607844f);
		style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_DragDropTarget] = ImVec4(0.16862746f, 0.23137255f, 0.5372549f, 1.0f);
		style.Colors[ImGuiCol_NavHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
		style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
		style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.5647059f);
	}
} // namespace clz::editor