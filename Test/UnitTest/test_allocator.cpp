#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"
#include "../../allocator.h"

#include <iostream>
#include <vector>

TEST_CASE("[Allocator] alloc memory for int type")
{
	tinySTL::allocator<int> alloc;

	SUBCASE("check value type") {
		auto ptr = alloc.allocate(1);
		
		CHECK(ptr);

		int* temp = nullptr;
		CHECK(typeid(ptr) == typeid(temp));
		std::cout << "check value type: " << typeid(ptr).name() << std::endl;

		alloc.deallocate(ptr);
	}
}

TEST_CASE("[Allocator] instead of vector's default allocator")
{
	/* This allocator<> is incompatible with std... */
	// int arr[5] = { 0,1,2,3,4 };
	// std::vector<int, tinySTL::allocator<int>> vec{arr, arr + 5};
	// CHECK(vec.size() == sizeof(arr) / sizeof(int));
}