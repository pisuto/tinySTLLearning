#pragma once

#include <new>

#include "type_traits.h"
#include "iterator.h"

namespace tinySTL {

	// construct

	template<typename T>
	void construct(T* p) {
		::new((void*)p) T();
	}

	template<typename T1, typename T2>
	void construct(T1* p, const T2& val) {
		::new((void*)p) T1(val);
	}

	template<typename T, typename... Args>
	void construct(T* p, Args&&... args) {
		::new((void*)p) T(tinySTL::forward<Args>(args)...);
	}

	// destroy

	template<typename T>
	void destroy_one(T*, std::true_type) {}

	template<typename T>
	void destroy_one(T* pointer, std::false_type) {
		if (pointer)
			pointer->~T(); // 执行自定义的析构函数，否则不处理
	}

	template<typename ForwardIter>
	void destroy_cat(ForwardIter, ForwardIter, std::true_type) {}

	template<typename ForwardIter>
	void destroy_cat(ForwardIter first, ForwardIter last, std::false_type) {
		for (; first != last; ++first)
			destroy(&*first);
	}

	template<typename T>
	void destroy(T* pointer) {
		destroy_one(pointer, std::is_trivially_destructible<T>{});
	}

	template<typename ForwardIter>
	void destroy(ForwardIter first, ForwardIter last) {
		destroy_cat(first, last, std::is_trivially_destructible<
			typename iterator_traits<ForwardIter>::value_type>{});
	}


}