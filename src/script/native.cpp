#include "script/native.hpp"

namespace clz::script
{
	bool initializeSol()
	{
		s_SolHandle.open_libraries(
			sol::lib::base,
			sol::lib::math,
			sol::lib::os
		);

		return true;
	}
}
