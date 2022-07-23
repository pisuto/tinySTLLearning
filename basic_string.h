#pragma once

// 字符串类型

#include <iostream>
#include <cassert>

#include "algobase.h"
#include "iterator.h"
#include "memory.h"
#include "functional.h"
#include "allocator.h"
#include "uninitialized.h"


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
		// 构造
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
			copy_init(first, last, tinySTL::iterator_category(first));
		}

		// 拷贝
		basic_string(const basic_string& rhs)
			:buffer_(nullptr), size_(0), cap_(0)
		{
			// 这里直接使用iterator类型的buffer_传入形参const_pointer其实并不准确
			init_from(rhs.buffer_, 0, rhs.size_);
		}

		basic_string(basic_string&& rhs) noexcept
			:buffer_(rhs.buffer_), size_(rhs.size_), cap_(rhs.cap_)
		{
			rhs.buffer_ = nullptr;
			rhs.size_ = 0;
			rhs.cap_ = 0;
		}

		// 析构
		~basic_string() { destroy_buffer(); }

	public:
		// 操作符重载

		basic_string& operator=(const basic_string& rhs)
		{
			if (&rhs != this)
			{
				basic_string tmp(rhs);
				swap(tmp);
			}
			return *this;
		}

		basic_string& operator=(basic_string&& rhs) noexcept
		{
			destroy_buffer();
			buffer_ = rhs.buffer_;
			size_ = rhs.size_;
			cap_ = rhs.cap_;
			
			rhs.buffer_ = nullptr;
			rhs.size_ = 0;
			rhs.cap_ = 0;
			return *this;
		}

		basic_string& operator=(const_pointer str)
		{
			// 使用一个局部变量构造再交换
			basic_string tmp(str);
			swap(tmp);
			return *this;
		}

		basic_string& operator=(value_type ch)
		{
			basic_string tmp(1, ch);
			swap(tmp);
			return *this;
		}

		basic_string& operator+=(const basic_string& str)
		{
			return append(str);
		}

	public:

		// 迭代器相关操作
		iterator begin() noexcept
		{
			return buffer_;
		}

		const_iterator begin() const noexcept
		{
			return buffer_;
		}

		iterator end() noexcept
		{
			return buffer_ + size_;
		}

		const_iterator end() const noexcept
		{
			return buffer_ + size_;
		}

		reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end());
		}

		const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}

		const_reverse_iterator rend() noexcept
		{
			return const_reverse_iterator(begin());
		}

		const_iterator cbegin() const noexcept
		{
			return begin();
		}

		const_iterator cend() const noexcept
		{
			return end();
		}

		const_reverse_iterator crbegin() const noexcept
		{
			return rbegin();
		}

		const_reverse_iterator crend()   const noexcept
		{
			return rend();
		}

	public:

		// 容器相关操作
		bool empty() const noexcept
		{
			return size_ == 0;
		}

		size_type size() const noexcept
		{
			return size_;
		}

		size_type length() const noexcept
		{
			return size_;
		}

		size_type capacity() const noexcept
		{
			return cap_;
		}

		size_type max_size() const noexcept
		{
			return static_cast<size_type>(-1);
		}

		void reserve(size_type cnt)
		{
			if (cap_ < cnt)
			{
				// 不使用reallocate的原因是reallocate会自己选择扩展空间
				assert(cnt < max_size());
				auto new_buffer = data_allocator::allocate(cnt);
				char_traits::move(new_buffer, buffer_, size_);
				data_allocator::deallocate(buffer_);
				buffer_ = new_buffer;
				cap_ = cnt;
			}
		}

		void shrink_to_fit()
		{
			if (size_ != cap_)
			{
				reinsert(size_);
			}
		}

		// insert
		iterator insert(const_iterator pos, value_type ch)
		{
			auto p = const_cast<iterator>(pos);
			if (size_ == cap_)
			{
				return reallocate_and_fill(p, 1, ch);
			}
			char_traits::move(p + 1, p, end() - p);
			++size_;
			*p = ch;
			return p;
		}

		iterator insert(const_iterator pos, size_type cnt, value_type ch)
		{
			auto p = const_cast<iterator>(pos);
			if (cnt == 0)
				return p;
			if (cap_ - size_ < cnt)
			{
				return reallocate_and_fill(p, cnt, ch);
			}
			if (pos == end())
			{
				char_traits::move(end(), ch, cnt);
				size_ += cnt;
				return p;
			}
			char_traits::move(p + cnt, p, cnt);
			char_traits::fill(p, ch, cnt);
			size_ += cnt;
			return p;
		}

		template<typename Iter>
		iterator insert(const_iterator pos, Iter first, Iter last)
		{
			auto p = const_cast<iterator>(pos);
			const size_type len = tinySTL::distance(first, last);
			if (!len) return p;
			if (cap_ - size_ < len)
			{
				return reallocate_and_copy(pos, first, last);
			}
			if (pos == end())
			{
				tinySTL::uninitialized_copy(first, last, end());
				size_ += len;
				return pos;
			}
			char_traits::move(pos + len, pos, len);
			tinySTL::uninitialized_copy(first, last, pos);
			size_ += len;
			return pos;
		}

		// append 重载
		basic_string& append(size_type cnt, value_type ch)
		{
			assert(max_size() > size_ + cnt);
			auto pos = buffer_ + size_;
			if (pos + cnt > cap_)
			{
				reallocate(cnt); // 重新分配空间，注意这个和C中的realloc实现不一样
			}
			while (cnt--)
			{
				*pos++ = ch;
			}
			return *this;
		}

		basic_string& append(const_pointer s, size_type cnt)
		{
			assert(max_size() > size_ + cnt);
			auto pos = buffer_ + size_;
			if(size_ + cnt > cap_)
			{
				reallocate(cnt);
			}
			char_traits::copy(pos, s, cnt);
			size_ += cnt;
			return *this;
		}

		basic_string& append(const_pointer s)
		{
			return append(s, char_traits::length(s));
		}

		basic_string& append(basic_string& str, size_type pos, size_type cnt)
		{
			assert(size_ + cnt < max_size());
			if (cnt == 0)
				return *this;
			if (size_ + cnt > cap_)
			{
				reallocate(cnt);
			}
			char_traits::copy(buffer_ + size_, str.buffer_ + pos, cnt);
			size_ += cnt;
			return *this;
		}

		basic_string& append(const basic_string& str)
		{
			return append(str, 0, str.size_);
		}

		basic_string& append(const basic_string& str, size_type pos)
		{
			return append(str, pos, str.size_ - pos);
		}

		template<typename Iter, typename tinySTL::enable_if_t<
			tinySTL::is_input_iterator<Iter>::value, int> = 0>
		basic_string& append(Iter first, Iter last)
		{
			return append_range(first, last);
		}

		// push_back realated
		void push_back(value_type ch)
		{
			append(1, ch);
		}

		void pop_back()
		{
			assert(!empty());
			// 这里并没有析构末尾元素
			/// todo
			--size_;
		}

		// swap
		void swap(basic_string& rhs) noexcept
		{
			if (this != &rhs)
			{
				tinySTL::swap(buffer_, rhs.buffer_);
				tinySTL::swap(size_, rhs.size_);
				tinySTL::swap(cap_, rhs.cap_);
			}
		}

		// 访问元素相关操作
		reference operator[](size_type idx)
		{
			assert(idx <= size_);
			if (idx == size_)
				*(buffer_ + idx) = value_type();
			return *(buffer_ + idx);
		}

		const_reference operator[](size_type idx) const
		{
			assert(idx <= size_);
			if (idx == size_)
				*(buffer_ + idx) = value_type();
			return *(buffer_ + idx);
		}

		reference at(size_type idx)
		{
			return (*this)[idx];
		}

		const_reference at(size_type idx)
		{
			return (*this)[idx];
		}

		reference front()
		{
			assert(!empty());
			return *begin();
		}

		const_refernece front() const
		{
			assert(!empty());
			return *begin();
		}

		reference back()
		{
			assert(!empty());
			return *(end() - 1);
		}

		const_reference back() const
		{
			assert(!empty());
			return *(end() - 1);
		}

		const_pointer data() const noexcept
		{
			return to_raw_pointer();
		}

		const_pointer c_str() const noexcept
		{
			return to_raw_pointer();
		}

		// erase/clear
		iterator erase(const_iterator pos)
		{
			assert(pos != end());
			auto p = const_cast<iterator>(pos);
			char_traits::move(pos, pos + 1, end() - pos - 1);
			--size_;
			return p;
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			assert(first != end());
			if (first == begin() && last == end())
			{
				clear();
				return end();
			}
			auto p = const_cast<iterator>(first);
			// 是否需要处理超过end()的部分
			if (end() < last)
			{
				size_ -= (end() - first);
				return p;
			}
			// 处理之内的部分
			const auto n = end() - last;
			char_traits::move(p, last, n);
			size_ -= (last - first);
			return p;
		}

		void clear() noexcept
		{
			size_ = 0;
		}

		// resize
		void resize(size_type cnt, value_type ch)
		{
			if (cnt < size_)
			{
				erase(buffer_ + cnt, end());
			}
			else
			{
				append(cnt - size_, ch);
			}
		}

		void resize(size_type cnt)
		{
			resize(cnt, value_type());
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
		
		// reallocate
		void reallocate(size_type need)
		{
			const auto new_cap = tinySTL::max(cap_ + need, cap_ + cap_ >> 1);
			auto new_buffer = data_allocator::allocate(new_cap);
			char_traits::move(new_buffer, buffer_, size_);
			data_allocator::deallocate(buffer_);
			buffer_ = new_buffer;
			cap_ = new_cap;
		}

		typename iterator reallocate_and_fill(iterator pos, size_type n, value_type ch)
		{
			const auto residue = pos - buffer_;
			const auto old_cap = cap_;
			const auto new_cap = tinySTL::max(cap_ + n, cap_ + cap_ >> 1);
			auto new_buffer = data_allocator::allocate(new_cap);
			// 重新分配内存并在原有的字符串中间插入n个ch字符
			auto p1 = char_traits::move(new_buffer, buffer_, residue) + residue;
			auto p2 = char_traits::fill(p1, ch, n) + n;
			char_traits::move(p2, pos, size_ - residue);
			data_allocator::deallocate(buffer_, old_cap);
			buffer_ = new_buffer;
			size_ += n;
			cap_ = new_cap;
			return buffer_ + residue;
		}

		typename iterator reallocate_and_copy(iterator pos, const_iterator first, const_iterator last)
		{
			const auto residue = pos - buffer_;
			const auto old_cap = cap_;
			const auto distance = tinySTL::distance(first, last);
			const auto new_cap = tinySTL::max(cap_ + distance, cap_ + cap_ >> 1);
			auto new_buffer = data_allocator::allocate(new_cap);
			// 类似于reallocate_and_fill
			auto p1 = char_traits::move(new_buffer, buffer_, residue) + residue;
			auto p2 = tinySTL::uninitialized_copy_n(first, distance, p1) + distance;
			char_traits::move(p2, pos, size_ - residue);
			data_allocator::deallocate(buffer_, old_cap);
			buffer_ = new_buffer;
			size_ += n;
			cap_ = new_cap;
			return buffer_ + residue;
		}

		// copy init
		template<typename Iter>
		void copy_init(Iter first, Iter last, tinySTL::input_iterator_tag)
		{
			const auto distance = tinySTL::distance(first, last);
			const auto init_size = tinySTL::max(static_cast<size_type>(STRING_INIT_SIZE), distance + 1);
			try
			{
				buffer_ = data_allocator::allocate(init_size);
				size_ = distance;
				cap_ init_size;
			}
			catch (...)
			{
				buffer_ = nullptr;
				size_ = 0;
				cap_ = 0;
				throw;
			}
			while (distance--)
			{
				append(*first++);
			}
		}

		template<typename Iter>
		void copy_init(Iter first, Iter last, tinySTL::forward_iterator_tag)
		{
			const auto distance = tinySTL::distance(first, last);
			const auto init_size = tinySTL::max(static_cast<size_type>(STRING_INIT_SIZE), distance + 1);
			try
			{
				buffer_ = data_allocator::allocate(init_size);
				size_ = distance;
				cap_ init_size;
				tinySTL::uninitialized_copy(first, last, buffer_);
			}
			catch (...)
			{
				buffer_ = nullptr;
				size_ = 0;
				cap_ = 0;
				throw;
			}
		}

		// append_range
		template<typename Iter>
		basic_string& append_range(Iter first, Iter last)
		{
			const auto distance = tinySTL::distance(first, last);
			assert(distance + size_ < max_size());
			if (size_ + distance > cap_)
			{
				reallocate(distance);
			}
			tinySTL::uninitialized_copy_n(first, distance, buffer_ + size_);
			size_ += distance;
			return *this;
		}

		// reinsert
		void reinsert(size_type size)
		{
			auto new_buffer = data_allocator::allocate(size);
			try
			{
				char_traits::move(new_buffer, buffer_, size);
			}
			catch (...)
			{
				data_allocator::deallocate(new_buffer);
			}
			data_allocator::deallocate(buffer_);
			buffer_ = new_buffer;
			size_ = size;
			cap_ = size;
		}

		// destory
		void destroy_buffer()
		{
			if (buffer_)
			{
				data_allocator::deallocate(buffer_, cap_);
				buffer_ = nullptr;
				size_ = 0;
				cap_ = 0;
			}
		}

		// to_raw_pointer
		const_pointer to_raw_pointer()
		{
			*(buffer_ + size_) = value_type();
			return buffer_;
		}
	};

	// 重载全局的swap
	template<typename CharType, typename CharTraits>
	void swap(basic_string<CharType, CharTraits>& lhs, basic_string<CharType, CharTraits>& rhs)
	{
		lhs.swap(rhs);
	}
}