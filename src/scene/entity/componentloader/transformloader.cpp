#include "scene/entity/componentloader/loader.hpp"

namespace clz::ecs
{
	TransformComponent retrieveTransformComponent(const nlohmann::basic_json<>& componentData)
	{
		const clz::math::quat q(componentData["rotation"][0], componentData["rotation"][1], componentData["rotation"][2],
					componentData["rotation"][3]);
		const clz::math::vec3 t(componentData["position"][0], componentData["position"][1], componentData["position"][2]);
		const clz::math::vec3 s(componentData["scale"][0], componentData["scale"][1], componentData["scale"][2]);

		return {q, t, s};
	}

	void saveTransformComponent(const TransformComponent& tc, nlohmann::json& componentData)
	{
		componentData["rotation"][0] = tc.rotation.w;
		componentData["rotation"][1] = tc.rotation.x;
		componentData["rotation"][2] = tc.rotation.y;
		componentData["rotation"][3] = tc.rotation.z;
		componentData["position"][0] = tc.position.x;
		componentData["position"][1] = tc.position.y;
		componentData["position"][2] = tc.position.z;
		componentData["scale"][0] = tc.scale.x;
		componentData["scale"][1] = tc.scale.y;
		componentData["scale"][2] = tc.scale.z;
	}
}
