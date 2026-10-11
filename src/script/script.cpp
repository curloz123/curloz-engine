/**
 * @file script.cpp
 * @author curl0z
 * @brief Script system public header implementation
 */

#include "script/script.hpp"
#include "script/native.hpp"
#include "script/interface.hpp"
#include "core/logs.hpp"

namespace clz::script
{
	/// @copydoc init()
	bool init()
	{
		/// --- Initializ Sol --- ///
		if (!initializeSol())
		{
			clz::log::error("Could not initialize Sol");
			clz::log::error("Could not initialize script system");
			return false;

		}
		
		/// --- Initialize core interface --- ///
		registerCoreInterface();

		/// --- Initialize entity related interface --- ///
		registerEntityInterface();

		/// --- Initialize audio related interface --- ///
		registerAudioInterface();

		return true;
	}

	/// @copydoc shutdown()
	void shutdown()
	{

	}
}
