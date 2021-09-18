#pragma once

// 包含函数对象和哈希函数 functor仿函数

namespace tinySTL {

	// 定义一元函数的参数类别和返回值型别
	template<typename Arg, typename Result>
	struct unarg_function {
		using argument_type = Arg;
		using result_type = Result;
	};

	// 二元函数
	template<typename Arg1, typename Arg2, typename Result>
	struct binary_function {
		using first_argument_type = Arg1;
		using second_argument_type = Arg2;
		using result_type = Result;
	};

	// 函数对象：加法
	template<typename T>
	struct plus : public binary_function<T, T, T> {
		T operator()(const T& x, const T& y) const { return x + y; }
	};

	template<typename T>
	struct minus :public binary_function<T, T, T> {
		T operator()(const T& x, const T& y) const { return x - y; }
	};

	// 函数对象：乘法
	template <class T>
	struct multiplies :public binary_function<T, T, T>
	{
		T operator()(const T& x, const T& y) const { return x * y; }
	};

	// 函数对象：除法
	template <class T>
	struct divides :public binary_function<T, T, T>
	{
		T operator()(const T& x, const T& y) const { return x / y; }
	};

	// 函数对象：模取
	template <class T>
	struct modulus :public binary_function<T, T, T>
	{
		T operator()(const T& x, const T& y) const { return x % y; }
	};

	// 函数对象：否定
	template <class T>
	struct negate :public unarg_function<T, T>
	{
		T operator()(const T& x) const { return -x; }
	};

	/// 加法的证同元素 ???
	template<typename T>
	T identity_element(plus<T>) { return T(0); }

	/// 乘法的证同元素 ???
	template <class T>
	T identity_element(multiplies<T>) { return T(1); }

	// 函数对象：等于
	template<typename T>
	struct equal_to :public binary_function<T, T, bool> {
		bool operator()(const T& x, const T& y) const { return x == y; }
	};

	// 函数对象：不等于
	template <class T>
	struct not_equal_to :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x != y; }
	};

	// 函数对象：大于
	template <class T>
	struct greater :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x > y; }
	};

	// 函数对象：小于
	template <class T>
	struct less :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x < y; }
	};

	// 函数对象：大于等于
	template <class T>
	struct greater_equal :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x >= y; }
	};

	// 函数对象：小于等于
	template <class T>
	struct less_equal :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x <= y; }
	};

	// 函数对象：逻辑与
	template <class T>
	struct logical_and :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x && y; }
	};

	// 函数对象：逻辑或
	template <class T>
	struct logical_or :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x || y; }
	};

	// 函数对象：逻辑非
	template <class T>
	struct logical_not :public unarg_function<T, bool>
	{
		bool operator()(const T& x) const { return !x; }
	};

	//证同函数。
	//任何数值通过此函数后，不会有任何改变
	//此式运用于<stl_set.h>，用来指定RB-tree所需的KeyOfValue op
	//那是因为set元素的键值即实际值，所以采用identity
	template<typename T>
	struct identity :public unarg_function<T, bool> {
		const T& operator()(const T& x) const { return x; }
	};

	//选择函数1。
	//接收一个pair，传回其第一元素
	//此式用于<stl_map.h>，用来指定RB-tree所需的KeyOfValue op
	//由于map是以pair元素的第一元素为其键值，所以采用select1st
	template<typename Pair>
	struct selectfirst :public unarg_function<Pair, typename Pair::first_type> {
		const typename Pair::first_type& operator()(const Pair& pair) const { return pair.first; }
	};

	template<typename Pair>
	struct selectsecond :public unarg_function<Pair, typename Pair::first_type> {
		const typename Pair::first_type& operator()(const Pair& pair) const { return pair.second; }
	};

	// 投射函数：返回第一参数，忽略第二参数
	template<typename Arg1, typename Arg2>
	struct projectfirst :public binary_function<Arg1, Arg2, Arg1> {
		Arg1 operator()(const Arg1& x, const Arg2& y) const { return x; }
	};

	template<typename Arg1, typename Arg2>
	struct projectsecond :public binary_function<Arg1, Arg2, Arg1> {
		Arg1 operator()(const Arg1& x, const Arg2& y) const { return y; }
	};

	/*---------------------------------------------------------------------------*/
	/// 哈希函数对象 待完成

	template<typename Key>
	struct hash {};

	/// 针对指针的偏特化版本 ???
	template<typename T>
	struct hash<T*> {
		size_t operator()(T* p) const noexcept { return reinterpret_cast<size_t>(p); }
	};


}