#include "script/script.hpp"
#include "script/native.hpp"
#include "core/logs.hpp"

namespace clz::script
{
	bool init()
	{
		if (!initializeSol())
		{
			clz::log::error("Could not initialize Sol");
			goto failure;	
		}


		return true;

		failure:
		clz::log::error("Could not initialize script system");
		return false;
	}

	bool loadScript(const std::filesystem::path& scriptPath)
	{
		
	}
}
