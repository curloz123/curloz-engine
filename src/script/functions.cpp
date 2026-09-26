/**
 * @file functions.cpp
 * @author curl0z
 * @brief Introduces C++ side functions to LUA
 */

#include "script/functions.hpp"
#include "audio/audio_components.hpp"
#include "core/logs.hpp"
#include "entity/componentmanager.hpp"
#include "script/native.hpp"
#include "entity/entitymanager.hpp"
#include "audio/buffer_manager.hpp"
#include "audio/buffer_player.hpp"
#include <sol/raii.hpp>
#include <type_traits>

namespace clz::script
{
	/// @copydoc registerCoreFunctions
	void registerCoreFunctions()
	{
		/// --- Log functions --- ///
		sol::table log = s_SolHandle.create_table();
		log.set_function(
			"error",
			[](const std::string& err){
				clz::log::error(err);
			}
		);
		log.set_function(
			"warn",
			[](const std::string& warn){
				clz::log::warn(warn);
			}
		);
		log.set_function(
			"debug",
			[](const std::string& dbg){
				clz::log::debug(dbg);
			}
		);
		log.set_function(
			"info",
			[](const std::string& info){
				clz::log::info(info);
			}
		);

		s_SolHandle["log"] = log;

		/// --- math interface --- ///
		sol::table math = s_SolHandle.create_table();
		math.new_usertype<math::vec3>(
			"vec3"
		);
		s_SolHandle["math"] = math;

	}

	/// @copydoc registerEntityFunctions
	void registerEntityFunctions()
	{
		sol::table entt = s_SolHandle.create_table();

		/// --- get entity by name --- ///
		entt.set_function(
			"getEntityByName",
			[](const std::string& name){
				const auto result = ecs::getEntityByName(name);
				if (result)
				{
					return result.value();
				}
				clz::log::warn(
					"Script tried retrieve entity with name: '" + 
					name + 
					"' But that entity doesn't exist");
				return ecs::NULL_ENTITY;
			}
		);
		s_SolHandle["ecs"] = entt;
	}

	/// @copydoc registerAudioInterface
	void registerAudioInterface()
	{
		/// --- general audio namespace --- ///
		sol::table audio = s_SolHandle.create_table();	

		/// --- Buffer related things --- ///
		audio.new_usertype<audio::BufferId>(
			"BufferId",
			sol::constructors<audio::BufferId(), audio::BufferId(std::uint32_t)>(),
			"isNull", &audio::BufferId::isNull,
			"getId", &audio::BufferId::getId
		);
		audio.set_function(
			"getBufferId",
			[](const std::string& audioFile){
				auto bufferId = audio::getBufferIdByFileName(audioFile);	
				if (!bufferId.isNull()) [[likely]]
				{
					return bufferId;
				}
				clz::log::warn(
					"Could not retrieve buffer: " + 
					audioFile + 
					", it is queried by script"
				);
				/// --- return null buffer id --- ///
				return bufferId;
			}
		);

		/// --- Buffer Player related things --- ///
		audio.set_function(
			"playBgAudio",
			[](
				const ecs::entity entt, 
				const audio::BufferId bufferId
			)
			{
				if (ecs::hasComponent<
					audio::AudioBufferPlayerComponent>(
						entt)) [[likely]]
				{
					auto bufferPlayerId = 
						ecs::getComponent<
							audio::AudioBufferPlayerComponent>(
								entt
							).bufferPlayerId;
					audio::bufferPlayerPlayBg(
							bufferPlayerId, 
							bufferId);
				}
				else
				{
					clz::log::warn(
						"Script tried to play buffer using an entity"
						", that does not have 'AudioBufferPlayerComponent'"
						" attached to it"
					);
				}
			}
		);

		audio.set_function(
			"playPosAudio",
			[](
				const ecs::entity entt, 
				const audio::BufferId bufferId,
				const math::vec3 position
			)
			{
				if (ecs::hasComponent<
					audio::AudioBufferPlayerComponent>(
						entt)) [[likely]]
				{
					auto bufferPlayerId = 
						ecs::getComponent<
							audio::AudioBufferPlayerComponent>(
								entt
							).bufferPlayerId;
					audio::bufferPlayerPlayPos(
							bufferPlayerId, 
							bufferId,
							position);
				}
				else
				{
					clz::log::warn(
						"Script tried to play buffer using an entity"
						", that does not have 'AudioBufferPlayerComponent'"
						" attached to it"
					);
				}
			}
		);
		auto registerBufferPlayerFunc = [&audio]<typename... Args>(
			const std::string& funcName,
			auto (*func)(const audio::BufferPlayerId bufferPlayerId, Args...)
		)
		{
			audio.set_function(
				funcName,
				[func](const ecs::entity entt, Args... args){
					if (ecs::hasComponent<
						audio::AudioBufferPlayerComponent>(
							entt)) [[likely]]
					{
						auto bufferPlayerId = 
							ecs::getComponent<
								audio::AudioBufferPlayerComponent>(
									entt
								).bufferPlayerId;
						return (*func)(bufferPlayerId, args...);
					}
					clz::log::warn(
						"Tried to perform a buffer player function"
						", over an entity that doesn't has buffer player component"
					);
					using returnType = std::invoke_result_t<
								decltype(func), 
								audio::BufferPlayerId, Args...>;
					if (!std::is_void<returnType>())
						return returnType{};
				}
			);
		};
		registerBufferPlayerFunc(
			"setGain",
			&audio::bufferPlayerSetGain
		);
		registerBufferPlayerFunc(
			"getGain",
			&audio::bufferPlayerGetGain
		);
		registerBufferPlayerFunc(
			"setPitch",
			&audio::bufferPlayerSetPitch
		);
		registerBufferPlayerFunc(
			"getPitch",
			&audio::bufferPlayerGetPitch
		);
		registerBufferPlayerFunc(
			"setLooping",
			&audio::bufferPlayerSetLooping
		);
		registerBufferPlayerFunc(
			"getLooping",
			&audio::bufferPlayerGetLooping
		);

		s_SolHandle["audio"] = audio;
	}
}
