/**
 * @file bufferplayer_component.cpp
 * @author curl0z
 * @brief Manages buffer player properties of entity 
 * in inspector
 */

#include "include/inspector/bufferplayer_component.hpp"
#include "audio/audio_components.hpp"
#include "audio/buffer_player.hpp"
#include "entity/componentmanager.hpp"
#include "entity/entitymanager.hpp"
#include "imgui.h"
#include "include/timemachine.hpp"
#include "include/inspector/inspector.hpp"
#include "include/scenetable.hpp"

namespace clz::editor
{
	template <typename T>
	struct AudioData
	{
		T value;
		T oldValue;
		bool changed;
	};

	/// @copydoc showBufferPlayerHeader
	void showBufferPlayerHeader()
	{
		if (!ImGui::CollapsingHeader(" Buffer Player"))
			return;

		ecs::entity entt = currentSelectedEntity.value();
		const auto& bufferPlayer = ecs::getComponent<audio::AudioBufferPlayerComponent>(
						entt);

		bool looping = audio::bufferPlayerGetLooping(bufferPlayer.bufferPlayerId);
		bool oldL = looping;
		if (ImGui::Checkbox("Looping", &looping))
		{
			audio::bufferPlayerSetLooping(bufferPlayer.bufferPlayerId, looping);

			timemachine::createSnapshot(
				[entt, oldL]{
					if (!ecs::hasComponent<
						audio::AudioBufferPlayerComponent>(
							entt)) [[unlikely]]
					{
						return;
					}					
					const auto& bufferPlayer = ecs::getComponent<
						audio::AudioBufferPlayerComponent>(
							currentSelectedEntity.value()
						);
					audio::bufferPlayerSetLooping(bufferPlayer.bufferPlayerId, oldL);
				},
				[entt, looping]{
					if (!ecs::hasComponent<
						audio::AudioBufferPlayerComponent>(
							entt)) [[unlikely]]
					{
						return;
					}					
					const auto& bufferPlayer = ecs::getComponent<
						audio::AudioBufferPlayerComponent>(
							currentSelectedEntity.value()
						);
					audio::bufferPlayerSetLooping(bufferPlayer.bufferPlayerId, looping);
				}
			);
		}

		ImGui::Text("Type: ");
		ImGui::SameLine();
		auto type = audio::bufferPlayerGetType(bufferPlayer.bufferPlayerId);
		switch (type)
		{
			case audio::PlayerType::BACKGROUND:
				if (ImGui::Button("Background"))
					ImGui::OpenPopup("Player Type");
				break;
			case audio::PlayerType::POSITIONAL:
				if (ImGui::Button("Positional"))
					ImGui::OpenPopup("Player Type");
				break;
		}
		if (ImGui::BeginPopup("Player Type"))
		{
			const auto oldType = audio::bufferPlayerGetType(bufferPlayer.bufferPlayerId);
			auto createTypeSnapshot = [entt, oldType](
				const audio::PlayerType newType
			)
			{
				timemachine::createSnapshot(
					[entt, oldType]{
						if (!ecs::hasComponent<
							audio::AudioBufferPlayerComponent>(
								entt)) [[unlikely]]
						{
							return;
						}					
						const auto& bufferPlayer = ecs::getComponent<
							audio::AudioBufferPlayerComponent>(
								currentSelectedEntity.value()
							);
						audio::bufferPlayerSetType(bufferPlayer.bufferPlayerId, oldType);
					},
					[entt, newType]{
						if (!ecs::hasComponent<
							audio::AudioBufferPlayerComponent>(
								entt)) [[unlikely]]
						{
							return;
						}					
						const auto& bufferPlayer = ecs::getComponent<
							audio::AudioBufferPlayerComponent>(
								currentSelectedEntity.value()
							);
						audio::bufferPlayerSetType(bufferPlayer.bufferPlayerId, newType);
					}
				);
			};

			if (ImGui::MenuItem("Background"))
			{
				createTypeSnapshot(audio::PlayerType::BACKGROUND);
				audio::bufferPlayerSetType(bufferPlayer.bufferPlayerId, audio::PlayerType::BACKGROUND);
			}
			if (ImGui::MenuItem("Positional"))
			{
				createTypeSnapshot(audio::PlayerType::POSITIONAL);
				audio::bufferPlayerSetType(bufferPlayer.bufferPlayerId, audio::PlayerType::POSITIONAL);
			}

			ImGui::EndPopup();
		}


		auto editBufferPlayer = [bufferPlayer, entt](
			const std::string& name,
			auto (*getter)(const audio::BufferPlayerId bufferPlayerId),
			auto (*setter)(const audio::BufferPlayerId bufferPlayerId, float newValue),
			float min, float max
		)
		{
			AudioData<float> data;		
			data.value = (*getter)(bufferPlayer.bufferPlayerId);
			data.oldValue = data.value;

			if (ImGui::SliderFloat(name.c_str(), &data.value, min, max))
				(*setter)(bufferPlayer.bufferPlayerId, data.value);
			if (ImGui::IsItemActivated())
				data.oldValue = (*getter)(bufferPlayer.bufferPlayerId);
			if (ImGui::IsItemDeactivated())
			{
				float oldValue = data.oldValue;
				float newValue = data.value;
				timemachine::createSnapshot(
					[entt, oldValue, setter]{
						if (!ecs::hasComponent<
							audio::AudioBufferPlayerComponent>(
								entt)) [[unlikely]]
						{
							return;
						}

						const auto& bufferPlayer = ecs::getComponent<
							audio::AudioBufferPlayerComponent>(
								currentSelectedEntity.value()
							);
						(*setter)(bufferPlayer.bufferPlayerId, oldValue);
					},
					[entt, newValue, setter]{
						if (!ecs::hasComponent<
							audio::AudioBufferPlayerComponent>(
								entt)) [[unlikely]]
						{
							return;
						}

						const auto& bufferPlayer = ecs::getComponent<
							audio::AudioBufferPlayerComponent>(
								currentSelectedEntity.value()
							);
						(*setter)(bufferPlayer.bufferPlayerId, newValue);
					}
				);

			}
		};

		editBufferPlayer(
			"Gain",
			&audio::bufferPlayerGetGain,
			&audio::bufferPlayerSetGain,
			0.0f, 1.0f
		);
		editBufferPlayer(
			"Pitch",
			&audio::bufferPlayerGetPitch,
			&audio::bufferPlayerSetPitch,
			0.1f, 2.0f
		);


	}
}
