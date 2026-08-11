#include "include/inspector/lightcomponent.hpp"
#include "include/timemachine.hpp"
#include "imgui.h"
#include "entity/componentmanager.hpp"
#include "include/editor_types.hpp"
#include "include/scenetable.hpp"
#include "renderer/lighting/lights.hpp"
#include "renderer/lighting/lighting.hpp"
#include "renderer/rendercomponent.hpp"

namespace clz::editor
{

void showDirectionalLightHeader()
{
	static renderer::DirectionalLight previousDirLight;

	if (!ImGui::CollapsingHeader("Directional Light"))
		return;

	const auto entityId = currentSelectedEntity.value();
	const auto DirLightId = ecs::getComponent<renderer::DirectionalLightComponent>(entityId);
	auto& rLight = renderer::Lights.directionalLight[DirLightId.Id.value];

	bool anyEditFinished = false;

	ImGui::Text("Color");

	ImGui::ColorEdit3("Color", &rLight.color.x, ImGuiColorEditFlags_PickerHueWheel);
	if (ImGui::IsItemActivated())
	{
		previousDirLight = rLight;
	}
	if (ImGui::IsItemDeactivated())
	{
		anyEditFinished = true;
	}
	ImGui::Separator();

	ImGui::Text("Intensity");
	ImGui::InputFloat("##Intensity: ", &rLight.color.w);
	if (ImGui::IsItemActivated())
	{
		previousDirLight = rLight;
	}
	if (ImGui::IsItemDeactivated())
	{
		anyEditFinished = true;
	}
	ImGui::Separator();

	ImGui::Text("Direction (XYZ)");

	//ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x / 3.0f);
	ImGui::SliderFloat3("##DirX", &rLight.direction.x, -1.0f, 1.0f);
	if (ImGui::IsItemActivated())
		previousDirLight = rLight;
	if (ImGui::IsItemDeactivated())
		anyEditFinished = true;

	if (anyEditFinished)
	{
		const auto prevDirLight = previousDirLight;
		const auto newDirLight = rLight;
		timemachine::createSnapshot(
			[entityId, prevDirLight] {
				const auto DirLightComp = ecs::getComponent<renderer::DirectionalLightComponent>(entityId);
				renderer::Lights.directionalLight[DirLightComp.Id.value] = prevDirLight;
			},
			[entityId, newDirLight] {
				const auto DirLightComp = ecs::getComponent<renderer::DirectionalLightComponent>(entityId);
				renderer::Lights.directionalLight[DirLightComp.Id.value] = newDirLight;
			});
	}

}

void showPointLightHeader()
{
	static renderer::PointLight previousPointLight;

	if (!ImGui::CollapsingHeader("Point Light"))
		return;

	const auto entityId = currentSelectedEntity.value();
	const auto pointLightId = ecs::getComponent<renderer::PointLightComponent>(entityId);
	auto& rLight = renderer::Lights.pointLights[pointLightId.Id.value];

	bool anyEditFinished = false;

	ImGui::Text("Color");

	ImGui::ColorEdit3("Color", &rLight.color.x, ImGuiColorEditFlags_PickerHueWheel);
	if (ImGui::IsItemActivated())
	{
		previousPointLight = rLight;
	}
	if (ImGui::IsItemDeactivated())
	{
		anyEditFinished = true;
	}
	ImGui::Separator();

	ImGui::Text("Intensity");
	ImGui::InputFloat("##Intensity: ", &rLight.intensity);
	if (ImGui::IsItemActivated())
	{
		previousPointLight = rLight;
	}
	if (ImGui::IsItemDeactivated())
	{
		anyEditFinished = true;
	}
	ImGui::Separator();

	ImGui::Text("Range");
	ImGui::SliderFloat("##range", &rLight.range, 0.1f, 100.0f);
	if (ImGui::IsItemActivated())
		previousPointLight = rLight;
	if (ImGui::IsItemDeactivated())
		anyEditFinished = true;
	ImGui::Separator();

	ImGui::PushFont(fontMonoBold);
	ImGui::Text("Attenuation");
	ImGui::PopFont();

	ImGui::Text("Linear: ");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputFloat("##Linear", &rLight.attenuation.y, 0.0f, 1.0f);
	if (ImGui::IsItemActivated())
		previousPointLight = rLight;
	if (ImGui::IsItemDeactivated())
		anyEditFinished = true;

	ImGui::Text("Quadratic");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::InputFloat("##Quadratic", &rLight.attenuation.z, 0.0f, 1.0f);
	if (ImGui::IsItemActivated())
		previousPointLight = rLight;
	if (ImGui::IsItemDeactivated())
		anyEditFinished = true;


	if (anyEditFinished)
	{
		const auto prevPointLight = previousPointLight;
		const auto newPointLight = rLight;
		timemachine::createSnapshot(
			[entityId, prevPointLight] {
				const auto pointLightComp = ecs::getComponent<renderer::PointLightComponent>(entityId);
				renderer::Lights.pointLights[pointLightComp.Id.value] = prevPointLight;
			},
			[entityId, newPointLight] {
				const auto pointLightComp = ecs::getComponent<renderer::PointLightComponent>(entityId);
				renderer::Lights.pointLights[pointLightComp.Id.value] = newPointLight;
			});
	}


}

}