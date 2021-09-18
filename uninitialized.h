#pragma once

#include "algobase.h"
#include "construct.h"
#include "iterator.h"
#include "type_traits.h"
#include "util.h"

namespace tinySTL {

	// uninitialize_copy 把[first, last)上的内容复制到以result为起始处的空间，返回复制结束的位置
	template<typename InputIter, typename ForwardIter>
	ForwardIter unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result,
		std::true_type) {
		return tinySTL::copy(first, last, result);
	}

	// &*的原因是函数的形参是T*,而迭代器不是指针不能直接传入，即提领取地址
	template<typename InputIter, typename ForwardIter>
	ForwardIter unchecked_uninit_copy(InputIter first, InputIter last, ForwardIter result,
		std::false_type) {
		auto cur = result;
		try {
			for (; first != last; ++cur, ++first) {
				tinySTL::construct(&*cur, *first);
			}
		}
		catch (...) {
			for (; result != cur; ++result) {
				tinySTL::destroy(&*result);
			}
		}
		return cur;
	}

	template<typename InputIter, typename ForwardIter>
	ForwardIter uninitialized_copy(InputIter first, InputIter last, ForwardIter result) {
		return tinySTL::unchecked_uninit_copy(first, last, result,
			std::is_trivially_copy_assignable<
			typename tinySTL::iterator_traits<ForwardIter>::value_type>{});
	}

	// unubutialized_copy_n 把[first, first + n)上的内容复制到以result为起始处的空间，返回复制结束的位置

	template<typename InputIter, typename Size, typename ForwardIter>
	ForwardIter uninit_copy_n(InputIter first, Size n, ForwardIter result,
		std::true_type) {
		return tinySTL::copy_n(first, n, result).second;
	}

	template<typename InputIter, typename Size, typename ForwardIter>
	ForwardIter unchecked_uninit_copy_n(InputIter first, Size n, ForwardIter result,
		std::false_type) {
		auto cur = result;
		try {
			for (; n > 0; ++cur, ++first, --n) {
				tinySTL::construct(&*cur, *first);
			}
		}
		catch (...) {
			for (; result != cur; ++result) {
				tinySTL::destroy(&*result);
			}
		}
		return cur;
	}

	template<typename InputIter, typename Size, typename ForwardIter>
	ForwardIter uninitialized_copy_n(InputIter first, Size n, ForwardIter result)
	{
		return tinySTL::unchecked_uninit_copy_n(first, n, result,
			std::is_trivially_copy_assignable<
			typename iterator_traits<InputIter>::
			value_type>{});
	}

	// uninitialized_fill 在[first, last)区间内填充元素值

	template<typename ForwardIter, typename T>
	void unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T& value,
		std::true_type) {
		tinySTL::fill(first, last, value);
	}

	template<typename ForwardIter, typename T>
	void unchecked_uninit_fill(ForwardIter first, ForwardIter last, const T& value,
		std::false_type) {
		auto cur = first;
		try {
			for (; cur != last; ++cur) {
				tinySTL::construct(&*cur, value);
			}
		}
		catch (...) {
			for (; first != cur; ++first) {
				tinySTL::destroy(&*first);
			}
		}
	}

	template<typename ForwardIter, typename T>
	void uninitialized_fill(ForwardIter first, ForwardIter last, const T& value) {
		tinySTL::unchecked_uninit_fill(first, last, value,
			std::is_trivially_copy_assignable<typename iterator_traits<ForwardIter>::
			value_type>{});
	}

	// uninitialized_fill_n

	template<typename ForwardIter, typename Size, typename T>
	ForwardIter uncheck_uninit_fill_n(ForwardIter first, Size n, const T& value,
		std::true_type) {
		return tinySTL::fill_n(first, n, value);
	}

	template<typename ForwardIter, typename Size, typename T>
	ForwardIter uncheck_uninit_fill_n(ForwardIter first, Size n, const T& value,
		std::false_type) {
		auto cur = first;
		try {
			for (; n > 0; --n, ++cur) {
				tinySTL::construct(&*cur, value);
			}
		}
		catch (...) {
			for (; first != cur; ++first) {
				tinySTL::destroy(&*first);
			}
		}
		return cur;
	}

	template<typename ForwardIter, typename Size, typename T>
	ForwardIter uninitialized_fill_n(ForwardIter first, Size n, const T& value) {
		return tinySTL::uncheck_uninit_fill_n(first, n, value,
			std::is_trivially_copy_assignable<typename iterator_traits<ForwardIter>::
			value_type>{});
	}

	// uninitialized_move

	template<typename InputIter, typename ForwardIter>
	ForwardIter unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result,
		std::true_type) {
		return tinySTL::move(first, last, result);
	}

	template<typename InputIter, typename ForwardIter>
	ForwardIter unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result,
		std::false_type) {
		auto cur = result;
		try {
			for (; first != last; ++first, ++cur) {
				tinySTL::construct(&*cur, tinySTL::move(*first));
			}
		}
		catch (...) {
			tinySTL::destroy(result, cur); /// why?
		}
		return cur;
	}

	template<typename InputIter, typename ForwardIter>
	ForwardIter unintialized_fill_n(InputIter first, InputIter last, ForwardIter result) {
		return tinySTL::unchecked_uninit_move(first, last, result,
			std::is_trivially_copy_assignable<typename iterator_traits<ForwardIter>::
			value_type>{});
	}

	// uninitialize_move_n

	template<typename InputIter, typename Size, typename ForwardIter>
	ForwardIter unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result,
		std::true_type) {
		return tinySTL::move(first, first + n, result);
	}

	template<typename InputIter, typename Size, typename ForwardIter>
	ForwardIter unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result,
		std::false_type) {
		auto cur = result;
		try {
			for (; n > 0; --n, ++first, ++cur) {
				tinySTL::construct(&*cur, tinySTL::move(*first));
			}
		}
		catch (...) {
			for (; result != cur; ++result) {
				tinySTL::destroy(&*result);
			}
		}
		return cur;
	}

	template <class InputIter, class Size, class ForwardIter>
	ForwardIter uninitialized_move_n(InputIter first, Size n, ForwardIter result)
	{
		return tinySTL::unchecked_uninit_move_n(first, n, result,
			std::is_trivially_move_assignable<
			typename iterator_traits<InputIter>::
			value_type>{});
	}
}