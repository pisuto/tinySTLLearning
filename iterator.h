#pragma once

#include "type_traits.h"

namespace tinySTL {

	// ���ֵ���������
	struct input_iterator_tag {};
	struct output_iterator_tag {};
	struct forward_iterator_tag : public input_iterator_tag {};
	struct bidirectional_iterator_tag : public forward_iterator_tag {};
	struct random_access_iterator_tag : public bidirectional_iterator_tag {};

	// iterator
	template <typename Category, typename T, typename Distance = ptrdiff_t, typename Pointer = T*,
		typename Reference = T&>
		struct iterator {
		using iterator_category = Category;
		using value_type = T;
		using pointer = Pointer;
		using reference = Reference;
		using difference_type = Distance;
	};

	// �ж��Ƿ���iterator_category
	template <typename T>
	struct has_iterator_category {
	private:
		struct two { char a; char b; };
		template <typename U> static two test(...);
		template <typename U> static char test(typename U::iterator_category* = 0);
	public:
		static const bool value = sizeof(test<T>(0) == sizeof(char));
	};

	template <typename Iterator, bool>
	struct iterator_traits_impl {};

	template <typename Iterator>
	struct iterator_traits_impl<Iterator, true> {
		using iterator_category = typename Iterator::iterator_category;
		using value_type = typename Iterator::value_type;
		using pointer = typename Iterator::pointer;
		using reference = typename Iterator::reference;
		using differece_type = typename Iterator::difference_type;
	};

	template <typename Iterator, bool>
	struct iterator_traits_helper {};

	template <typename Iterator>
	struct iterator_traits_helper<Iterator, true> : public iterator_traits_impl < Iterator,
		std::is_convertible<typename Iterator::iterator_category, input_iterator_tag>::value ||
		std::is_convertible<typename Iterator::iterator_category, output_iterator_tag>::value> {};

	// ��ȡ������
	template <typename Iterator>
	struct iterator_traits : public iterator_traits_helper<Iterator,
		has_iterator_category<Iterator>::value> {};

	// ���ԭ��ָ���ƫ�ػ��汾
	template <typename T>
	struct iterator_traits<T*> {
		using iterator_category = random_access_iterator_tag;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using difference_type = ptrdiff_t;
	};

	template <typename T>
	struct iterator_traits<const T*> {
		using iterator_category = random_access_iterator_tag;
		using value_type = T;
		using pointer = const T*;
		using reference = const T&;
		using difference_type = ptrdiff_t;
	};

	// ��������iterator_categoryʱ�����ж��ǲ���ĳһiterator����
	template <typename T, typename U, bool = has_iterator_category<iterator_traits<T>>::value>
	struct has_iterator_category_of :public m_bool_constant<std::is_convertible<
		typename iterator_traits<T>::iterator_category, U>::value > {};

	// ��������ʱֱ��Ϊ�̳�false_type
	template <typename T, typename U>
	struct has_iterator_category_of<T, U, false> :public m_false_type {};

	template <typename Iter>
	struct is_input_iterator :public has_iterator_category_of<Iter, input_iterator_tag> {};

	template <typename Iter>
	struct is_output_iterator :public has_iterator_category_of<Iter, output_iterator_tag> {};

	template <typename Iter>
	struct is_forward_iterator :public has_iterator_category_of<Iter, forward_iterator_tag> {};

	template <typename Iter>
	struct is_bidirectional_iterator :public has_iterator_category_of<Iter, bidirectional_iterator_tag> {};

	template <typename Iter>
	struct is_random_access_iterator :public has_iterator_category_of<Iter, random_access_iterator_tag> {};

	// �ж��Ƿ��ǵ�����
	template <typename Iterator>
	struct is_iterator : public m_bool_constant<is_input_iterator<Iterator>::value ||
		is_output_iterator<Iterator>::value> {};

	// ����ĳ�������������Category
	template<typename Iterator>
	typename iterator_traits<Iterator>::iterator_category
		iterator_category(const Iterator&) {
		using Category = typename iterator_traits<Iterator>::iterator_category;
		return Category();
	}

	/// fix ... ����������ָ���������ж����ͣ�
	template <typename Iterator>
	typename iterator_traits<Iterator>::value_type*
		value_type(const Iterator&) {
		return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
	}

	// ���º�������������ľ���

	template<typename InputIter>
	typename iterator_traits<InputIter>::difference_type
		distance_dispatch(InputIter first, InputIter last, input_iterator_tag) {
		typename iterator_traits<InputIter>::difference_type n = 0;
		while (first != last) {
			++first; ++n;
		}
		return n;
	}

	template <typename RandomIter>
	typename iterator_traits<RandomIter>::difference_type
		distance_dispatch(RandomIter first, RandomIter last, random_access_iterator_tag) {
		return last - first;
	}

	template <typename InputIter>
	typename iterator_traits<InputIter>::difference_type
		distance(InputIter first, InputIter last) {
		return distance_dispatch(first, last, iterator_category(first));
	}

	// ���º��������õ�����ǰ��n������

	template <typename InputIter, typename Distance>
	void advance_dispatch(InputIter& i, Distance n, input_iterator_tag) {
		while (n--)
			++i;
	}

	template <typename BidirectionalIter, typename Distance>
	void advance_dispatch(BidirectionalIter& i, Distance n, bidirectional_iterator_tag) {
		if (n >= 0)
			while (n--) ++i;
		else
			while (n++) --i;
	}

	template <typename RandomIter, typename Distance>
	void advance_dispatch(RandomIter& i, Distance n, random_access_iterator_tag) {
		i += n;
	}

	template <typename InputIter, typename Distance>
	void advance(InputIter& i, Distance n) {
		advance_dispatch(i, n, iterator_category(i));
	}

	/*==================================================================================*/
	// ���������
	template <typename Iterator>
	class reverse_iterator {
	private:
		Iterator current;

	public:
		using iterator_category = typename iterator_traits<Iterator>::iterator_category;
		using value_type = typename iterator_traits<Iterator>::value_type;
		using difference_type = typename iterator_traits<Iterator>::difference_type;
		using pointer = typename iterator_traits<Iterator>::pointer;
		using reference = typename iterator_traits<Iterator>::reference;

		using iterator_type = Iterator;
		using self = typename reverse_iterator<Iterator>;

	public:
		reverse_iterator() {}
		explicit reverse_iterator(iterator_type i) :current(i) {}
		reverse_iterator(const self& rhs) :current(rhs.current) {}

	public:
		// ȡ����Ӧ�����������
		iterator_type base() const {
			return current;
		}

		// ���ز�����,ʵ�ʶ�Ӧ�����������ǰһ��λ��
		reference operator*() const {
			auto tmp = current;
			return *--tmp;
		}

		pointer operator->() const {
			return &(operator*());
		}

		self& operator++() {
			--current;
			return *this;
		}

		self operator++(int) {
			self tmp = *this;
			--current;
			return tmp;
		}

		self& operator--() {
			++current;
			return *this;
		}

		self operator--(int) {
			self tmp = *this;
			++current;
			return tmp;
		}

		self& operator+=(difference_type n) {
			current -= n;
			return *this;
		}

		self operator+(difference_type n) const {
			return self(current - n);
		}

		self& operator-=(difference_type n) {
			current += n;
			return *this;
		}

		self operator-(difference_type n) const {
			return self(current + n);
		}

		reference operator[](difference_type n) const {
			return *(*this + n);
		}
	};

	template <typename Iterator>
	typename reverse_iterator<Iterator>::difference_type
		operator-(const reverse_iterator<Iterator>& lhs,
			const reverse_iterator<Iterator>& rhs) {
		return rhs.base() - lhs.base(); // ������reverse�򷴼�
	}

	// ���رȽϲ�����
	template <typename Iterator>
	bool operator==(const reverse_iterator<Iterator>& lhs,
		const reverse_iterator<Iterator>& rhs) {
		return rhs.base() == lhs.base();
	}

	template <typename Iterator>
	bool operator!=(const reverse_iterator<Iterator>& lhs,
		const reverse_iterator<Iterator>& rhs) {
		return !(rhs == lhs);
	}

	// ����reverse_iterator��Ϊ������lhs < rhs => lhs.current > rhs.current
	template <typename Iterator>
	bool operator<(const reverse_iterator<Iterator>& lhs,
		const reverse_iterator<Iterator>& rhs) {
		return rhs.base() < lhs.base();
	}

	template <typename Iterator>
	bool operator>(const reverse_iterator<Iterator>& lhs,
		const reverse_iterator<Iterator>& rhs) {
		return rhs < lhs;
	}

	template <typename Iterator>
	bool operator>=(const reverse_iterator<Iterator>& lhs,
		const reverse_iterator<Iterator>& rhs) {
		return !(lhs < rhs);
	}

	template <typename Iterator>
	bool operator<=(const reverse_iterator<Iterator>& lhs,
		const reverse_iterator<Iterator>& rhs) {
		return !(lhs > rhs);
	}
}