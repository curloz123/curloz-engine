/**
 * @file buffer_player.hpp
 * @author curl0z
 * @brief This file is the one you'd need for all audio playing relatives.
 */

#pragma once

#include "core/id_interface.hpp"
#include <vector>
#include "math/vec3.hpp"
#include "buffer_manager.hpp"
#include "source_manager.hpp"
#include "entity/entitymanager.hpp"

namespace clz::audio
{
	///< @brief Buffer Player Id -> Indices all fields below.
	class BufferPlayerId : public IdInterface<>{};

	///< @brief Global associated sources to buffer player
	///< In case of none attached, NullId will be present in the index.
	///< Consider it similar to sparse array, which holds all sourceId's
	inline std::vector<SourceId> au_associatedSources;
	///< @brief Holds current active buffers
	///< Consider it like dense array.
	inline std::vector<BufferPlayerId> au_activeBufferPlayers;

	///< @brief Stores to which entity the buffer player is attached to.
	inline std::vector<ecs::entity> au_associatedEntities;
	/// @brief Retrieves entity to which the buffer player is attached to
	/// @param bufferPlayerId Id of buffer player
	/// @return entt The associated entity.
	inline ecs::entity bufferPlayerGetAssociatedEntity(const BufferPlayerId bufferPlayerId)
	{
		return au_associatedEntities[bufferPlayerId.getId()];
	}


	///< @brief Global audio source gain LUT.
	inline std::vector<float> au_bufferPlayerGain;
	///< @brief Global audio source pitch LUT.
	inline std::vector<float> au_bufferPlayerPitch;
	///< @brief Global audio source looping LUT.
	inline std::vector<bool> au_bufferPlayerLooping;

	///< @brief Defines type of buffer player
	///< Background states buffer player will play a background music
	///< POSITIONL states buffer will play a positional audio
	enum class PlayerType
	{
		BACKGROUND,
		POSITIONAL
	};
	///< @brief Global buffer player types LUT.
	inline std::vector<PlayerType> au_bufferPlayerType;


	///< @brief Defines Buffer player creation data.
	struct BufferPlayerDef
	{
		bool looping = false;
		float gain = 1.0f;
		float pitch = 1.0f;
		PlayerType playerType = PlayerType::POSITIONAL;
		ecs::entity entt = ecs::NULL_ENTITY;
	};

	/// @brief Creates a buffer player in audio system.
	/// This is the function that is called once maybe by
	/// external script, or something similar.
	/// @param bufferPlayerDef Source definition data.
	/// @return BufferPlayerId of newly created buffer player.
	/// @note Always check whether id is null or not after creation.
	BufferPlayerId createBufferPlayer(BufferPlayerDef& bufferPlayerDef);

	/// Precisely position and velocity
	/// @param bufferPlayerId Id of buffer player to update
	/// @note To be called only for buffer players who are active right now.
	/// Aka Those who has a source attached to them.
	void updateBufferPlayerData(const BufferPlayerId bufferPlayerId);

	/// @brief Plays a positional buffer through a bufferPlayer
	/// @param bufferPlayerId Id of buffer player.
	/// @param bufferId Id of buffer to play
	/// @param position Position of buffer player
	/// @param velocity Velocity of buffer player(0 by default)
	/// else default param will be used)
	/// @note In the main audio update function you have to update
	/// the position and velocity of sources each frame via bufferPlayerUpdateData
	/// For those who still didnt understand how'd we get position and vel,
	/// check au_components.hpp
	/// @note Will do nothing if buffer player is bg type
	void bufferPlayerPlayPos(
		const BufferPlayerId bufferPlayerId, 
		const BufferId bufferId,
		const math::vec3& position,
		const math::vec3& velocity = math::vec3(0.0f)
	);

	/// @brief Plays a backgroundal buffer through a bufferPlayer
	/// @param bufferPlayerId Id of buffer player.
	/// @param bufferId Id of buffer to play
	/// else default param will be used)
	/// @note In the main audio update function you have to update
	/// the position and velocity of sources each frame via bufferPlayerUpdateData
	/// For those who still didnt understand how'd we get position and vel,
	/// check au_components.hpp
	void bufferPlayerPlayBg(
		const BufferPlayerId bufferPlayerId, 
		const BufferId bufferId
	);


	/// @brief Stops a buffer player
	/// Also frees the associated source too
	/// @param bufferPlayerId Id of buffer player
	/// @note Is technically an O(n) function, 
	/// so don't go gunh-ho and use this each frame
	void bufferPlayerStop(
		const BufferPlayerId bufferPlayerId
	);


	/// @brief Gets the gain (volume) value for the given audio source.
	/// @param id The BufferPlayerId whose gain to retrieve.
	/// @return The current gain value.
	inline float bufferPlayerGetGain(const BufferPlayerId id)
	{
		return au_bufferPlayerGain[id.getId()];
	}

	/// @brief Sets the gain (volume) value for the given audio source.
	/// @param bufferPlayerId The SourceId whose gain to set.
	/// @param value The new gain value.
	inline void bufferPlayerSetGain(
		const BufferPlayerId bufferPlayerId, 
		const float value
	)
	{
		au_bufferPlayerGain[bufferPlayerId.getId()] = value;
		if (!au_associatedSources[bufferPlayerId.getId()].isNull())
		{
			sourceSetGain(
				au_associatedSources[bufferPlayerId.getId()],
				value
			);
		}
	}

	/// @brief Gets the pitch value for the given audio source.
	/// @param id The BufferPlayerId whose pitch to retrieve.
	/// @return The current pitch value.
	inline float bufferPlayerGetPitch(const BufferPlayerId id)
	{
		return au_bufferPlayerPitch[id.getId()];
	}

	/// @brief Sets the pitch value for the given audio source.
	/// @param bufferPlayerId The Id of BufferPlayer whose pitch to set.
	/// @param value The new pitch value.
	inline void bufferPlayerSetPitch(
		const BufferPlayerId bufferPlayerId, 
		const float value)
	{
		au_bufferPlayerPitch[bufferPlayerId.getId()] = value;
		if (!au_associatedSources[bufferPlayerId.getId()].isNull())
		{
			sourceSetPitch(
				au_associatedSources[bufferPlayerId.getId()],
				value
			);
		}

	}

	/// @brief Gets whether the given audio source is set to loop.
	/// @param bufferPlayerId The Id BufferPlayer to query.
	/// @return True if the buffer player is looping, false otherwise.
	inline bool bufferPlayerGetLooping(
		const BufferPlayerId bufferPlayerId)
	{
		return au_bufferPlayerLooping[bufferPlayerId.getId()];
	}

	/// @brief Sets whether the given audio source should loop.
	/// @param bufferPlayerId The Id of BufferPlayer to modify.
	/// @param value True to enable looping, false to disable.
	inline void bufferPlayerSetLooping(
		const BufferPlayerId bufferPlayerId, 
		const bool value)
	{
		au_bufferPlayerLooping[bufferPlayerId.getId()] = value;
		if (!au_associatedSources[bufferPlayerId.getId()].isNull())
		{
			sourceSetLooping(
				au_associatedSources[bufferPlayerId.getId()],
				value
			);
		}

	}

	/// @brief Retrieves buffer player type.
	/// @param bufferPlayerId The Id of BufferPlayer.
	/// @return PlayerType
	inline PlayerType bufferPlayerGetType(
		const BufferPlayerId bufferPlayerId
	)
	{
		return au_bufferPlayerType[bufferPlayerId.getId()];
	}

	/// @brief Sets buffer player type/
	/// @param bufferPlayerId The Id of bufferPlayer.
	/// @param playerType New PlayerType
	inline void bufferPlayerSetType(
		const BufferPlayerId bufferPlayerId,
		const PlayerType playerType
	)
	{
		au_bufferPlayerType[bufferPlayerId.getId()] = playerType;
	}

	/// @brief sets buffer player position
	/// @param bufferPlayerId
	/// @param position position to set
	/// @note does nothing if player type is background
	inline void bufferPlayerSetPosition(
		const BufferPlayerId bufferPlayerId,
		const math::vec3& position
	)
	{
		if (au_bufferPlayerType[bufferPlayerId.getId()]
				!= PlayerType::POSITIONAL) [[unlikely]]
		{
			clz::log::warn("Tried to set position of buffer player of bg type");
			return;
		}

		if (au_associatedSources[
			bufferPlayerId.getId()].isNull()) [[unlikely]]
		{
			clz::log::warn("Tried to set position of a buffer player, "
					" which has no associated source");
			return;
		}

		sourceSetPosition(
			au_associatedSources[bufferPlayerId.getId()],
			position
		);
	}

	/// @brief sets buffer player velocity
	/// @param bufferPlayerId
	/// @param velocity velocity to set
	/// @note does nothing if player type is background
	inline void bufferPlayerSetVelocity(
		const BufferPlayerId bufferPlayerId,
		const math::vec3& velocity
	)
	{
		if (au_bufferPlayerType[bufferPlayerId.getId()]
				!= PlayerType::POSITIONAL) [[unlikely]]
		{
			clz::log::warn("Tried to set velocity of buffer player of bg type");
			return;
		}

		if (au_associatedSources[
			bufferPlayerId.getId()].isNull()) [[unlikely]]
		{
			clz::log::warn("Tried to set velocity of a buffer player, "
					" which has no associated source");
			return;
		}

		sourceSetVelocity(
			au_associatedSources[bufferPlayerId.getId()],
			velocity
		);
	}

	/// @brief Stops all active buffers
	inline void bufferPlayerStopAll()
	{
		std::erase_if(
			au_activeBufferPlayers,
			[](const BufferPlayerId bufferPlayerId)
			{
				auto associatedSource = 
					au_associatedSources[bufferPlayerId.getId()];
				freeSource(associatedSource);
				return true;
			}
		);
	}

}
