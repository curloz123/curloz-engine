/**
 * @file source_manager.hpp
 * @author curl0z
 * @brief Manages all audio source related functions
 */

#pragma once

#include "core/id_interface.hpp"
#include "native.hpp"
#include "core/logs.hpp"
#include <cstdint>
#include <vector>
#include "math/vec3.hpp"
#include "buffer_manager.hpp"

namespace clz::audio
{
	///< @brief Distance Model identifier
	enum DistanceModel
	{
		NONE = AL_NONE,
		INVERSE = AL_INVERSE_DISTANCE,
		INVERSE_CLAMPED = AL_INVERSE_DISTANCE_CLAMPED,
		LINEAR = AL_LINEAR_DISTANCE,
		LINEAR_CLAMPED = AL_LINEAR_DISTANCE_CLAMPED,
		EXPONENT = AL_EXPONENT_DISTANCE,
		EXPONENT_CLAMPED = AL_EXPONENT_DISTANCE_CLAMPED
	};

	///< @brief Global audio distance model.
	inline DistanceModel au_distanceModel;	

	/// @brief Sets the global distance model.
	/// @param distanceModel The new distance model to set.
	inline void setDistanceModel(DistanceModel distanceModel)
	{
		alDistanceModel(static_cast<ALenum>(distanceModel));
	}

	/// @brief Retrieves the current Distance model.
	inline DistanceModel getDistanceModel()
	{
		return au_distanceModel;
	}


	///< @brief Declares maximum number of audio sources.
	inline constexpr std::uint8_t MAX_AUDIO_SOURCES = 128;

	///< @brief Global Sources of array.
	inline std::array<ALuint, MAX_AUDIO_SOURCES> au_sourcesLUT;
	///< @brief SourceId -> Indices the sources lut above.
	///< @note It does make sense to use uint8_t here, 
	///< because max audio sources are 128. But if it was 256,
	///< then IdInterface will define maxof(uint8_t) as null,
	///< and we dont want that now do we.
	class SourceId : public IdInterface<std::uint8_t>{};
	///< @brief Defines which sources are free, by index.
	inline std::vector<uint8_t> au_freeSources;	


	/// @brief Initializes all sources and all related data.
	inline void generateAllSources()
	{
		/// --- generate sources --- ///
		alGenSources(
			au_sourcesLUT.size(), 
			au_sourcesLUT.data()
		);

		/// --- reserve memory for free, and mark all free at beginning --- ///
		au_freeSources.reserve(MAX_AUDIO_SOURCES);
		for (std::uint8_t i = 0; i < MAX_AUDIO_SOURCES; ++i)
		{
			au_freeSources.push_back(i);
		}
	}


	/**
	 * @brief Retrieves Id of a free source.
	 * @return SourceId of a free source.
	 * @note In case if there are no free sources,
	 * engine will warn about it, and return null ID.
	 */
	inline SourceId getFreeSourceId()
	{
		/// --- check if free sources are present --- ///
		if (!au_freeSources.empty()) [[likely]]
		{
			const auto freeIndex = au_freeSources.back();

			/// --- remove it from free sources --- ///
			au_freeSources.pop_back();

			return SourceId(freeIndex);
		}

		clz::log::warn(
			"MAXIMUM AUDIO SOURCES LIMIT REACHED. " 
			"You might notice some audio missing. "
			"Either free some sources, "
			"or increase maximum limit "
		);
		/// return null id
		return SourceId();
	}

	/**
	 * @brief Frees a source.
	 * @param sourceId ID of source.
	 * @note Make sure to make your sourceId null after calling this.
	 */
	inline void freeSource(const SourceId sourceId)
	{
		if (sourceId.isNull()) [[unlikely]]
		{
			clz::log::error("While freeing an audio source, null ID was passed");
			return;
		}
		
		/// --- pass this to free sources list --- ///
		const auto freeIndex = sourceId.getId();
		au_freeSources.push_back(freeIndex);
	}


	/// @brief Sets Position of source.
	/// @param sourceId Id of source whose data needs be changed.
	/// @param newVelocity New position of source.
	/// @note Must be updated each frame.
	inline void sourceSetPosition(
		const SourceId sourceId, 
		const math::vec3& newPosition)
	{
		alSource3f(
			au_sourcesLUT[sourceId.getId()], 
			AL_POSITION, 
			newPosition.x,
			newPosition.y,
			newPosition.z
		);
	}
	/// @brief Sets Velotity of source.
	/// @param sourceId Id of source whose data needs be changed.
	/// @param newVelocity New velocity of source.
	/// @note Must be updated each frame.
	inline void sourceSetVelocity(
		const SourceId sourceId, 
		const math::vec3& newVelocity)
	{
		alSource3f(
			au_sourcesLUT[sourceId.getId()], 
			AL_VELOCITY, 
			newVelocity.x,
			newVelocity.y,
			newVelocity.z
		);
	}

	/// @brief Plays a buffer from a source.
	/// @param sourceId Id of source through which buffer will be played.
	/// @param bufferId Id of buffer which will be played.
	/// @param If a sound is being played on the source, it will be stopped in favor of this buffer.
	inline void sourcePlay(const SourceId sourceId, const BufferId bufferId)
	{
		if (bufferId.isNull()) [[unlikely]]
		{
			clz::log::warn("Tried to play a null buffer");
			return;
		}

		const ALuint source = au_sourcesLUT[sourceId.getId()];
		alSourceStop(source);
		alSourcei(source, AL_BUFFER, au_bufferLUT[bufferId.getId()]);
		alSourcePlay(source);
	}

	/// @brief Stops a source
	/// @param sourceId Id of source to act on.
	inline void sourceStop(const SourceId sourceId)
	{
		alSourceStop(au_sourcesLUT[sourceId.getId()]);
	}

	/// @brief Queries whether a buffer is playing right now.
	/// @param sourceId Id of source which is being queried.
	/// @return true if yes, false otherwise.
	inline bool isSourceBusy(const SourceId sourceId)
	{
		ALint state;
		alGetSourcei(
			au_sourcesLUT[sourceId.getId()], 
			AL_SOURCE_STATE, 
			&state
		);
		return state == AL_PLAYING;
	}

	/// @brief Pauses a source.
	/// @param sourceId Id of source to act on.
	inline void sourcePause(const SourceId sourceId)
	{
		alSourcePause(au_sourcesLUT[sourceId.getId()]);
	}

	/// @brief Resumes playback of a paused source.
	/// @param sourceId Id of source to act on.
	inline void continuePlaying(const SourceId sourceId)
	{
		alSourcePlay(au_sourcesLUT[sourceId.getId()]);
	}


	/// @brief Sets the gain (volume) value for the given audio source.
	/// @param sourceId Id of source whose gain to set.
	/// @param value The new gain value.
	inline void sourceSetGain(const SourceId sourceId, const float value)
	{
	    	alSourcef(au_sourcesLUT[sourceId.getId()], AL_GAIN, value);
	}

	/// @brief Sets the pitch value for the given audio source.
	/// @param sourceId Id of source whose pitch to set.
	/// @param value The new pitch value.
	inline void sourceSetPitch(const SourceId sourceId, const float value)
	{
	    	alSourcef(au_sourcesLUT[sourceId.getId()], AL_PITCH, value);
	}

	/// @brief Sets whether the given audio source should loop.
	/// @param sourceId Id of source to modify.
	/// @param value True to enable looping, false to disable.
	inline void sourceSetLooping(const SourceId sourceId, const bool value)
	{
		alSourcei(
			au_sourcesLUT[sourceId.getId()], 
			AL_LOOPING, value ? AL_TRUE : AL_FALSE
		);
	}
}
