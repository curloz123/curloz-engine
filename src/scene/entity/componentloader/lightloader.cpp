#include "core/logs.hpp"
#include "entity/componentmanager.hpp"
#include "math/vec3.hpp"
#include "renderer/lighting/lighting.hpp"
#include "scene/entity/loader.hpp"

namespace clz::scene
{
	/// @copydoc retrieveDirectionalLightComponent
	std::expected<renderer::DirectionalLightComponent, std::string>
	retrieveDirectionalLightComponent(
			const nlohmann::json& componentData,
			std::string_view entityName)
	{
		math::vec3 direction;
		{
			if (componentData.contains("direction"))
			{
				const auto dirArray =
					componentData["direction"].get<std::array<float, 3>>();
				direction = math::vec3(dirArray[0], dirArray[1], dirArray[2]);
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': directional "
					"light does not have direction component, assigning it "
					"default value"
				);
				direction = math::vec3(0.0f, -1.0f, 0.0f);
			}
		}

		math::vec3 color;
		{
			if (componentData.contains("color"))
			{
				const auto colorArray =
					componentData["color"].get<std::array<float, 3>>();
				color = math::vec3(colorArray[0], colorArray[1], colorArray[2]);
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': color "
					"component does not have color, assigning it default "
					"value"
				);
				color = math::vec3(1.0f);
			}
		}

		float intensity;
		{
			if (componentData.contains("intensity"))
			{
				intensity = componentData["intensity"].get<float>();
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': intensity "
					"component does not have intensity component, assigning "
					"it default value"
				);
				intensity = 1.0f;
			}
		}

		const auto result = renderer::registerDirectionalLight(direction, color, intensity);
		if (!result)
		{
			const auto error = result.error();
			switch (error)
			{
			case renderer::LightRegisterError::INVALID_PARAMETER:
				clz::log::warn(
					"entity '" + std::string(entityName) + "': directional "
					"light entry in JSON has invalid parameters, unless "
					"resolved, directional light will be disabled"
				);
				break;

			case renderer::LightRegisterError::LIMIT_EXCEEDED:
				clz::log::warn(
					"entity '" + std::string(entityName) + "': MAX LIMIT "
					"reached for directional lights, cannot add further "
					"lights"
				);
				break;
			}
			return std::unexpected("Could not register directional light");
		}

		renderer::DirectionalLightComponent dirLightComp;
		dirLightComp.Id = result.value();
		return dirLightComp;
	}

	/// @copydoc saveDirectionalLightComponent
	void saveDirectionalLightComponent(
		const renderer::DirectionalLightComponent& dlt,
		nlohmann::json& componentData,
		std::string_view entityName
	)
	{
		const auto LightId = dlt.Id;

		const auto dir = renderer::getDirLightDirection(LightId);
		componentData["direction"] = {dir.x, dir.y, dir.z};

		const auto col = renderer::getDirLightColor(LightId);
		componentData["color"] = {col.x, col.y, col.z};

		const float intensity = renderer::getDirLightIntensity(LightId);
		componentData["intensity"] = intensity;

		clz::log::info(
			"Saved directional light component for entity '" +
			std::string(entityName) + "'"
		);
	}

	/// @copydoc retrievePointLightComponent
	std::expected<renderer::PointLightComponent, std::string>
	retrievePointLightComponent(
			const nlohmann::json& componentData,
			std::string_view entityName)
	{
		math::vec3 position;
		{
			if (componentData.contains("position"))
			{
				const auto posArray =
					componentData["position"].get<std::array<float, 3>>();
				position = math::vec3(posArray[0], posArray[1], posArray[2]);
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': point light "
					"does not have position field, assigning it default "
					"value"
				);
				position = math::vec3(0.0f, -1.0f, 0.0f);
			}
		}
		float range;
		{
			if (componentData.contains("range"))
			{
				range = componentData["range"].get<float>();
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': point light "
					"does not have range, assigning it default value"
				);
				range = 10.0f;
			}
		}
		math::vec3 color;
		{
			if (componentData.contains("color"))
			{
				const auto colorArray =
					componentData["color"].get<std::array<float, 3>>();
				color = math::vec3(colorArray[0], colorArray[1], colorArray[2]);
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': point light "
					"does not have color, assigning it default value"
				);
				color = math::vec3(1.0f);
			}
		}
		float intensity;
		{
			if (componentData.contains("intensity"))
			{
				intensity = componentData["intensity"].get<float>();
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': point light "
					"does not have intensity component, assigning it "
					"default value"
				);
				intensity = 1.0f;
			}
		}
		float attenuation_linear;
		float attenuation_quadratic;
		{
			if (componentData.contains("attenuationlinear"))
			{
				attenuation_linear =
					componentData["attenuationlinear"].get<float>();
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': point light "
					"does not have attenuation component, assigning it "
					"default value"
				);
				attenuation_linear = 0.09f;
			}

			if (componentData.contains("attenuationquadratic"))
			{
				attenuation_quadratic =
					componentData["attenuationquadratic"].get<float>();
			}
			else
			{
				clz::log::warn(
					"entity '" + std::string(entityName) + "': point light "
					"does not have attenuation component, assigning it "
					"default value"
				);
				attenuation_quadratic = 0.032f;
			}
		}

		const auto result = renderer::registerPointLight(
			position,
			range,
			color,
			intensity,
			attenuation_linear,
			attenuation_quadratic
		);
		if (!result)
		{
			const auto error = result.error();
			switch (error)
			{
			case renderer::LightRegisterError::INVALID_PARAMETER:
				clz::log::warn(
					"entity '" + std::string(entityName) + "': point light "
					"entry in JSON has invalid parameters, unless resolved, "
					"this light will be disabled"
				);
				break;

			case renderer::LightRegisterError::LIMIT_EXCEEDED:
				clz::log::warn(
					"entity '" + std::string(entityName) + "': MAX LIMIT "
					"was already reached for point lights, cannot add "
					"further lights"
				);
				break;
			}
			return std::unexpected("Could not register directional light");
		}

		renderer::PointLightComponent pointLightComp;
		pointLightComp.Id = result.value();
		return pointLightComp;
	}

	/// @copydoc savePointLightComponent
	void savePointLightComponent(
		const renderer::PointLightComponent& plc,
		nlohmann::json& componentData,
		std::string_view entityName
	)
	{
		const auto lightId = plc.Id;

		const auto& pos = renderer::getPointLightPosition(lightId);
		componentData["position"] = {pos.x, pos.y, pos.z};
		componentData["range"] = renderer::getPointLightRange(lightId);

		const auto col = renderer::getPointLightColor(lightId);
		componentData["color"] = {col.x, col.y, col.z};
		componentData["intensity"] = renderer::getPointLightIntensity(lightId);

		componentData["attenuationlinear"] =
			renderer::getPointLightLinearAttenuation(lightId);
		componentData["attenuationquadratic"] =
			renderer::getPointLightQuadraticAttenuation(lightId);

		clz::log::info(
			"Saved point light component for entity '" +
			std::string(entityName) + "'"
		);
	}

} // namespace clz::scene
