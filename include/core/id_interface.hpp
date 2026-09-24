/**
 * @file id_interface.hpp
 * @author curl0z
 * @brief Provides a generic ID interface for vector lookup in different areas
 */

#pragma once

#include <cstdint>
#include <limits>

namespace clz
{
	///< @brief Generic ID Interface
	template <typename T = std::uint32_t>
	class IdInterface
	{
	private:
		///< @brief NULL ID
		inline static T s_NULL_ID = std::numeric_limits<T>::max();

		///< @brief The actual ID
		T m_Id;
	public:
		/// --- By default, nullify the ID --- ///
		IdInterface() : m_Id(s_NULL_ID) {}
		IdInterface(const T Id) : m_Id(Id) {}

		/// @brief returns the ID
		std::uint32_t getId() const
		{
			return m_Id;
		}

		/// @brief Nullifies the Id
		void nullify()
		{
			m_Id = s_NULL_ID;
		}

		/// @brief returns whether ID is null
		[[nodiscard]] bool isNull() const
		{
			return m_Id == s_NULL_ID;
		}
	};
}
