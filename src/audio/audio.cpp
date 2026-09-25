/**
 * @file audio.cpp
 * @author curl0z
 * @brief Audio system public header implementation
 */

#include "audio/audio.hpp"
#include "core/logs.hpp"
#include "audio/buffer_manager.hpp"
#include "core/enginestate.hpp"
#include "audio/source_manager.hpp"
#include "audio/listener_manager.hpp"
#include "audio/buffer_player.hpp"


namespace clz::audio
{
	/// @copydoc init
	bool init()
	{
		if (!initializeOpenAL())
		{
			clz::log::error("Could not initialize OpenAL");
			return false;
		}

		/// --- generate all sources and related data --- ///
		generateAllSources();
		

		clz::log::info("Initialized audio system");
		return true;
	}

	/// @copydoc update
	void update()
	{
#ifdef CLZ_ENABLE_EDITOR
		if (state::g_engineState == state::EngineState::Editor)
		{
			return;
		}
#endif
		/// --- update listener data --- ///
		updateListenerData();

		/// --- Remove all inactive/completed buffer players --- ///
		std::erase_if(
			au_activeBufferPlayers,
			[](BufferPlayerId& bufferPlayerId)
			{
				auto& associatedSourceId = 
					au_associatedSources[bufferPlayerId.getId()];	
				if (!isSourceBusy(associatedSourceId))
				{
					freeSource(associatedSourceId);
					associatedSourceId.nullify();
					return true;
				}
				return false;
			}
		);
		/// --- Update all 'Active' buffer players --- ///
		for (const auto& bufferPlayerId : au_activeBufferPlayers)
		{
			updateBufferPlayerData(bufferPlayerId);
		}
	}

	/// @copydoc shutdown
	void shutdown()
	{
		/// --- stop all buffer players --- ///
		bufferPlayerStopAll();
		/// --- delete all buffers --- ///
		deleteAllBuffers();

		closeOpenAL();
		clz::log::info("Shut down audio system");
	}
} // namespace clz::audio
