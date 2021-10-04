#pragma once

namespace tmpl
{
    // null
    struct null_type {};

    // num type
    template<typename T, T N>
    struct num_type
    {
        using value_type = T;
        static constexpr T value = N;
    };

    template<int N>
    struct int_type : num_type<int, N> {};

    // bool
    template<bool b>
    struct bool_type : num_type<bool, b> {};

    using false_result = bool_type<false>;

    using true_result = bool_type<true>;
}