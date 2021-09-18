#pragma once

// 字符串类型

#include <iostream>
#include <cassert>

#include "algobase.h"
#include "iterator.h"
#include "memory.h"
#include "functional.h"
#include "allocator.h"


namespace tinySTL {

	template<typename CharType>
	struct char_traits {
		using char_type = CharType;

		static size_t length(const char_type* str) {
			size_t len = 0;
			while (*str++ != char_type(0))
				len++;
			return len;
		}

		static int compare(const char_type* s1, const char_type* s2,
			size_t n) {
			for (; n != 0; --n, ++s1, ++s2) {
				if (*s1 < *s2)
					return -1;
				if (*s1 > *s2)
					retuen 1;
			}
			return 0;
		}

		static char_type* copy(char_type* dst, const char_type* src, size_t n)
		{
			assert(src + n <= dst || dst + n <= src); // 这里直接使用assert规避
			char_type* rtn = dst;
			while (n--)
			{
				*dst++ = *src++;
			}
			return rtn;
		}

		static char_type* move(char_type* dst, const char_type* src, size_t n)
		{
			char_type* rtn = dst;
			if (dst < src) // 可能会存在覆盖src的问题
			{
				return copy(dst, src, n);
			}
			else if (src < dst) // 可能会存在src后端和dst前段重合的问题
			{
				dst += n;
				src += n;
				while (--n)
				{
					*--dst == *--src;
				}
			}
			return rtn;
		}

		static char_type* fill(char_type* dst, char_type ch, size_t cnt)
		{
			char_type* rtn = dst;
			while (cnt--)
			{
				*dst++ = ch;
			}
			return rtn;
		}
	};


	/* 模板特化，直接封装了原有的库函数 */
	template<>
	struct char_traits<char>
	{
		using char_type = char;

		static size_t length(const char_type* str) noexcept
		{
			return std::strlen(str);
		}

		static int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
		{
			return std::memcmp(s1, s2, n);
		}

		static char_type* copy(char_type* dst, const char_type* src, size_t n) noexcept
		{
			assert(dst + n <= src || src + n <= dst);
			return static_cast<char_type*>(std::memcpy(dst, src, n));
		}

		static char_type* move(char_type* dst, const char_type* src, size_t n) noexcept
		{
			return static_cast<char_type*>(std::memmove(dst, src, n));
		}

		static char_type* fill(char_type* dst, char_type ch, size_t cnt) noexcept
		{
			return static_cast<char_type*>(std::memset(dst, ch, cnt));
		}
	};

	template<>
	struct char_traits<wchar_t>
	{
		using char_type = wchar_t;

		static size_t length(const char_type* str) noexcept
		{
			return std::wcslen(str);
		}

		static int compare(const char_type* s1, const char_type* s2, size_t n) noexcept
		{
			return std::wmemcmp(s1, s2, n);
		}

		static char_type* copy(char_type* dst, const char_type* src, size_t n) noexcept
		{
			assert(dst + n <= src || src + n <= dst);
			return static_cast<char_type*>(std::wmemcpy(dst, src, n));
		}

		static char_type* move(char_type* dst, const char_type* src, size_t n) noexcept
		{
			return static_cast<char_type*>(std::wmemmove(dst, src, n));
		}

		static char_type* fill(char_type* dst, char_type ch, size_t cnt) noexcept
		{
			return static_cast<char_type*>(std::wmemset(dst, ch, cnt));
		}
	};


	template <typename CharType, typename CharTraits = char_traits<CharType>>
	class basic_string
	{
	public:
		using traits_type		= CharTraits;
		using char_traits		= CharTraits;
		
		using allocator_type	= tinySTL::allocator<CharType, tinySTL::new_alloc<CharType>>;
		using data_allocator	= tinySTL::allocator<CharType, tinySTL::new_alloc<CharType>>;

		using value_type		= typename allocator_type::value_type;
		using pointer			= typename allocator_type::pointer;
		using const_pointer		= typename allocator_type::const_pointer;
		using reference			= typename allocator_type::reference;
		using const_reference	= typename allocator_type::const_reference;
		using size_type			= typename allocator_type::size_type;
		using difference_type	= typename allocator_type::difference_type;

		using iterator			= value_type*;
		using const_iterator	= const value_type*;
		using reverse_iterator	= tinySTL::reverse_iterator<iterator>;
		using const_reverse_iterator = tinySTL::reverse_iterator<const_iterator>;
	
		allocator_type get_allocator() { return allocator_type(); }

		// 关于POD类型请参看https://blog.csdn.net/wizardtoh/article/details/80767740
		static_assert(std::is_pod<CharType>::value, "Character type of basic_string must be a POD");
		static_assert(std::is_same<CharType, typename traits_type::char_type>::value,
			"CharType must be same as traits_type::char_type");

	public:
		static constexpr size_type npos = static_cast<size_type>(-1);

	private:
		iterator buffer_; // 迭代器，即所处位置
		size_type size_;  // 字符长度
		size_type cap_;   // 分配空间大小

		constexpr size_type STRING_INIT_SIZE = 32; // 初始化时basic_string的长度

	public:
		// 基本的类成员函数。包括构造、析构、拷贝、移动、操作符重载等

		basic_string() noexcept
		{
			try
			{
				buffer_ = data_allocator::allocate(static_cast<size_type>(STRING_INIT_SIZE));
				size_ = 0;
				cap_ = 0;
			}
			catch (...)
			{
				buffer_ = nullptr;
				size_ = 0;
				cap_ = 0;
			}
		}

		basic_string(size_type n, value_type ch)
			: buffer_(nullptr), size_(0), cap_(0)
		{
			const auto init_size = tinySTL::max(static_cast<size_type>(STRING_INIT_SIZE), n + 1); // +1为保留的'\0'
			buffer_ = data_allocator::allocate(init_size);
			char_traits::fill(buffer_, ch, n);
			size_ = n;
			cap_ = init_size;
		}

		basic_string(const basic_string& other, size_type pos)
			: buffer_(nullptr), size_(0), cap_(0)
		{
			init_from(other.buffer_, pos, other.size_ - pos);
		}

		basic_string(const basic_string& other, size_type pos, size_type cnt)
			:buffer_(nullptr), size_(0), cap_(0)
		{
			init_from(other.buffer_, pos, cnt);
		}

		basic_string(const_pointer str, size_type cnt)
			:buffer_(nullptr), size_(0), cap_(0)
		{
			init_from(str, 0, cnt);
		}

		basic_string(const_pointer str)
			:buffer_(nullptr), size_(0), cap_(0)
		{
			init_from(str, 0, char_traits::length(str));
		}

		template<typename Iter, typename tinySTL::enable_if_t<
			tinySTL::is_input_iterator<Iter>::value, int> = 0>
		basic_string(Iter first, Iter last)
		{

		}


	private:
		// helper func

		void init_from(const_pointer src, size_type pos, size_type cnt)
		{
			const auto init_size = tinySTL::max(static_cast<size_type>(STRING_INIT_SIZE), cnt + 1); // +1为保留的'\0'
			buffer_ = data_allocator::allocate(init_size);
			char_traits::copy(buffer_, src + pos, cnt);
			size_ = cnt;
			cap_ = init_size;
		}

		template<typename Iter>
		void copy_init(Iter first, Iter last, input_iterator_tag)
		{
			size_type n = tinySTL::distance(first, last);
			const auto init_size = tinySTL::max(static_cast<size_type>(STRING_INIT_SIZE), cnt + 1);
			try
			{
				buffer_ = data_allocator::allocate(init_size);
				size_ = n;
				cap_ = init_size;
			}
			catch (...)
			{
				buffer_ = nullptr;
				size_ = 0;
				cap_ = 0;
				throw;
			}

			while (n--)
			{
				append(*first++);
			}
		}

		// append 重载
		basic_string& append(size_type cnt, value_type ch)
		{
			auto pos = buffer_ + size_;
			if (pos + cnt > cap_)
			{
				reallocate(cnt); // 重新分配空间，注意这个和C中的realloc实现不一样
			}
			while (cnt--)
			{
				*pos++ = ch;
			}
		}

		basic_string& append(const_pointer s)
		{

		}
		
		// reallocate
		void reallocate(size_type need)
		{

		}
	};
}