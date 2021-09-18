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
		* rebind�����������ڽ��������ʹ��ݸ���ͬ��������ʹ��ͬһ�ռ������
		* ���¿��Կ���rebind�������������ã���������������ռ��������
		* template<typename T, typename Alloc>
		* class List_Base
		* {
		* protected:
		*		using node_alloc_type = typename Alloc::template rebind<List_Node<T>>::other;
		*		using t_alloc_type    = typename Alloc::template rebind<T>::other;
		* }
		* 
		* ���粻����rebind������ͻ����±���쳣������
		* template<typename T, typename Alloc1 = alloc<T>, typename Alloc2 = alloc<Node<T>>>
		* class List_Base
		* {
		*		// ... ... 
		* }
		* ����߻���ֱ��ʹ���ڲ�ָ���ķ�����������ʹ���߾Ͳ������ɵظı������
		* template<typename T>
		* class List_Base
		* {
		*		using node_alloc_type = allocator<List_Node<T>>;
		*		uisng t_alloc_type    = allocator<T>;
		* }
		* �ο����ӣ�https://blog.csdn.net/tmhanks/article/details/90740914
		*/
		template<typename U>
		struct rebind
		{
			using other = allocator<U, Alloc>;
		};

		/*
		* ΪʲôSTL��allocatorû��realloc��
		* ��ֱ�ӵ�ԭ���Ƕ���POD���Ϳ���ֱ�ӽ���realloc����C++��������POD���͡�
		* ��Щ������reallocԭ�ط���ʧ�ܻ����µ�ַ����memcpy������POD���Ͳ�����ʹ��memcpy��memcmp������
		* ��һ��˵�����ǣ�
		* reallocֻ����ֵ������ƽ���������������䣩���͵�����²���ʹ�á�
		* �ο����ӣ�https://www.zhihu.com/question/384869006/answer/1130101522
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