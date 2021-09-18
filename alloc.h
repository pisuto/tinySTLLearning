#pragma once

#include <new>
#include <cstdio>

namespace tinySTL {

	// 4 = sizeof FreeList
	union FreeList {
		union FreeList* next;
		char data[1];
	};

	// 为了便于管理，二级空间配置器在分配的时候都是以8的倍数对齐。也就是说
	// 二级配置器会将任何小块内存的需求上调到8的倍数处(例如：要7个字节，会
	// 给你分配8个字节。要9个字节，会给你16个字节), 尽管这样做有内碎片的问
	// 题，但是对于我们管理来说却简单了不少。因为这样的话只要维护16个FreeList
	// 就可以了，FreeList这16个结点分别管理大小为8, 16, 24, 32, 40, 48, 
	// 56, 64, 72, 80, 88, 86, 96, 104, 112, 120, 128字节大小的内存块就行
	// 了。
	enum {
		ALIGN = 8,
	};

	enum {
		MAXBytes = 128,
	};

	enum {
		FREELISTS = 16,
	};

	using MALLOCALLOCFUN = void(*)();

	template <int inst>
	class MallocAllocTemplate {									  // 一级空间配置器
	private:
		static void* oomMalloc(size_t n);						  // malloc失败时调用，即oom为out of memory

		static MALLOCALLOCFUN MallocAllocOomHandler;              // 默认不处理
	public:
		static void* allocate(size_t n);

		static void deallocate(void* p) { std::free(p); }

		static MALLOCALLOCFUN SetMallocHandler(MALLOCALLOCFUN f); // OOM_malloc** 中回调你的freeMemor函数进而帮助os获得内存，使得malloc分配成功。

	};

	template<int inst>
	MALLOCALLOCFUN MallocAllocTemplate<inst>::MallocAllocOomHandler = nullptr; // 默认不处理

	template<int inst>
	void* MallocAllocTemplate<inst>::oomMalloc(size_t n) {
		MALLOCALLOCFUN myMallocHandler;
		void* result;
		while (true) {
			myMallocHandler = MallocAllocOomHandler; // 没有设置内存不足处理机制则抛出异常
			if (!myMallocHandler)
				throw std::bad_alloc();
			(*myMallocHandler)();
			if (result = std::malloc(n))
				break;
		}
		return result;
	}

	template<int inst>
	void* MallocAllocTemplate<inst>::allocate(size_t n) {
		void* result = 0;
		result = std::malloc(n); // malloc分配失败
		if (!result)
			oomMalloc(n);
		return result;
	}

	template <int inst>
	MALLOCALLOCFUN MallocAllocTemplate<inst>::SetMallocHandler(MALLOCALLOCFUN f) {
		MALLOCALLOCFUN old = MallocAllocOomHandler;
		MallocAllocOomHandler = f;
		return old;
	}

	using malloc_alloc = MallocAllocTemplate<0>;

	template <bool threads, int inst>
	class DefaultAllocTemplate {
	private:
		static char* startFree;
		static char* endFree;
		static size_t heapSize;
		static union FreeList* freeList[FREELISTS];

		static size_t getFreeListIdx(size_t bytes) { // 得到这个字节对应在自由链表中应取的位置
			return (bytes + (size_t)ALIGN - 1) / (size_t)ALIGN - 1;
		}

		static size_t getRoundUp(size_t bytes) { // 对这个字节向上取成8的倍数
			return (bytes + (size_t)ALIGN - 1) & (~(ALIGN - 1));
		}

		static void* reFill(size_t n); // 在自由链表中申请内存,n表示要的内存的大小
		static char* chunkAlloc(size_t size, int& objs); // 在内存池中申请内存objs个对象，每个对象size个大小

	public:
		static void* allocate(size_t n);
		static void deallocate(void* p, size_t n);
	};

	template <bool threads, int inst>
	char* DefaultAllocTemplate<threads, inst>::startFree = nullptr;

	template <bool threads, int inst>
	char* DefaultAllocTemplate<threads, inst>::endFree = nullptr;

	template <bool threads, int inst>
	size_t DefaultAllocTemplate<threads, inst>::heapSize = 0;

	template <bool threads, int inst>
	FreeList* DefaultAllocTemplate<threads, inst>::freeList[FREELISTS] = { nullptr };

	template<bool threads, int inst>
	void* DefaultAllocTemplate<threads, inst>::allocate(size_t n) {
		void* ret;
		if (MAXBytes < n) { // 大于_MAXBYTES个字节则认为是大块内存，直接调用一级空间配置器

			ret = malloc_alloc::allocate(n);
		}
		else {

			FreeList** nowLoc = freeList + getFreeListIdx(n); // 让myFreeList指向自由链表中n向上取8的整数倍的地址
			FreeList* result = *nowLoc;
			if (!result) {
				ret = reFill(getRoundUp(n));
			}
			else {
				*nowLoc = result->next; // 将result->next覆盖链表上的地址，原地址给分配的对象使用和释放
				ret = result;
			}
		}
		return ret;
	}

	template <bool threads, int inst>
	void DefaultAllocTemplate<threads, inst>::deallocate(void* p, size_t n) {
		if (MAXBytes < n) {
			malloc_alloc::deallocate(p);
		}
		else {
			FreeList* q = (FreeList*)p;
			FreeList** nowLoc = freeList + getFreeListIdx(n);
			q->next = *nowLoc;
			*nowLoc = q; // 不对指针p进行释放
		}
	}

	template<bool threads, int inst>
	void* DefaultAllocTemplate<threads, inst>::reFill(size_t n) {
		int objs = 20;
		char* chunk = chunkAlloc(n, objs);    // 因为现在链表中没有，所以要想内存池中申请，多余的再挂到自由链表下面
		if (1 == objs) // 如果可供分配的空间只有一个，则直接返回
			return chunk;

		FreeList* ret = (FreeList*)chunk;
		FreeList** myFreeList = freeList + getFreeListIdx(n);
		*myFreeList = (FreeList*)(chunk + n); // 为什么加n？因为生成的chunk内存是要被使用的，因此后一个chunk被加上来
		FreeList* cur = *myFreeList;
		FreeList* next = nullptr;
		cur->next = nullptr;
		for (int i = 2; i < objs; ++i) {
			next = (FreeList*)(chunk + n * i);
			cur->next = next;
			cur = next;
		}
		cur->next = nullptr;
		return ret;
	}

	template<bool threads, int inst>
	char* DefaultAllocTemplate<threads, inst>::chunkAlloc(size_t size, int& objs) {
		char* result = nullptr;
		size_t totalBytes = size * objs;
		size_t leftBytes = endFree - startFree;
		if (leftBytes >= totalBytes) {
			result = startFree;
			startFree += totalBytes;
			return  result;
		}
		else if (leftBytes > size) {
			objs = (int)(leftBytes / size);
			result = startFree;
			startFree += objs * size;
			return result;
		}
		else {
			size_t newBytes = 2 * totalBytes + getRoundUp(heapSize >> 4); // 右移一位除于2，移四位除于16
			if (leftBytes > 0) {										  // 还剩余的空间加入到链表当中	
				FreeList** nowLoc = freeList + getFreeListIdx(leftBytes);
				((FreeList*)startFree)->next = *nowLoc;
				*nowLoc = (FreeList*)startFree;
			}

			startFree = (char*)std::malloc(newBytes);
			if (!startFree) { // //如果开辟失败的话，则表明系统已经没有内存了，这时候我们就要到自由链表中找一块比n还大的内存块，如果还没有的话，那就掉一级空间配置器
				for (size_t i = size; i < (size_t)MAXBytes; i += (size_t)ALIGN) {
					FreeList** nowLoc = freeList + getFreeListIdx(i);
					FreeList* p = *nowLoc;
					if (p) {
						startFree = (char*)p;
						*nowLoc = p->next;
						endFree = startFree + i;
						return chunkAlloc(size, objs);
					}
				}

				endFree = nullptr;
				startFree = (char*)malloc_alloc::allocate(newBytes);
			}

			heapSize += newBytes;
			endFree = startFree + newBytes;
			return chunkAlloc(size, objs);
		}
	}

	using alloc = DefaultAllocTemplate<0, 0>;

	/* new alloc */
	template <typename T>
	class SimpleAllocTemplate {
	public:
		static T* allocate(ptrdiff_t size) {
			if (0 == size) return nullptr;
			return (T*)(::operator new((size_t)(size)));
		}

		static T* allocate() {
			return (T*)(::operator new(sizeof(T)));
		}

		static void dellocate(T* buffer) {
			::operator delete (buffer);
		}

		static void dellocate(T* buffer, ptrdiff_t /* size */) {
			::operator delete (buffer);
		}
	};

	template <typename T>
	using new_alloc = SimpleAllocTemplate<T>;
}