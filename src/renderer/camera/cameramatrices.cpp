/**
 * @file cameramatrices.cpp
 * @author curl0z
 * @brief Camera Matrices implementation file
 */

#include "renderer/camera/cameramatrices.hpp"
#include "math/angle.hpp"
#include "math/mat4x4.hpp"
#include "math/worldtransform.hpp"
#include "renderer/camera/cameradata.hpp"
#include "renderer/vk_types.hpp"
#include "core/logs.hpp"

namespace clz::renderer::camera
{
	math::mat4 getViewMatrix()
	{
		view = makeViewMatrix(Position[activeCamera], Position[activeCamera] + localFront[activeCamera], WorldUp);
		return view.value();
	}
	math::mat4 getProjectionMatrix()
	{
		if (ProjMatrixChanged[activeCamera])
		{
			const auto width = static_cast<float>(r_swapchainContext.extent.width);
			const auto height =  static_cast<float>(r_swapchainContext.extent.height);
			projection = math::makePerspectiveMatrix(
			    Far[activeCamera], Near[activeCamera],
			    width / height, math::radians(Fov[activeCamera]));

			ProjMatrixChanged[activeCamera] = false;
			clz::log::debug("Updating projection matrix");
			return projection.value();
		}

		return projection.value();
	}
} // namespace clz::renderer::camera
