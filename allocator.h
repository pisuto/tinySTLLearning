#pragma once

#include "alloc.h"

namespace tinySTL {

	template <typename T, typename Alloc = new_alloc<T>>
	class allocator {
	public:
		using value_type       = T;
		using pointer          = T*;
		using const_pointer	   = const T*;
		using reference        = T&;
		using const_reference  = const T&;
		using size_type		   = size_t;
		using difference_type  = ptrdiff_t;

	public:

		static pointer allocate();
		static pointer allocate(size_type n);
		static void deallocate(pointer ptr);
		static void deallocate(pointer ptr, size_type n);

		/*
		* rebind的作用是用于将泛型类型传递给不同的类型且使用同一空间分配器
		* 从下可以看出rebind对于容器的作用，不用再声明多个空间分配器。
		* template<typename T, typename Alloc>
		* class List_Base
		* {
		* protected:
		*		using node_alloc_type = typename Alloc::template rebind<List_Node<T>>::other;
		*		using t_alloc_type    = typename Alloc::template rebind<T>::other;
		* }
		* 
		* 假如不适用rebind，结果就会如下变得异常繁琐：
		* template<typename T, typename Alloc1 = alloc<T>, typename Alloc2 = alloc<Node<T>>>
		* class List_Base
		* {
		*		// ... ... 
		* }
		* 亦或者会变成直接使用内部指定的分配器，这样使用者就不能自由地改变分配器
		* template<typename T>
		* class List_Base
		* {
		*		using node_alloc_type = allocator<List_Node<T>>;
		*		uisng t_alloc_type    = allocator<T>;
		* }
		* 参考链接：https://blog.csdn.net/tmhanks/article/details/90740914
		*/
		template<typename U>
		struct rebind
		{
			using other = allocator<U, Alloc>;
		};

		/*
		* 为什么STL的allocator没有realloc？
		* 最直接的原因是对于POD类型可以直接进行realloc，但C++还有许多非POD类型。
		* 这些类型在realloc原地分配失败会向新地址进行memcpy，但非POD类型并不能使用memcpy和memcmp操作。
		* 换一种说法就是：
		* realloc只有在值类型是平凡拷贝（拷贝不变）类型的情况下才能使用、
		* 参考链接：https://www.zhihu.com/question/384869006/answer/1130101522
		*/
		// void realloc(pointer ptr, size_type n);
	};

	template <typename T, typename Alloc>
	T* allocator<T, Alloc>::allocate() {
		return static_cast<T*>(Alloc::allocate(sizeof(T)));
	}

	template<typename T, typename Alloc>
	T* allocator<T, Alloc>::allocate(size_type n) {
		return static_cast<T*>(Alloc::allocate(sizeof(T) * n));
	}

	template<typename T, typename Alloc>
	void allocator<T, Alloc>::deallocate(pointer ptr) {
		if (!ptr) return;
		Alloc::deallocate(ptr, sizeof(T));
	}

	template<typename T, typename Alloc>
	void allocator<T, Alloc>::deallocate(pointer ptr, size_type n) {
		if (!ptr) return;
		Alloc::deallocate(ptr, sizeof(T) * n);
	}

}