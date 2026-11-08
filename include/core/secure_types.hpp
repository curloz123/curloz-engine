#pragma once

#include <cstdint>
#include <limits>
#include "./assert.hpp"

namespace clz
{
	enum class Types
	{
		U32,
		U64 ,
		S32,
		S64,
		F32,
		D64
	};

	template <Types T>
	struct SecureType;

	template<> struct SecureType<Types::U32>
	{
		using type = uint32_t;
	};
	template<> struct SecureType<Types::U64>
	{
		using type = uint64_t;
	};
	template<> struct SecureType<Types::S32>
	{
		using type = int32_t;
	};
	template<> struct SecureType<Types::S64>
	{
		using type = int64_t;
	};
	template<> struct SecureType<Types::F32>
	{
		using type = float;
	};
	template<> struct SecureType<Types::D64>
	{
		using type = double;
	};

	template <Types T>
	using toType_t = SecureType<T>::type;

	template <Types T>
	class SecureType_t
	{
	private:
		toType_t<T> m_value;

	public:
		SecureType_t()
		{
			m_value = std::numeric_limits<toType_t<T>>::max();
		}

		[[nodiscard]] bool hasValue() const
		{
			return
				m_value != std::numeric_limits<toType_t<T>>::max();
		}

		void makeNull()
		{
			m_value = std::numeric_limits<toType_t<T>>::max();
		}

		void operator=(const toType_t<T> val)
		{
			m_value = val;
		}

		[[nodiscard]] toType_t<T> value() const
		{
#ifdef CLZ_ENABLE_CHECKS
			CLZ_ASSERT(
				hasValue(),
				"Attempt to retrieve a null value"
			);
#endif
			return m_value;
		}

	};
}
