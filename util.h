#pragma once

// util.h����Ҫ����һЩͨ�ù��ߣ����� move, forward, swap �Ⱥ������Լ� pair ��

#include "type_traits.h"

namespace tinySTL {

	// move
	// �����ȡ�ö�ĳһ����ִ���ƶ���������Ҫ��������Ϊ��������Ϊ��Գ�������ִ�е�
	// �ƶ��������任Ϊ���Ʋ���
	template<typename T>
	remove_reference_t<T>&& move(T&& arg) noexcept {
		return static_cast<remove_reference_t<T>&&>(arg);
	}

	// forward
	// ����ʹ��forward��ʵ������ת���ģ����ܲ���ʱ��Ӧ��д�� void f(T&& x) 
	// { g(std::forward<T>(x)); }������ �� f ���ܵĲ������ͱ�����T&&, g ��
	// �ܵĲ���Ӧ����std::foward<T>(x)
	template<typename T>
	T&& forward(remove_reference_t<T>& arg) noexcept {
		return static_cast<T&&>(arg);
	}

	template<typename T>
	T&& forward(remove_reference_t<T>&& arg) noexcept {
		return static_cast<T&&>(arg);
	}

	// swap

	template<typename T>
	void swap(T& lhs, T& rhs) {
		auto tmp(tinySTL::move(lhs));
		lhs = tinySTL::move(rhs);
		rhs = tinySTL::move(tmp);
	}

	template<typename ForwardIter1, typename ForwardIter2>
	ForwardIter2 swap_range(ForwardIter1 first1, ForwardIter1 last1, ForwardIter2 first2) {
		for (; first1 != last1; ++first1, ++first2) {
			tinySTL::swap(*first1, *first2);
		}
		return first2;
	}

	template <typename Tp, size_t N>
	void swap(Tp(&a)[N], Tp(&b)[N]) {
		tinySTL::swap_range(a, a + N, b);
	}

	/*------------------------------------------------------------------------------------*/
	// pair

	template<typename T1, typename T2>
	struct pair {
		using first_type = T1;
		using second_type = T2;

		first_type  first;
		second_type second;

		// Ĭ�Ϲ��캯��
		template<typename Other1 = T1, typename Other2 = T2,
			typename tinySTL::enable_if<
			std::is_default_constructible<Other1>::value&&
			std::is_default_constructible<Other2>::value, void>::type>
			constexpr pair() : first(), second() {}

		// ��ʽ���캯�� ... why
		template<typename U1 = T1, typename U2 = T2,
			typename tinySTL::enable_if<
			std::is_copy_constructible<U1>::value&&
			std::is_copy_constructible<U2>::value&&
			std::is_convertible<const U1&, T1>::value&&
			std::is_convertible<const U2&, T2>::value, int>::type = 0>
			constexpr pair(const T1& a, const T2& b) :first(a), second(b) {}

		// explicit constructible for this type
		template <class U1 = T1, class U2 = T2,
			typename tinySTL::enable_if<
			std::is_copy_constructible<U1>::value&&
			std::is_copy_constructible<U2>::value &&
			(!std::is_convertible<const U1&, T1>::value ||
				!std::is_convertible<const U2&, T2>::value), int>::type = 0>
			explicit constexpr pair(const U1& a, const U2& b)
			: first(a), second(b) {}

		// ǳ�������ƶ����졢����
		pair(const pair&) = default;
		pair(pair&&) = default;
		~pair() = default;

		// implicit constructiable for other type
		template <class Other1, class Other2,
			typename tinySTL::enable_if<
			std::is_constructible<T1, Other1>::value&&
			std::is_constructible<T2, Other2>::value&&
			std::is_convertible<Other1&&, T1>::value&&
			std::is_convertible<Other2&&, T2>::value, int>::type = 0>
			constexpr pair(Other1&& a, Other2&& b)
			: first(tinySTL::forward<Other1>(a)),
			second(tinySTL::forward<Other2>(b)) {}

		// explicit constructiable for other type
		template <class Other1, class Other2,
			typename tinySTL::enable_if<
			std::is_constructible<T1, Other1>::value&&
			std::is_constructible<T2, Other2>::value &&
			(!std::is_convertible<Other1, T1>::value ||
				!std::is_convertible<Other2, T2>::value), int>::type = 0>
			explicit constexpr pair(Other1&& a, Other2&& b)
			: first(tinySTL::forward<Other1>(a)),
			second(tinySTL::forward<Other2>(b)) {}

		// implicit constructiable for other pair
		template <class Other1, class Other2,
			typename tinySTL::enable_if<
			std::is_constructible<T1, const Other1&>::value&&
			std::is_constructible<T2, const Other2&>::value&&
			std::is_convertible<const Other1&, T1>::value&&
			std::is_convertible<const Other2&, T2>::value, int>::type = 0>
			constexpr pair(const pair<Other1, Other2>& other)
			: first(other.first),
			second(other.second) {}

		// explicit constructiable for other pair
		template <class Other1, class Other2,
			typename tinySTL::enable_if<
			std::is_constructible<T1, const Other1&>::value&&
			std::is_constructible<T2, const Other2&>::value &&
			(!std::is_convertible<const Other1&, T1>::value ||
				!std::is_convertible<const Other2&, T2>::value), int>::type = 0>
			explicit constexpr pair(const pair<Other1, Other2>& other)
			: first(other.first),
			second(other.second) {}

		// implicit constructiable for other pair
		template <class Other1, class Other2,
			typename tinySTL::enable_if<
			std::is_constructible<T1, Other1>::value&&
			std::is_constructible<T2, Other2>::value&&
			std::is_convertible<Other1, T1>::value&&
			std::is_convertible<Other2, T2>::value, int>::type = 0>
			constexpr pair(pair<Other1, Other2>&& other)
			: first(tinySTL::forward<Other1>(other.first)),
			second(tinySTL::forward<Other2>(other.second)) {}

		// explicit constructiable for other pair
		template <class Other1, class Other2,
			typename tinySTL::enable_if<
			std::is_constructible<T1, Other1>::value&&
			std::is_constructible<T2, Other2>::value &&
			(!std::is_convertible<Other1, T1>::value ||
				!std::is_convertible<Other2, T2>::value), int>::type = 0>
			explicit constexpr pair(pair<Other1, Other2>&& other)
			: first(tinySTL::forward<Other1>(other.first)),
			second(tinySTL::forward<Other2>(other.second)) {}

		// operator overload for this pair 

		pair& operator=(const pair& rhs) {
			if (this != &rhs) {
				first = rhs.first;
				second = rhs.second;
			}
			return *this;
		}

		pair& operator=(pair&& rhs) {
			if (this != &rhs) {
				first = tinySTL::move(rhs.first);
				second = tinySTL::move(rhs.second);
			}
			return *this;
		}

		// operator overload for other pair

		template<typename Other1, typename Other2>
		pair& operator=(const pair<Other1, Other2>& other) {
			first = other.first;
			second = other.second;
			return *this;
		}

		// why use forward<>()?
		template<typename Other1, typename Other2>
		pair& operator=(pair<Other1, Other2>&& other) {
			first = tinySTL::forward<Other1>(other.first);
			second = tinySTL::forward<Other2>(other.second);
			return *this;
		}

		void swap(pair& rhs) {
			if (this != &rhs) {
				tinySTL::swap(first, rhs.first);
				tinySTL::swap(second, rhs.second);
			}
		}

		// ���رȽϺ���

		bool operator==(const pair& rhs) {
			return first == rhs.first && second == rhs.second;
		}

		bool operator!=(const pair& rhs) {
			return !(*this == rhs);
		}

		bool operator<(const pair& rhs) {
			return first < rhs.first || (first == rhs.first && second < rhs.second);
		}

		bool operator>(const pair& rhs) {
			return rhs < *this;
		}

		bool operator<=(const pair& rhs) {
			return !(*this > rhs);
		}

		bool operator>=(const pair& rhs) {
			return !(*this < rhs);
		}

	};

	// ����tinySTL��swap

	template<typename T1, typename T2>
	void swap(pair<T1, T2>& lhs, pair<T1, T2>& rhs) {
		lhs.swap(rhs);
	}

	template<typename T1, typename T2>
	pair<T1, T2> make_pair(T1&& first, T2&& second) {
		return pair<T1, T2>(tinySTL::forward<T1>(first), tinySTL::forward<T2>(second));
	}
}