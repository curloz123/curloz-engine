/**
 * @file cameramatrices.cpp
 * @author curl0z
 * @brief Camera Matrices implementation file
 */

#include "math/angle.hpp"
#include "math/mat4x4.hpp"
#include "math/worldtransform.hpp"
#include "renderer/camera/cameradata.hpp"
#include "renderer/camera/cameramatrices.hpp"
#include "renderer/vk_types.hpp"

namespace clz::renderer::camera
{
	math::mat4 getViewMatrix()
	{
		view = makeViewMatrix(Position[activeCamera], Position[activeCamera] + localFront[activeCamera], WorldUp);
		;
		return view.value();
	}
	math::mat4 getProjectionMatrix()
	{
		if (FovChanged[activeCamera])
		{
			projection = math::makePerspectiveMatrix(
			    100.0f, 1.0f, static_cast<float>(r_swapchainContext.extent.width) / r_swapchainContext.extent.height,
			    math::radians(Fov[activeCamera]));

			FovChanged[activeCamera] = false;
			return projection.value();
		}

		return projection.value();
	}
} // namespace clz::renderer::camera
