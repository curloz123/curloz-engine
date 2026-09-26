/**
 * @file buffer_player.cpp
 * @author curl0z
 * @brief This file is the one you'd need for all audio playing relatives.
 */

#include <algorithm>
#include "audio/buffer_player.hpp"
#include "audio/source_manager.hpp"
#include "entity/componentmanager.hpp"
#include "entity/entitymanager.hpp"
#include "entity/corecomponents.hpp"
#include "physics/body.hpp"
#include "physics/physicscomponent.hpp"

namespace clz::audio
{
	BufferPlayerId createBufferPlayer(BufferPlayerDef& bufferPlayerDef)
	{
		if (bufferPlayerDef.entt == ecs::NULL_ENTITY)
		{
			clz::log::warn(
				"Null entity passed, "
				"while creating an audio buffer player");
			return BufferPlayerId();
		}

		/// --- Clamp all source def values first --- ///
		bufferPlayerDef.gain = std::clamp(bufferPlayerDef.gain, 0.0f, 1.0f);
		bufferPlayerDef.pitch = std::clamp(bufferPlayerDef.pitch, 0.01f, 4.0f);

		BufferPlayerId bufferPlayerId(au_bufferPlayerGain.size());

		/// --- push back null source id initially --- ///
		au_associatedSources.push_back(SourceId());

		au_associatedEntities.push_back(bufferPlayerDef.entt);
		au_bufferPlayerGain.push_back(bufferPlayerDef.gain);
		au_bufferPlayerPitch.push_back(bufferPlayerDef.pitch);
		au_bufferPlayerLooping.push_back(bufferPlayerDef.looping);
		au_bufferPlayerType.push_back(bufferPlayerDef.playerType);

		return bufferPlayerId;
	}

	/// @copydoc bufferPlayerPlayPos
	void bufferPlayerPlayPos(
		const BufferPlayerId bufferPlayerId, 
		const BufferId bufferId,
		const math::vec3& position,
		const math::vec3& velocity
	)
	{
#ifdef CLZ_ENABLE_CHECKS
		if (au_bufferPlayerType[bufferPlayerId.getId()]
				!= PlayerType::POSITIONAL) [[unlikely]]
		{
			clz::log::warn("Tried to player positional audio"
					", through of buffer player of bg type");
			return;
		}
#endif

		SourceId& associatedSourceId = 
			au_associatedSources[bufferPlayerId.getId()];

#ifdef CLZ_ENABLE_CHECKS
		/// if associated source id is not null, then
		/// most probably this buffer player is already playing
		/// or either is paused or something like that
		/// this means it is present in active buffer players array
		if (associatedSourceId.isNull())
		{

			associatedSourceId = getFreeSourceId();
			/// --- check if id is null still or not --- ///
			if (associatedSourceId.isNull())
			{
				clz::log::warn(
					"Null source id passed"
					", when tried to retrieve a free source"
					", not playing bufferId: " + 
					std::to_string(bufferId.getId())
				);
				return;
			}

			/// in this block id IS null so push it there
			/// --- push back to dense array --- ///
			au_activeBufferPlayers.push_back(bufferPlayerId);

		}
#endif

		sourceSetGain(
			associatedSourceId, 
			au_bufferPlayerGain[bufferPlayerId.getId()]
		);
		sourceSetPitch(
			associatedSourceId,
			au_bufferPlayerPitch[bufferPlayerId.getId()]
		);
		sourceSetLooping(
			associatedSourceId,
			au_bufferPlayerLooping[bufferPlayerId.getId()]
		);
		sourceSetListenerRelative(
			associatedSourceId,
			false
		);
		sourceSetPosition(
			associatedSourceId, 
			position
		);
		sourceSetVelocity(
			associatedSourceId,
			velocity
		);

		sourcePlay(associatedSourceId, bufferId);
	}

	/// @copydoc bufferPlayerPlayBg
	void bufferPlayerPlayBg(
		const BufferPlayerId bufferPlayerId, 
		const BufferId bufferId
	)
	{
#ifdef CLZ_ENABLE_CHECKS
		if (au_bufferPlayerType[bufferPlayerId.getId()]
				!= PlayerType::BACKGROUND) [[unlikely]]
		{
			clz::log::warn("Tried to player backgroundal audio"
					", through of buffer player of pos type");
			return;
		}
#endif

		SourceId& associatedSourceId = 
			au_associatedSources[bufferPlayerId.getId()];

#ifdef CLZ_ENABLE_CHECKS
		/// if associated source id is not null, then
		/// most probably this buffer player is already playing
		/// or either is paused or something like that
		/// this means it is present in active buffer players array
		if (associatedSourceId.isNull())
		{
			associatedSourceId = getFreeSourceId();
			/// --- check if id is null still or not --- ///
			if (associatedSourceId.isNull())
			{
				clz::log::warn(
					"Null source id passed"
					", when tried to retrieve a free source"
					", not playing bufferId: " + 
					std::to_string(bufferId.getId())
				);
				return;
			}

			/// in this block id IS null so push it there
			/// --- push back to dense array --- ///
			au_activeBufferPlayers.push_back(bufferPlayerId);

		}
#endif

		sourceSetGain(
			associatedSourceId, 
			au_bufferPlayerGain[bufferPlayerId.getId()]
		);
		sourceSetPitch(
			associatedSourceId,
			au_bufferPlayerPitch[bufferPlayerId.getId()]
		);
		sourceSetLooping(
			associatedSourceId,
			au_bufferPlayerLooping[bufferPlayerId.getId()]
		);
		sourceSetListenerRelative(
			associatedSourceId,
			true
		);

		sourcePlay(associatedSourceId, bufferId);

	}

	/// @copydoc bufferPlayerStop
	void bufferPlayerStop(
		const BufferPlayerId bufferPlayerId
	)
	{
#ifdef CLZ_ENABLE_CHECKS
		if (bufferPlayerId.isNull())
		{
			clz::log::warn("Tried to stop a null buffer player");
			return;
		}
		if (au_associatedSources[bufferPlayerId.getId()].isNull())
		{
			clz::log::warn("Tried to stop a non-active buffer player");
			return;
		}
#endif
		
		/// --- free source first --- ///
		SourceId associatedSource = 
			au_associatedSources[bufferPlayerId.getId()];
		freeSource(associatedSource);

		/// --- remove this player from busy list --- ///
		std::erase_if(
			au_activeBufferPlayers,
			[bufferPlayerId](const BufferPlayerId bPlayerId)
			{
				return bufferPlayerId.getId() ==
						bPlayerId.getId();
			}
		);
	}

	/// @copydoc updateBufferPlayerData
	void updateBufferPlayerData(const BufferPlayerId bufferPlayerId)
	{
		if (bufferPlayerGetType(bufferPlayerId) != PlayerType::POSITIONAL)
			return;

		SourceId& associatedSourceId =
			au_associatedSources[bufferPlayerId.getId()];
#ifdef CLZ_ENABLE_CHECKS
		if (associatedSourceId.isNull()) [[unlikely]]
		{
			clz::log::warn(
				"Tried to update a buffer player"
				" which is not active right now");
			return;
		}
#endif

		const auto& associatedEntity = bufferPlayerGetAssociatedEntity(bufferPlayerId);
		const auto& transform = ecs::getComponent<ecs::TransformComponent>(
						associatedEntity);
		sourceSetPosition(
			associatedSourceId,
			transform.position
		);
		if (ecs::hasComponent<physics::RigidBodyComponent>(
			associatedEntity
		))
		{
			const auto velocity =
				physics::getBodyVelocity(
					ecs::getComponent<physics::RigidBodyComponent>(
						associatedEntity
					).rigidBodyId
				);

			sourceSetVelocity(
				associatedSourceId,
				velocity
			);
		}
	}


}
