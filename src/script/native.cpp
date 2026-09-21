/**
 * @file native.cpp
 * @author curl0z
 * @brief SOL's(third_party) implementation
 */

#include "script/native.hpp"

namespace clz::script
{
	/// @copydoc initializeSol()
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
