#pragma once

#include <new>
#include <cstdio>

namespace tinySTL {

	// 4 = sizeof FreeList
	union FreeList {
		union FreeList* next;
		char data[1];
	};

	// Ϊ�˱��ڹ��������ռ��������ڷ����ʱ������8�ı������롣Ҳ����˵
	// �����������Ὣ�κ�С���ڴ�������ϵ���8�ı�����(���磺Ҫ7���ֽڣ���
	// �������8���ֽڡ�Ҫ9���ֽڣ������16���ֽ�), ����������������Ƭ����
	// �⣬���Ƕ������ǹ�����˵ȴ���˲��١���Ϊ�����Ļ�ֻҪά��16��FreeList
	// �Ϳ����ˣ�FreeList��16�����ֱ�����СΪ8, 16, 24, 32, 40, 48, 
	// 56, 64, 72, 80, 88, 86, 96, 104, 112, 120, 128�ֽڴ�С���ڴ�����
	// �ˡ�
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
	class MallocAllocTemplate {									  // һ���ռ�������
	private:
		static void* oomMalloc(size_t n);						  // mallocʧ��ʱ���ã���oomΪout of memory

		static MALLOCALLOCFUN MallocAllocOomHandler;              // Ĭ�ϲ�����
	public:
		static void* allocate(size_t n);

		static void deallocate(void* p) { std::free(p); }

		static MALLOCALLOCFUN SetMallocHandler(MALLOCALLOCFUN f); // OOM_malloc** �лص����freeMemor������������os����ڴ棬ʹ��malloc����ɹ���

	};

	template<int inst>
	MALLOCALLOCFUN MallocAllocTemplate<inst>::MallocAllocOomHandler = nullptr; // Ĭ�ϲ�����

	template<int inst>
	void* MallocAllocTemplate<inst>::oomMalloc(size_t n) {
		MALLOCALLOCFUN myMallocHandler;
		void* result;
		while (true) {
			myMallocHandler = MallocAllocOomHandler; // û�������ڴ治�㴦��������׳��쳣
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
		result = std::malloc(n); // malloc����ʧ��
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

		static size_t getFreeListIdx(size_t bytes) { // �õ�����ֽڶ�Ӧ������������Ӧȡ��λ��
			return (bytes + (size_t)ALIGN - 1) / (size_t)ALIGN - 1;
		}

		static size_t getRoundUp(size_t bytes) { // ������ֽ�����ȡ��8�ı���
			return (bytes + (size_t)ALIGN - 1) & (~(ALIGN - 1));
		}

		static void* reFill(size_t n); // �����������������ڴ�,n��ʾҪ���ڴ�Ĵ�С
		static char* chunkAlloc(size_t size, int& objs); // ���ڴ���������ڴ�objs������ÿ������size����С

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
		if (MAXBytes < n) { // ����_MAXBYTES���ֽ�����Ϊ�Ǵ���ڴ棬ֱ�ӵ���һ���ռ�������

			ret = malloc_alloc::allocate(n);
		}
		else {

			FreeList** nowLoc = freeList + getFreeListIdx(n); // ��myFreeListָ������������n����ȡ8���������ĵ�ַ
			FreeList* result = *nowLoc;
			if (!result) {
				ret = reFill(getRoundUp(n));
			}
			else {
				*nowLoc = result->next; // ��result->next���������ϵĵ�ַ��ԭ��ַ������Ķ���ʹ�ú��ͷ�
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
			*nowLoc = q; // ����ָ��p�����ͷ�
		}
	}

	template<bool threads, int inst>
	void* DefaultAllocTemplate<threads, inst>::reFill(size_t n) {
		int objs = 20;
		char* chunk = chunkAlloc(n, objs);    // ��Ϊ����������û�У�����Ҫ���ڴ�������룬������ٹҵ�������������
		if (1 == objs) // ����ɹ�����Ŀռ�ֻ��һ������ֱ�ӷ���
			return chunk;

		FreeList* ret = (FreeList*)chunk;
		FreeList** myFreeList = freeList + getFreeListIdx(n);
		*myFreeList = (FreeList*)(chunk + n); // Ϊʲô��n����Ϊ���ɵ�chunk�ڴ���Ҫ��ʹ�õģ���˺�һ��chunk��������
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
			size_t newBytes = 2 * totalBytes + getRoundUp(heapSize >> 4); // ����һλ����2������λ����16
			if (leftBytes > 0) {										  // ��ʣ��Ŀռ���뵽������	
				FreeList** nowLoc = freeList + getFreeListIdx(leftBytes);
				((FreeList*)startFree)->next = *nowLoc;
				*nowLoc = (FreeList*)startFree;
			}

			startFree = (char*)std::malloc(newBytes);
			if (!startFree) { // //�������ʧ�ܵĻ��������ϵͳ�Ѿ�û���ڴ��ˣ���ʱ�����Ǿ�Ҫ��������������һ���n������ڴ�飬�����û�еĻ����Ǿ͵�һ���ռ�������
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