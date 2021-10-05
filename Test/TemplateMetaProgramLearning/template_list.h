#pragma once

#include "template_util.h"

namespace tmpl
{
    template<typename H, typename T>
    struct type_elem
    {
        using head = H;
        using tail = T;
    };

    template<typename H, typename... Ts>
    struct type_list
    {
        using result = type_elem<H, typename type_list<Ts...>::result>;
        // static constexpr auto size = sizeof(Ts)...;
    };

    template<typename H>
    struct type_list<H>
    {
        using result = type_elem<H, null_type>;
    };

    template<typename H>
    struct type_length;

    template<>
    struct type_length<null_type>
    {
        using result = int_type<0>;
    };

    template<typename H, typename T>
    struct type_length<type_elem<H, T>>
    {
        using result = typename type_add<int_type<1>, typename type_length<T>::result>::result;
    };

    template<typename TL, typename N>
    struct type_at;

    template<typename H>
    struct type_at<H, int_type<0>>
    {
        using result = typename H::head;
    };

    template<typename N>
    struct type_at<null_type, N>
    {
        using result = null_type;
    };

    template<typename H, typename T, int n>
    struct type_at<type_elem<H, T>, int_type<n>>
    {
        using result = typename type_at<T, int_type<n - 1>>::result;
    };

    template<typename TL, typename V>
    struct type_is_included
    {
        using result = typename type_is_valid<typename type_at<TL, V>::result>::result;
    };

    template<typename TL, typename V>
    struct type_push;

    template<typename T>
    struct type_push<null_type, T>
    {
        using result = typename type_elem<T, null_type>;
    };

    template<>
    struct type_push<null_type, null_type>
    {
        using result = null_type;
    };

    template<typename H, typename T, typename V>
    struct type_push<type_elem<H, T>, V>
    {
        using result = type_elem<H, typename type_push<T, V>::result>;
    };

    template<typename TL, typename V>
    struct type_erase;

    template<typename V>
    struct type_erase<null_type, V>
    {
        using result = null_type;
    };

    //template<typename H, typename T>
    //struct type_erase<type_elem<H, T>, H>
    //{
    //    using result = T;
    //};
    //
    //template<typename H, typename T, typename V>
    //struct type_erase<type_elem<H, T>, V>
    //{
    //    using result = type_elem<H, typename type_erase<T, V>::result>>;
    //};

    template<typename H, typename T, typename V>
    struct type_erase<type_elem<H, T>, V>
    {
        using result = typename type_is_choose<
            typename type_is_equal<H, V>::result,
            typename T,
            type_elem<H, typename type_erase<T, V>::result>>::result;
    };

    template<typename TL, typename V>
    struct type_erase_all;

    template<typename V>
    struct type_erase_all<null_type, V>
    {
        using result = null_type;
    };

    template<typename H, typename T, typename V>
    struct type_erase_all<type_elem<H, T>, V>
    {
        using result = typename type_is_choose<
            typename type_is_equal<H, V>::result,
            null_type,
            type_elem<H, typename type_erase_all<T, V>::result>>::result;
    };

    template<typename TL>
    struct type_unique;

    template<>
    struct type_unique<null_type>
    {
        using result = null_type;
    };

    template<typename H, typename T>
    struct type_unique<type_elem<H, T>>
    {
        using result = type_elem<H, typename type_unique<typename type_erase<T, H>::result>::result>;
    };

    template<typename TL, typename K, typename U>
    struct type_replace;

    template<typename H, typename T, typename U>
    struct type_replace<type_elem<H, T>, H, U>
    {
        using result = type_elem<U, T>;
    };

    template<typename K, typename U>
    struct type_replace<null_type, K, U>
    {
        using result = null_type;
    };

    template<typename H, typename T, typename K, typename U>
    struct type_replace<type_elem<H, T>, K, U>
    {
        using result = type_elem<H, typename type_replace<T, K, U>::result>;
    };

    template<typename L1, typename L2>
    struct type_is_subset;

    template<typename L1>
    struct type_is_subset<L1, null_type>
    {
        using result = false_result;
    };

    template<typename L2>
    struct type_is_subset<null_type, L2>
    {
        using result = true_result;
    };

    template<>
    struct type_is_subset<null_type, null_type>
    {
        using result = true_result;
    };

    template<typename H, typename T1, typename T2>
    struct type_is_subset<type_elem<H, T1>, type_elem<H, T2>>
    {
        using result = typename type_is_subset<T1, T2>::result;
    };

    template<typename H1, typename T1, typename H2, typename T2>
    struct type_is_subset<type_elem<H1, T1>, type_elem<H2, T2>>
    {
        using result = typename type_is_subset<type_elem<H1, T1>, T2>::result;
    };

    template<typename TL1, typename TL2>
    struct type_append;

    template<typename H1, typename TL2>
    struct type_append<type_elem<H1, null_type>, TL2>
    {
        using result = type_elem<H1, TL2>;
    };

    template<typename H1, typename T1, typename TL2>
    struct type_append<type_elem<H1, T1>, TL2>
    {
        using result = typename type_elem<H1, typename type_append<T1, TL2>::result>;
    };

    template<typename TL, typename V, template<typename S1, typename S2> class Pred>
    struct type_filter;

    template<typename V, template<typename S1, typename S2> class Pred>
    struct type_filter<null_type, V, Pred>
    {
        using result = null_type;
    };

    template<typename H, typename T, typename V, template<typename H, typename V> class Pred>
    struct type_filter<type_elem<H, T>, V, Pred>
    {
        using result = typename type_is_choose<typename Pred<H, V>::result,
            type_elem<H, typename type_filter<T, V, Pred>::result>,
            typename type_filter<T, V, Pred>::result>::result;
    };

    template<typename TL>
    struct type_quick_sort;

    template<>
    struct type_quick_sort<null_type>
    {
        using result = null_type;
    };

    template<typename H, typename T>
    struct type_quick_sort<type_elem<H, T>>
    {
        using result = typename type_append<
            typename type_push<typename type_filter<typename type_quick_sort<T>::result, H, type_less>::result, H>::result,
            typename type_filter<typename type_quick_sort<T>::result, H, type_greater_equal>::result
        >::result;
    };

    using list = typename type_list<int_type<2>, int_type<1>, int_type<2>, int_type<0>, int_type<3>>::result;
    // using length   = typename type_length<list>::result;
    // using index_of = typename type_at<list, 1>::result; error usage
    // using index_of = typename type_at<list, int_type<1>>::result;
    using push = typename type_push<list, int_type<4>>::result;
    // using erase     = typename type_erase<list, int_type<2>>::result;
    using erase_all = typename type_erase_all<list, int_type<2>>::result;
    using unique = typename type_unique<list>::result;
    using replace = typename type_replace<list, int_type<1>, int_type<2>>::result;
    // using replace_all = ... try this
    // maybe wrong
    using is_subset = typename type_is_subset<
        type_elem<int_type<3>, type_elem<int_type<2>, null_type>>,
        list>::result;
    using append = typename type_append<
        type_elem<int_type<3>, type_elem<int_type<2>, null_type>>,
        type_elem<int_type<1>, type_elem<int_type<2>, null_type>>>::result;
    // 切记不能忘记result
    // using filter = typename type_filter<list, int_type<2>, type_greater>::result;
    using quick_sort = typename type_quick_sort<list>::result;

}