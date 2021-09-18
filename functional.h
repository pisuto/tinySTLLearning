#pragma once

// ������������͹�ϣ���� functor�º���

namespace tinySTL {

	// ����һԪ�����Ĳ������ͷ���ֵ�ͱ�
	template<typename Arg, typename Result>
	struct unarg_function {
		using argument_type = Arg;
		using result_type = Result;
	};

	// ��Ԫ����
	template<typename Arg1, typename Arg2, typename Result>
	struct binary_function {
		using first_argument_type = Arg1;
		using second_argument_type = Arg2;
		using result_type = Result;
	};

	// �������󣺼ӷ�
	template<typename T>
	struct plus : public binary_function<T, T, T> {
		T operator()(const T& x, const T& y) const { return x + y; }
	};

	template<typename T>
	struct minus :public binary_function<T, T, T> {
		T operator()(const T& x, const T& y) const { return x - y; }
	};

	// �������󣺳˷�
	template <class T>
	struct multiplies :public binary_function<T, T, T>
	{
		T operator()(const T& x, const T& y) const { return x * y; }
	};

	// �������󣺳���
	template <class T>
	struct divides :public binary_function<T, T, T>
	{
		T operator()(const T& x, const T& y) const { return x / y; }
	};

	// ��������ģȡ
	template <class T>
	struct modulus :public binary_function<T, T, T>
	{
		T operator()(const T& x, const T& y) const { return x % y; }
	};

	// �������󣺷�
	template <class T>
	struct negate :public unarg_function<T, T>
	{
		T operator()(const T& x) const { return -x; }
	};

	/// �ӷ���֤ͬԪ�� ???
	template<typename T>
	T identity_element(plus<T>) { return T(0); }

	/// �˷���֤ͬԪ�� ???
	template <class T>
	T identity_element(multiplies<T>) { return T(1); }

	// �������󣺵���
	template<typename T>
	struct equal_to :public binary_function<T, T, bool> {
		bool operator()(const T& x, const T& y) const { return x == y; }
	};

	// �������󣺲�����
	template <class T>
	struct not_equal_to :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x != y; }
	};

	// �������󣺴���
	template <class T>
	struct greater :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x > y; }
	};

	// ��������С��
	template <class T>
	struct less :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x < y; }
	};

	// �������󣺴��ڵ���
	template <class T>
	struct greater_equal :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x >= y; }
	};

	// ��������С�ڵ���
	template <class T>
	struct less_equal :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x <= y; }
	};

	// ���������߼���
	template <class T>
	struct logical_and :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x && y; }
	};

	// ���������߼���
	template <class T>
	struct logical_or :public binary_function<T, T, bool>
	{
		bool operator()(const T& x, const T& y) const { return x || y; }
	};

	// ���������߼���
	template <class T>
	struct logical_not :public unarg_function<T, bool>
	{
		bool operator()(const T& x) const { return !x; }
	};

	//֤ͬ������
	//�κ���ֵͨ���˺����󣬲������κθı�
	//��ʽ������<stl_set.h>������ָ��RB-tree�����KeyOfValue op
	//������ΪsetԪ�صļ�ֵ��ʵ��ֵ�����Բ���identity
	template<typename T>
	struct identity :public unarg_function<T, bool> {
		const T& operator()(const T& x) const { return x; }
	};

	//ѡ����1��
	//����һ��pair���������һԪ��
	//��ʽ����<stl_map.h>������ָ��RB-tree�����KeyOfValue op
	//����map����pairԪ�صĵ�һԪ��Ϊ���ֵ�����Բ���select1st
	template<typename Pair>
	struct selectfirst :public unarg_function<Pair, typename Pair::first_type> {
		const typename Pair::first_type& operator()(const Pair& pair) const { return pair.first; }
	};

	template<typename Pair>
	struct selectsecond :public unarg_function<Pair, typename Pair::first_type> {
		const typename Pair::first_type& operator()(const Pair& pair) const { return pair.second; }
	};

	// Ͷ�亯�������ص�һ���������Եڶ�����
	template<typename Arg1, typename Arg2>
	struct projectfirst :public binary_function<Arg1, Arg2, Arg1> {
		Arg1 operator()(const Arg1& x, const Arg2& y) const { return x; }
	};

	template<typename Arg1, typename Arg2>
	struct projectsecond :public binary_function<Arg1, Arg2, Arg1> {
		Arg1 operator()(const Arg1& x, const Arg2& y) const { return y; }
	};

	/*---------------------------------------------------------------------------*/
	/// ��ϣ�������� �����

	template<typename Key>
	struct hash {};

	/// ���ָ���ƫ�ػ��汾 ???
	template<typename T>
	struct hash<T*> {
		size_t operator()(T* p) const noexcept { return reinterpret_cast<size_t>(p); }
	};


}