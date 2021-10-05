#pragma once

#include "template_type.h"

// helper for type_list

namespace tmpl
{
    template<typename H, typename T>
    struct type_add
    {
        using result = int_type<H::value + T::value>;
    };

    template<typename H, typename T>
    struct type_is_equal
    {
        using result = bool_type<H::value == T::value>;
    };

    template<typename H, typename T>
    struct type_is_same
    {
        using result = false_result;
    };

    template<typename H>
    struct type_is_same<H, H>
    {
        using result = true_result;
    };

    template<typename H>
    struct type_negate;

    template<bool b>
    struct type_negate<bool_type<b>>
    {
        using result = bool_type<!b>;
    };

    template<typename H1, typename H2>
    struct type_and;

    template<bool b1, bool b2>
    struct type_and<bool_type<b1>, bool_type<b2>>
    {
        using result = bool_type<b1&& b2>;
    };

    template<typename T>
    struct type_is_valid
    {
        using result = true_result;
    };

    template<>
    struct type_is_valid<null_type>
    {
        using result = false_result;
    };

    template<typename B, typename H, typename T>
    struct type_is_choose;

    template<typename H, typename T>
    struct type_is_choose<true_result, H, T>
    {
        using result = H;
    };

    template<typename H, typename T>
    struct type_is_choose<false_result, H, T>
    {
        using result = T;
    };

    template<typename H, typename T>
    struct type_less
    {
        using result = bool_type < H::value < T::value>;
    };

    template<typename H, typename T>
    struct type_greater
    {
        // >大于号好像不能很好区分
        using result = bool_type < T::value < H::value>;
    };

    template<typename H, typename T>
    struct type_greater_equal
    {
        // >大于号好像不能很好区分
        using result = bool_type < H::value >= T::value>;
    };

    template<typename B, typename D>
    struct type_is_convertible
    {
    //private:
    //    static D self();
    //    static bool test(...) { return false; };
    //    static bool test(B) { return true; };

    //public:
    //    // 这种方法不行是因为返回值是在运行时候决定的
    //    using result = bool_type<test(self())>;

    private:
        using Yes = char;
        struct No { char dummy[2]; };

        static Yes test(B);
        static No test(...);
        static D self();

    public:
        using result = bool_type<sizeof(Yes) == sizeof(test(self()))>;
    };

    template<typename B, typename D>
    struct type_is_base_of
    {
    private:
        using condition_not_same_type = typename type_and<
            typename type_negate<typename type_is_same<B, D>::result>::result,
            typename type_negate<typename type_is_same<void*, D*>::result>::result>::result;
    public:
        using result = typename type_and<condition_not_same_type,
            typename type_is_convertible<B*, D*>::result>::result;
    };
}