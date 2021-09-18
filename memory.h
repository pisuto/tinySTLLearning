#pragma once

#include <type_traits>
#include <ostream>
#include <exception>
#include <functional>


namespace tinySTL {

	// 获取对象地址
	template<typename Tp>
	constexpr Tp* address_of(Tp& value) noexcept {
		return &value;
	}

	// 获取/释放临时缓冲区

	template<typename T>
	pair<T*, ptrdiff_t> get_buffer_helper(ptrdiff_t len, T*) {
		if (len > static_cast<ptrdiff_t>(INT_MAX / sizeof(T)))
			len = INT_MAX / sizeof(T);
		while (len > 0) {
			T* tmp = static_cast<T*>(malloc(static_cast<size_t>(len) * sizeof(T)));
			if (tmp)
				return pair<T*, ptrdiff_t>(tmp, len);
			len /= 2; // 申请失败时减少len的大小
		}
		return pair<T*, ptrdiff_t>(nullptr, 0);
	}

	template<typename T>
	pair<T*, ptrdiff_t> get_temporary_buffer(ptrdiff_t len) {
		return get_buffer_helper(len, static_cast<T*>(0));
	}

	template<typename T>
	void release_temporary_buffer(T* ptr) {
		free(ptr);
	}

	//-----------------------------------------------------------------------
	// 类模板 : temporary_buffer
	// 进行临时缓冲区的申请与释放
	template<typename ForwardIter, typename T>
	class temporary_buffer {
	private:
		ptrdiff_t original_len;   // 缓冲区申请的大小
		ptrdiff_t len;            // 缓冲区实际的大小
		T* buffer;         // 指向缓冲区的指针

	public:
		temporary_buffer(ForwardIter first, ForwardIter last);

		~temporary_buffer() {
			destroy(buffer, buffer + len);
			free(buffer);
		}

		temporary_buffer(const temporary_buffer&) = delete;
		void operator=(const temporary_buffer&) = delete;

	public:

		ptrdiff_t requested_size() const noexcept { return original_len; }
		ptrdiff_t size()           const noexcept { return len; }
		T* begin()                noexcept { return buffer; }
		T* end()                  noexcept { return buffer + len; }

	private:
		void allocate_buffer();
		void initialize_buffer(const T&, std::true_type) {}
		void initialize_buffer(const T& value, std::false_type) {
			uninitialized_fill_n(buffer, len, value);
		}
	};

	template<typename ForwardIter, typename T>
	temporary_buffer<ForwardIter, T>::temporary_buffer(ForwardIter first,
		ForwardIter last) {
		try {
			len = distance(first, last);
			allocate_buffer();
			if (len > 0) {
				initialize_buffer(*first, std::is_trivially_default_constructible<T>());
			}
		}
		catch (...) {
			free(buffer);
			buffer = nullptr;
			len = 0;
		}
	}

	template<typename ForwardIter, typename T>
	void temporary_buffer<ForwardIter, T>::allocate_buffer() {
		original_len = len;
		auto res = get_temporary_buffer<T>(original_len);
		buffer = res.first;
		len = res.second;
	}

	//------------------------------------------------------------------
	// 模板类: auto_ptr
	// 一个具有严格对象所有权的小型智能指针
	// 用unique_ptr替代

	template<typename T>
	class auto_ptr {
	public:
		using element_type = T;

	private:
		T* ptr; // raw pointer

	public:
		explicit auto_ptr(T* p = nullptr) : ptr(p) {}
		auto_ptr(auto_ptr& rhs) :ptr(rhs.release()) {}

		template<typename U>
		auto_ptr(auto_ptr<U>& rhs) : ptr(rhs.release()) {}

		auto_ptr& operator=(auto_ptr& rhs) {
			if (this != &rhs) {
				delete ptr;
				ptr = rhs.release();
			}
			return *this;
		}

		template<typename U>
		auto_ptr& operator=(auto_ptr<U>& rhs) {
			if (this->get() != rhs.get()) {
				delete ptr;
				ptr = rhs.release();
			}
			return *this;
		}

		~auto_ptr() { delete ptr; }

	public:

		T& operator*() { return *ptr; }
		T* operator->() { return ptr; }

		// 获得指针
		T* get() const {
			return ptr;
		}

		// 释放指针
		T* release() {
			T* tmp = ptr;
			ptr = nullptr;
			return tmp;
		}

		// 重置指针
		void reset(T* p = nullptr) {
			if (ptr != p) {
				delete ptr;
				ptr = p;
			}
		}
	};

	//-----------------------------------------------------------------------
	// 智能指针: unique_ptr
	// https://github.com/RRRadicalEdward/smart_pointers/blob/master/

	template<typename T>
	class weak_ptr;

	template<typename T>
	class shared_ptr;

	// typename = void 区分不同的特化类型 
	template<typename FROM, typename TO>
	using pointers_are_convertible = std::enable_if_t<std::is_convertible<FROM*, TO*>::value>;

	template<typename T>
	struct is_unbounded_array : std::false_type {};

	template<typename T>
	struct is_unbounded_array<T[]> : std::true_type {};

	template<typename T>
	struct is_bounded_array : std::false_type {};

	template<typename T, std::size_t N>
	struct is_unbounded_array<T[N]> : std::true_type {};

	// default deleter

	template<typename T>
	struct default_deleter final {
	public:
		constexpr default_deleter() noexcept = default;

		template<typename U, typename = pointers_are_convertible<U, T>>
		default_deleter(const default_deleter<U>&) noexcept {}

		void operator()(T* ptr) const noexcept {
			delete ptr;
		}
	};

	template<typename T>
	struct default_deleter<T[]> {
	public:
		constexpr default_deleter() noexcept = default;

		template<typename U, typename = pointers_are_convertible<U, T>>
		default_deleter(const default_deleter<U>&) noexcept {}

		template<typename U, typename = pointers_are_convertible<U, T>>
		default_deleter(default_deleter<U>&&) noexcept {}

		virtual void operator()(T* ptr) const noexcept {
			delete[] ptr;
		}
	};

	template<typename T, typename Deleter = default_deleter<T>>
	class unique_ptr final {
	public:
		// Member types
		using pointer = T*;
		using element_type = T;
		using deleter_type = Deleter;
	private:
		pointer ptr;
		Deleter deleter;

	public:
		// Member function
		constexpr unique_ptr() noexcept : ptr(nullptr) {}

		constexpr unique_ptr(std::nullptr_t) noexcept : ptr(nullptr) {}

		explicit unique_ptr(pointer res) : ptr(res) {}

		unique_ptr(pointer res, const deleter_type& deleter) noexcept
			: ptr(res), deleter(deleter) {}

		unique_ptr(pointer res, deleter_type&& deleter)
			: ptr(res), deleter(move(deleter)) {}

		unique_ptr(const unique_ptr&) = delete;

		unique_ptr(unique_ptr&& other) noexcept
			: ptr(other.release()), deleter(move(other.get_deleter())) {}

		template<typename Y, typename D,
			typename = pointers_are_convertible<typename unique_ptr<Y, D>::element_type, T>>
			unique_ptr(unique_ptr<Y, D>&& other) noexcept
			: ptr(static_cast<pointer>(other.release())),
			deleter(move(static_cast<deleter_type>(other.release()))) {}

		~unique_ptr() {
			if (ptr) {
				deleter(ptr);
			}
		}

	public:
		// operator override
		unique_ptr& operator=(const unique_ptr&) = delete;

		unique_ptr& operator=(unique_ptr&& rhs) noexcept {
			if (this == &rhs)
				return *this;
			reset(rhs.release());
			deleter = move(rhs.get_deleter());
			return *this;
		}

		template<typename Y, typename D,
			typename = pointers_are_convertible<typename unique_ptr<Y, D>::element_type, T>>
			unique_ptr & operator=(unique_ptr<Y, D>&& rhs) noexcept {
			reset(static_cast<pointer>(rhs.release()));
			deleter = move(static_cast<Deleter>(rhs.get_deleter()));
			return *this;
		}

	public:
		// modifiers
		pointer release() noexcept {
			pointer old = ptr;
			ptr = nullptr;
			return old;
		}

		void reset(pointer p = pointer{}) noexcept {
			pointer old = ptr;
			ptr = p;
			if (old)
				deleter(old);
		}

		void swap(unique_ptr& other) noexcept {
			if (this != other) {
				std::swap(ptr, other.ptr);
				std::swap(deleter, other.deleter);
			}
		}

	public:
		// Observers
		pointer get() const noexcept {
			return ptr;
		}

		deleter_type& get_deleter() noexcept {
			return deleter;
		}

		const deleter_type& get_deleter() const noexcept {
			return deleter;
		}

		explicit operator bool() const noexcept {
			return ptr;
		}

		T& operator*() const {
			return *ptr;
		}

		pointer operator->() const noexcept {
			return ptr;
		}
	};

	template<typename CharT, typename Traits, typename T, typename D>
	std::basic_ostream<CharT, Traits>& operator<<(std::basic_ostream<CharT, Traits>& os,
		const unique_ptr<T, D>& unique) {
		std::cout << *unique;
		return os;
	}

	template<typename T, typename ...Args,
		typename = std::enable_if_t<!::std::is_array<T>::value>>
		unique_ptr<T> make_unique(Args&& ...args) {
		return unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template<typename T,
		typename = std::enable_if_t<is_unbounded_array<T>::value>>
		unique_ptr<T> make_unique(std::size_t size) {
		return unique_ptr<T>(new std::remove_extent_t<T>[size]());
	}

	/// ???
	template<typename T, typename...Args>
	std::enable_if_t<is_bounded_array<T>::value>
		make_unique(Args&&...) = delete;

	template<class T,
		typename = std::enable_if_t<!::std::is_array<T>::value>>
		unique_ptr<T> make_unique_for_overwrite() {
		return unique_ptr<T>(new T);
	}

	template<typename T,
		typename = std::enable_if_t<is_unbounded_array<T>::value>>
		unique_ptr<T> make_unique_for_overwrite(size_t size) {
		return unique_ptr<T>(new std::remove_extent_t<T>[size]);
	}

	/// ???
	template<typename T, typename...Args>
	std::enable_if_t<is_bounded_array<T>::value>
		make_unique_for_overwrite(Args&&...) = delete;

	template<typename T1, class D1, typename T2, class D2>
	bool operator==(const unique_ptr<T1, D1>& lhs, const unique_ptr<T2, D2>& rhs) noexcept
	{
		return lhs.get() == rhs.get();
	}

	template<typename T1, class D1, typename T2, class D2>
	bool operator!=(const unique_ptr<T1, D1>& lhs, const unique_ptr<T2, D2>& rhs) noexcept
	{
		return !(lhs == rhs);
	}

	template<typename T1, class D1, typename T2, class D2>
	bool operator<(const unique_ptr<T1, D1>& lhs, const unique_ptr<T2, D2>& rhs) noexcept
	{
		return  lhs.get() < rhs.get();
	}
	template<typename T1, class D1, typename T2, class D2>
	bool operator>(const unique_ptr<T1, D1>& lhs, const unique_ptr<T2, D2>& rhs) noexcept
	{
		return rhs < lhs;
	}
	template<typename T1, class D1, typename T2, class D2>
	bool operator<=(const unique_ptr<T1, D1>& lhs, const unique_ptr<T2, D2>& rhs) noexcept
	{
		return !(rhs < lhs);
	}
	template<typename T1, class D1, typename T2, class D2>
	bool operator>=(const unique_ptr<T1, D1>& lhs, const unique_ptr<T2, D2>& rhs) noexcept
	{
		return !(lhs < rhs);
	}
	template<typename T, class D>
	bool operator==(const unique_ptr<T, D>& lhs, ::std::nullptr_t) noexcept
	{
		return !lhs;
	}
	template<typename T, class D>
	bool operator==(::std::nullptr_t, const unique_ptr<T, D>& rhs) noexcept
	{
		return !rhs;
	}
	template<typename T, class D>
	bool operator!=(const unique_ptr<T, D>& lhs, ::std::nullptr_t) noexcept
	{
		return (bool)lhs;
	}
	template<typename T, class D>
	bool operator!=(::std::nullptr_t, const unique_ptr<T, D>& rhs) noexcept
	{
		return (bool)rhs;
	}
	template<typename T, class D>
	bool operator<(const unique_ptr<T, D>& lhs, ::std::nullptr_t) noexcept
	{
		return lhs.get() < nullptr;
	}
	template<typename T, class D>
	bool operator<(::std::nullptr_t, const unique_ptr<T, D>& rhs) noexcept
	{
		return nullptr < rhs.get();
	}
	template<typename T, class D>
	bool operator>(const unique_ptr<T, D>& lhs, ::std::nullptr_t) noexcept
	{
		return nullptr < lhs;
	}
	template<typename T, class D>
	bool operator>(::std::nullptr_t, const unique_ptr<T, D>& rhs) noexcept
	{
		return rhs < nullptr;
	}
	template<typename T, class D>
	bool operator<=(const unique_ptr<T, D>& lhs, ::std::nullptr_t) noexcept
	{
		return !(nullptr < lhs);
	}
	template<typename T, class D>
	bool operator<=(::std::nullptr_t, const unique_ptr<T, D>& rhs) noexcept
	{
		return !(rhs < nullptr);
	}
	template<typename T, class  D>
	bool operator>=(const unique_ptr<T, D>& lhs, ::std::nullptr_t) noexcept
	{
		return !(lhs < nullptr);
	}
	template<typename T, class D>
	bool operator>=(::std::nullptr_t, const unique_ptr<T, D>& rhs) noexcept
	{
		return !(nullptr < rhs);
	}
	template<typename T, class D>
	void swap(unique_ptr<T, D>& lhs, unique_ptr<T, D>& rhs) noexcept
	{
		lhs.swap(rhs);
	}
	template<typename R, typename D, typename OtherType>
	using is_constructible_from = std::enable_if<
		std::is_same<OtherType*, typename  unique_ptr<R, D>::pointer>::value ||
		std::is_same<OtherType*, std::nullptr_t>::value ||
		(std::is_same<typename unique_ptr<R, D>::pointer, typename unique_ptr<R, D>::element_type*>::value
			&& std::is_convertible<OtherType(*)[], R(*)[]>::value)
	>;
	template<typename R1, class D1, typename R2, class D2>
	using is_constructible_from_array = std::enable_if<
		std::is_array<R2>::value&&
		std::is_same<typename unique_ptr<R1, D1>::pointer, typename unique_ptr<R1, D1>::element_type*>::value&&
		std::is_same<typename unique_ptr<R2, D2>::pointer, typename unique_ptr<R2, D2>::element_type*>::value&&
		std::is_convertible<typename unique_ptr<R2, D2>::element_type(*)[], typename unique_ptr<R2, D2>::element_type(*)[]>::value
	>;
	template<typename T, typename Deleter>
	class unique_ptr<T[], Deleter> {
	public:
		using pointer = T*;
		using element_type = T;
		using deleter_type = Deleter;
	private:
		pointer ptr;
		deleter_type deleter;
	public:
		constexpr unique_ptr() noexcept
			: ptr(nullptr), deleter(default_deleter<T[]>{}) {}
		constexpr unique_ptr(std::nullptr_t) noexcept
			: ptr(nullptr), deleter(default_deleter<T[]>{}) {}
		template<typename Y, typename = is_constructible_from<T, deleter_type, Y>>
		explicit unique_ptr(Y* ptr) noexcept
			: ptr(static_cast<pointer>(ptr)), deleter(default_deleter<T[]>{}) {}
		template<typename Y, typename = is_constructible_from<T, deleter_type, Y>>
		unique_ptr(Y* ptr, const deleter_type& deleter) noexcept
			: ptr(static_cast<pointer>(ptr)), deleter(move(deleter)) {}
		unique_ptr(unique_ptr&& other) noexcept
			: ptr(other.release()), deleter(move(other.get_deleter())) {}
		template<typename Y, typename D,
			typename = is_constructible_from_array<T, Deleter, Y, D>>
			unique_ptr(unique_ptr<Y, D>&& other) noexcept
			: ptr(other.release()), deleter(move(other.get_deleter())) {}
		~unique_ptr() {
			if (this->ptr)
				this->deleter(ptr);
		}
	public:
		template<typename Y,
			typename = TypeCanBeConstructedFrom<T, Deleter, Y>>
			void reset(Y* ptr) noexcept
		{
			pointer old_ptr = this->ptr;
			this->ptr = static_cast<pointer>(ptr);
			if (old_ptr)
				deleter(old_ptr);
		}
		void reset(std::nullptr_t = nullptr) noexcept
		{
			pointer old_ptr = this->ptr;
			this->ptr = nullptr;
			if (old_ptr)
				deleter(old_ptr);
		}
		pointer release() noexcept
		{
			pointer old_ptr = this->ptr;
			this->ptr = nullptr;
			return old_ptr;
		}
		template<typename Y, class D>
		void swap(unique_ptr& other) noexcept
		{
			if (this == &other)
				return;
			tinySTL::swap(this->ptr, other.ptr);
			tinySTL::swap(this->ptr, other.ptr);
		}
		pointer get() const noexcept
		{
			return ptr;
		}
		deleter_type& get_deleter() noexcept
		{
			return deleter;
		}
		const deleter_type& get_deleter() const noexcept
		{
			return deleter;
		}
		explicit operator bool() const noexcept
		{
			return ptr;
		}
		element_type& operator[](size_t index) const
		{
			return get()[index];
		}
		unique_ptr& operator=(unique_ptr&& other) noexcept {
			if (this == &other)
				return *this;
			this->reset(other.release());
			this->deleter = move(other.get_deleter());
			return *this;
		}
	};

	// 智能指针：shared_ptr
	// 
	//

	// for enable_shared_from_this https://segmentfault.com/a/1190000020861953
	template<typename...>
	using void_t = void;

	template<typename T, typename = void>
	struct can_enable_shared : std::false_type {};

	template<typename T>
	struct can_enable_shared<T, void_t<typename T::class_type>> : std::is_convertible<std::remove_cv_t<T>*,
		typename T::class_type*> {};

	template<typename T, typename Y>
	void check_if_enable_shared(const shared_ptr<T>& other, Y* ptr, std::false_type) {}

	template<typename T, typename Y>
	void check_if_enable_shared(const shared_ptr<T>& other, Y* ptr, std::true_type) {
		if (ptr) {
			ptr->construct_weak_from(other, ptr);
		}
	}

	template<typename T, typename Y>
	void set_enable_shared(const shared_ptr<T>& other, Y* ptr) {
		check_if_enable_shared<T, Y>(other, ptr,
			std::conjunction<std::negation<std::is_array<T>>,
			std::negation<std::is_volatile<Y>>,
			can_enable_shared<Y>>{});
	}
	/*----------------------------------------------------------------------------------------------------*/

	class bad_weak_ptr final : public std::exception {
	public:
		bad_weak_ptr() = default;
		const char* what() const noexcept override {
			return "expired weak ptr";
		}
	};

	template<typename T>
	class ControlBlock final {
	public:
		using deleter_type = std::function<void(T*)>;
		using pointer = T*;
		using element_type = std::remove_extent_t<T>;

	private:
		pointer ptr;
		std::atomic_size_t sharedCounter;
		std::atomic_size_t weakCounter;
		deleter_type deleter;

	public:
		ControlBlock(size_t s, size_t w, pointer ptr, deleter_type deleter) noexcept
			: ptr(ptr), sharedCounter(s), weakCounter(w), deleter(deleter) {}

		~ControlBlock() noexcept {}

		template<typename Deleter, typename U>
		friend Deleter* get_deleter(const shared_ptr<U>& ptr) noexcept;

		size_t shared_count() const noexcept {
			return sharedCounter.load();
		}

		size_t weak_count() const noexcept {
			return weakCounter.load();
		}

		void delSharedPtr() noexcept {
			decreaseSharedCounter();
			if (!sharedCounter) {
				deleter(ptr);
				delWeakPtr();
			}
		}

		void delWeakPtr() noexcept {
			decreaseWeakCounter();
			if (!weakCounter)
				delete this;
		}

		void increaseSharedCounter() noexcept
		{
			this->sharedCounter++;
			this->weakCounter++;
		}

		void increaseWeakCounter() noexcept
		{
			this->weakCounter++;
		}

		void decreaseSharedCounter() noexcept
		{
			this->sharedCounter--;
			this->weakCounter--;
		}

		void decreaseWeakCounter() noexcept
		{
			this->weakCounter--;
		}
	};

	template<typename Deleter, typename T>
	Deleter* get_deleter(const shared_ptr<T>& shared) noexcept {
		using funcRef = void(*)(T*);
		return !shared.cb ? nullptr : shared.cb->deleter.template target<funcRef>();
	}

	template<typename T>
	class shared_ptr final {
	public:
		using pointer = T*;
		using deleter_type = std::function<void(pointer)>;
		using element_type = std::remove_extent_t<T>;
		using control_block_ptr = ControlBlock<T>*;

		template<typename>
		friend class weak_ptr;

		template<typename>
		friend class shared_ptr;

	private:
		pointer ptr;
		control_block_ptr cb;

	public:
		shared_ptr() noexcept
			: ptr(nullptr), cb(nullptr) {}

		shared_ptr(std::nullptr_t) noexcept
			: ptr(nullptr), cb(nullptr) {}

		template<typename Y,
			typename = pointers_are_convertible<Y, T>>
			explicit shared_ptr(Y* res) noexcept
			: ptr(static_cast<element_type*>(res)) {
			cb = new ControlBlock<element_type>(1, 1,
				static_cast<element_type*>(res), default_deleter<T>{});
			set_enable_shared(*this, ptr); // check if ptr derivrd from enable_shared_from_this
		}

		template<typename Y, typename Deleter,
			typename = pointers_are_convertible<Y, T>>
			shared_ptr(Y* res, const Deleter& deleter)
			: ptr(static_cast<T*>(res)) {
			auto deleter_wrapper = [deleter](T* res) {
				deleter(static_cast<Y*>(res));
			};
			cb = new ControlBlock<T>(1, 1,
				static_cast<T*>(res), deleter_wrapper);
			set_enable_shared(*this, ptr); // check if ptr derivrd from enable_shared_from_this
		}

		template<typename Deleter>
		shared_ptr(std::nullptr_t ptr, const Deleter& deleter)
			: ptr(nullptr) {
			auto deleter_wrapper = [deleter](T* res) {
				deleter(nullptr);
			};
			cb = new ControlBlock<T>(1, 1,
				nullptr, deleter_wrapper);
		}

		// enable_shared_from
		template<typename Y>
		shared_ptr(const shared_ptr<Y>& other, element_type* ptr) noexcept
			: ptr(ptr), cb(reinterpret_cast<control_block_ptr>(other.cb)) {
			if (cb)
				cb->increaseSharedCounter();
		}

		// enable_shared_from
		template<typename Y>
		shared_ptr(shared_ptr<Y>&& other, element_type* ptr) noexcept
			: ptr(ptr), cb(reinterpret_cast<control_block_ptr>(other.cb)) {
			other.cb = nullptr;
			other.ptr = nullptr;
		}

		shared_ptr(const shared_ptr& other) noexcept
			: ptr(other.ptr), cb(other.cb) {
			if (cb)
				cb->increaseSharedCounter();
		}

		shared_ptr(shared_ptr&& other) noexcept
			: ptr(other.ptr), cb(other.cb) {
			other.ptr = nullptr;
			other.cb = nullptr;
		}

		template<typename Y,
			typename = pointers_are_convertible<typename shared_ptr<Y>::element_type, T>>
			shared_ptr(const shared_ptr<Y>& other) noexcept
			: ptr(static_cast<pointer>(other.ptr)), cb(reinterpret_cast<control_block_ptr>(other.cb)) {
			if (cb)
				cb->increaseSharedCounter();
		}

		template<typename Y,
			typename = pointers_are_convertible<typename shared_ptr<Y>::element_type, T>>
			shared_ptr(shared_ptr<Y>&& other) noexcept
			: ptr(static_cast<pointer>(other.ptr)), cb(reinterpret_cast<control_block_ptr>(other.cb)) {
			other.ptr = nullptr;
			other.cb = nullptr;
		}

		template<typename Y,
			typename = pointers_are_convertible<typename weak_ptr<Y>::element_type, T>>
			explicit shared_ptr(const weak_ptr<Y>& other)
			: ptr(nullptr), cb(nullptr) {
			if (other.expired())
				throw bad_weak_ptr();
			cb = reinterpret_cast<control_block_ptr>(other.cb);
			ptr = static_cast<T*>(other.ptr);
			cb->increaseSharedCounter();
		}

		template<typename Y, typename Deleter = default_deleter<T>,
			typename = pointers_are_convertible<typename unique_ptr<Y, Deleter>::element_type, T>>
			shared_ptr(unique_ptr<Y, Deleter>&& unique) noexcept
			: ptr(static_cast<T*>(unique.get())) {
			auto deleter = std::move(unique.get_deleter());
			auto deleter_wrapper = [deleter](T* res) {
				deleter(static_cast<Y*>(res));
			};
			cb = new ControlBlock<T>(1, 1, static_cast<T*>(unique.release()), std::move(deleter_wrapper));
		}

		~shared_ptr() {
			if (cb) {
				cb->delSharedPtr();
			}
		}

	public:

		shared_ptr& operator=(const shared_ptr& rhs) noexcept {
			shared_ptr(rhs).swap(*this);
			return *this;
		}

		template<typename Y, typename = pointers_are_convertible<Y, T>>
		shared_ptr& operator=(const shared_ptr<Y>& rhs) noexcept {
			shared_ptr<T>(rhs).swap(*this);
			return *this;
		}

		shared_ptr& operator=(shared_ptr&& rhs) noexcept {
			shared_ptr(move(rhs)).swap(*this);
			return *this;
		}

		template<typename Y,
			typename = pointers_are_convertible<typename shared_ptr<Y>::element_type, T>>
			shared_ptr & operator=(shared_ptr<Y>&& rhs) noexcept {
			shared_ptr<T>(move(rhs)).swap(*this);
			return *this;
		}

		template<typename Deleter = default_deleter<T>>
		shared_ptr& operator=(unique_ptr<T, Deleter>&& rhs) noexcept {
			shared_ptr<T>(move(rhs)).swap(*this);
			return *this;
		}

		T& operator*() const noexcept {
			return *ptr;
		}

		T* operator->() const noexcept {
			return ptr;
		}

		explicit operator bool() const noexcept {
			return (bool)ptr;
		}

	public:

		void swap(shared_ptr& rhs) noexcept {
			if (this != &rhs) {
				std::swap(cb, rhs.cb);
				std::swap(ptr, rhs.ptr);
			}
		}

		void reset() noexcept {
			if (ptr) {
				shared_ptr().swap(*this);
			}
		}

		template<typename Y,
			typename = pointers_are_convertible<Y, T>>
			void reset(Y* res) noexcept {
			shared_ptr<T>(res).swap(*this);
		}

		template<typename Y, typename D,
			typename = pointers_are_convertible<Y, T>>
			void reset(Y* res, const D& deleter) noexcept {
			shared_ptr<T>(res, deleter).swap(*this);
		}

		element_type* get() const noexcept {
			return ptr;
		}

		size_t use_count() const noexcept {
			return (!ptr || !cb) ? 0 : cb->shared_count();
		}

		bool unqiue() const noexcept {
			return use_count() == 1;
		}

		bool owner_before(const shared_ptr& other) const noexcept {
			return cb == other.cb || (!ptr && !other.ptr);
		}

		bool owner_before(const weak_ptr<T>& other) const noexcept {
			return cb == other.cb || (!ptr && !other.ptr);
		}

	};

	template<typename T>
	void swap(shared_ptr<T>& lhs, shared_ptr<T>& rhs) noexcept {
		lhs.swap(rhs);
	}

	//template<typename Deleter, typename T>
	//Deleter* get_deleter(const shared_ptr<T>& shared) noexcept {
	//	if (shared.cb) {
	//		using funcRef = void(*)(T*);
	//		return shared.cb->deleter.template target<funcRef>();
	//	}
	//}

	template<typename T, typename U>
	bool operator==(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)  noexcept
	{
		return lhs.get() == rhs.get();
	}


	template<typename T, typename U>
	bool operator!=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)  noexcept
	{
		return !(lhs == rhs);
	}

	template<typename T, typename U>
	bool operator<(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)  noexcept
	{
		return lhs.get() < rhs.get();
	}

	template<typename T, typename U>
	bool operator>(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)  noexcept
	{
		return rhs < lhs;
	}


	template<typename T, typename U>
	bool operator<=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)  noexcept
	{
		return !(rhs < lhs);
	}

	template<typename T, typename U>
	bool operator>=(const shared_ptr<T>& lhs, const shared_ptr<U>& rhs)  noexcept
	{
		return !(lhs < rhs);
	}

	template<typename T>
	bool operator==(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
	{
		return !lhs;
	}

	template<typename T>
	bool operator==(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
	{
		return !rhs;
	}

	template<typename T>
	bool operator!=(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
	{
		return (bool)lhs;
	}

	template<typename T>
	bool operator!=(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
	{
		return (bool)rhs;
	}

	template<typename T>
	bool operator<(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
	{
		return lhs.get() < nullptr;
	}

	template<typename T>
	bool operator<(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
	{
		return nullptr < rhs.get();
	}

	template<typename T>
	bool operator>(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
	{
		return lhs.get() > nullptr;
	}

	template<typename T>
	bool operator>(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
	{
		return nullptr > rhs.get();
	}


	template<typename T>
	bool operator<=(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
	{
		return !(nullptr < lhs);
	}

	template<typename T>
	bool operator<=(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
	{
		return !(rhs < nullptr);
	}


	template<typename T>
	bool operator>=(const shared_ptr<T>& lhs, std::nullptr_t) noexcept
	{
		return !(nullptr < lhs);
	}

	template<typename T>
	bool operator>=(std::nullptr_t, const shared_ptr<T>& rhs) noexcept
	{
		return !(rhs < nullptr);
	}

	template<typename T, typename U>
	shared_ptr<T> static_pointer_cast(const shared_ptr<U>& other) noexcept {
		using pointer_type = typename shared_ptr<T>::element_type*;
		auto pointer = static_cast<pointer_type>(other.get());
		return shared_ptr<T>(other, pointer);
	}

	template<typename T, typename U>
	shared_ptr<T> dynamic_pointer_cast(const shared_ptr<U>& other) noexcept {
		using pointer_type = typename shared_ptr<T>::element_type*;
		auto pointer = dynamic_cast<pointer_type>(other.get());
		return pointer ? shared_ptr<T>(other, pointer) : shared_ptr<T>();
	}

	template<typename T, typename U>
	shared_ptr<T> reinterpret_pointer_cast(const shared_ptr<U>& other) noexcept {
		using pointer_type = typename shared_ptr<T>::element_type*;
		auto pointer = reinterpret_cast<pointer_type>(other.get());
		return shared_ptr<T>(other, pointer);
	}

	template<typename T, typename ...Args>
	shared_ptr<T> make_shared(Args&&... args) {
		return shared_ptr<T>(new T(std::forward<Args>(args)...));
	}

	// 智能指针：weak_ptr
	//
	//

	template<typename T>
	class weak_ptr final {
	public:
		using element_type = std::remove_extent_t<T>;
		using pointer = T*;
		using control_block_ptr = ControlBlock<T>*;

		template<typename>
		friend class shared_ptr;

	private:
		pointer ptr;
		control_block_ptr cb;

	public:
		weak_ptr() noexcept
			: ptr(nullptr), cb(nullptr) {}

		weak_ptr(const weak_ptr& other) noexcept
			: ptr(other.ptr), cb(other.cb) {
			if (cb)
				cb->increaseWeakCounter();
		}

		template<typename Y,
			typename = pointers_are_convertible<typename weak_ptr<Y>::element_type, T>>
			weak_ptr(const weak_ptr<Y>& other) noexcept
			: ptr(static_cast<pointer>(other.ptr)),
			cb(reinterpret_cast<control_block_ptr>(other.cb)) {
			if (cb)
				cb->increaseWeakCounter();
		}

		template<typename Y,
			typename = pointers_are_convertible<typename shared_ptr<Y>::element_type, T>>
			weak_ptr(const shared_ptr<Y>& other) noexcept
			: ptr(static_cast<pointer>(other.ptr)),
			cb(reinterpret_cast<control_block_ptr>(other.cb)) {
			if (cb)
				cb->increaseWeakCounter();
		}

		weak_ptr(weak_ptr&& other) noexcept
			:ptr(other.ptr), cb(other.cb) {
			other.ptr = nullptr;
			other.cb = nullptr;
		}

		template<typename Y,
			typename = pointers_are_convertible<typename weak_ptr<Y>::element_type, T>>
			weak_ptr(weak_ptr<Y>&& other) noexcept
			: ptr(static_cast<pointer>(other.ptr)),
			cb(reinterpret_cast<control_block_ptr>(other.cb)) {
			other.ptr = nullptr;
			other.cb = nullptr;
		}

		weak_ptr& operator=(const weak_ptr& rhs) noexcept {
			weak_ptr(rhs).swap(*this);
			return *this;
		}

		weak_ptr& operator=(weak_ptr&& rhs) noexcept {
			weak_ptr(move(rhs)).swap(*this);
			return *this;
		}

		template<typename Y,
			typename = pointers_are_convertible<typename weak_ptr<Y>::element_type, T>>
			weak_ptr & operator=(const weak_ptr<Y>& rhs) noexcept {
			weak_ptr(rhs).swap(*this);
			return *this;
		}

		template<typename Y,
			typename = pointers_are_convertible<typename shared_ptr<Y>::element_type, T>>
			weak_ptr & operator=(const shared_ptr<Y>& rhs) noexcept {
			weak_ptr(rhs).swap(*this);
			return *this;
		}

		template<typename Y,
			typename = pointers_are_convertible<typename weak_ptr<Y>::element_type, T>>
			weak_ptr & operator=(weak_ptr<Y>&& rhs) noexcept {
			weak_ptr(move(rhs)).swap(*this);
			return *this;
		}

		~weak_ptr() noexcept {
			if (cb) {
				cb->delWeakPtr();
			}
		}

	public:
		size_t use_count() const {
			return cb->shared_count();
		}

		void reset() {
			if (cb) {
				cb->decreaseWeakCounter();
				cb = nullptr;
				ptr = nullptr;
			}
		}

		void swap(weak_ptr& other) {
			if (this != &other) {
				std::swap(cb, other.cb);
				std::swap(ptr, other.ptr);
			}
		}

		bool expired() const {
			return !ptr || !cb->shared_count();
		}

		shared_ptr<T> lock() const {
			return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
		}
	};

	template<typename T>
	class enable_shared_from_this {
	public:
		using class_type = enable_shared_from_this;

	protected:
		enable_shared_from_this() {}

		enable_shared_from_this(const enable_shared_from_this&) {}

		enable_shared_from_this& operator=(const enable_shared_from_this&) { return *this; }

		~enable_shared_from_this() = default;

	public:
		shared_ptr<T> shared_from_this() { return weak_this.lock(); }

		shared_ptr<const T> shared_from_this() const { return weak_this.lock(); }

		template<typename Y>
		void construct_weak_from(const shared_ptr<T>& other, Y* ptr) {
			if (weak_this.expired())
				weak_this = shared_ptr<std::remove_cv_t<T>>(other, const_cast<std::remove_cv_t<Y>*>(ptr));
		}

	private:
		mutable weak_ptr<T> weak_this;
	};
}