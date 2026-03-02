#pragma once

#include <type_traits>

namespace Dodo {

	template <typename... Types>
	struct TypeList
	{
		template <template <typename...> class TT>
		using Apply = TT<Types...>;

		template<typename T>
        static constexpr bool includes() {
            return std::disjunction<std::is_same<T, Types>...>::value;
        }
	};

	template <typename>
	struct AlwaysFalse : std::false_type
	{
	};

} // namespace Dodo
