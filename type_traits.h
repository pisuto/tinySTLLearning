#pragma once


#include <type_traits> // 可以继续添加

namespace tinySTL {

	// enable_if
	// https://zhuanlan.zhihu.com/p/328534637
	template<bool B, typename T = void>
	struct enable_if {};

	template<typename T>
	struct enable_if<true, T> {
		using type = T;
	};

	template<bool B, typename T = void>
	using enable_if_t = typename enable_if<B, T>::type;

	// remove_reference

	template<typename T>
	struct remove_refernece {
		using type = T;
	};

	template<typename T>
	struct remove_refernece<T&> {
		using type = T;
	};

	template<typename T>
	struct remove_refernece<T&&> {
		using type = T;
	};

	template<typename T>
	using remove_reference_t = typename remove_refernece<T>::type;

	// true_type & false_type

	template <typename T, T v>
	struct m_integral_constant {
		static constexpr T value = v;
	};

	template<bool b>
	using m_bool_constant = m_integral_constant<bool, b>;

	using m_true_type = m_bool_constant<true>;
	using m_false_type = m_bool_constant<false>;

	template <typename T1, typename T2>
	struct pair;

	// is_pair

	template <typename T>
	struct is_pair : m_false_type {};

	template <typename T1, typename T2>
	struct is_pair<pair<T1, T2>> : m_true_type {};

	// is_unbounded_array

	template<typename T>
	struct is_unbounded_array : std::false_type {};

	template<typename T>
	struct is_unbounded_array<T[]> :std::true_type {};

	template<typename T>
	struct is_bounded_array : std::false_type {};

	template<typename T, ::std::size_t N>
	struct is_bounded_array<T[N]> : std::true_type {};

}