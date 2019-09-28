/*
* Created by Mark Johnson on 6/22/2019.
*/

#ifndef DIMETA_CONSISTENCY_HPP
#define DIMETA_CONSISTENCY_HPP

#ifdef DIMETA_CONSISTENCY_CHECKS

#include <cstddef>
#include <utility>

#define DIMETA_ASSERT(...) static_assert(__VA_ARGS__)

namespace dm::detail {
    template <bool... Bs>
    struct template_assert {
        static_assert((Bs && ...), "Contract not satisfied");

        template <class T>
        using f = T;
    };
}

#define DIMETA_TMPL_ASSERT_IMPL(...) ::template f<__VA_ARGS__>
#define DIMETA_TMPL_ASSERT(...) typename ::dm::detail::template_assert<__VA_ARGS__> DIMETA_TMPL_ASSERT_IMPL

namespace dm::detail {
    template <std::size_t I, class C>
    struct indexed_template_log_assert_case : C {};

    template <bool B, class... Ts>
    struct template_log_assert_case {
        static_assert(B, "Contract not satisfied");
    };

    template <class Indices, class... Cs>
    struct template_log_assert_impl;

    template <std::size_t... Is, class... Cs>
    struct template_log_assert_impl<std::index_sequence<Is...>, Cs...> : indexed_template_log_assert_case<Is, Cs>... {
        template <class T>
        using f = T;
    };

    template <class... Cs>
    struct template_log_assert : template_log_assert_impl<std::make_index_sequence<sizeof...(Cs)>, Cs...> {
        using template_log_assert_impl<std::make_index_sequence<sizeof...(Cs)>, Cs...>::f;
    };
}

#define DIMETA_TMPL_LOG_ASSERT_IMPL(...) ::template f<__VA_ARGS__>
#define DIMETA_TMPL_LOG_ASSERT(...) typename ::dm::detail::template_log_assert<__VA_ARGS__> DIMETA_TMPL_LOG_ASSERT_IMPL
#define DIMETA_TMPL_LOG_CASE(...) ::dm::detail::template_log_assert_case<__VA_ARGS__>

#else

#define DIMETA_ASSERT(...)

#define DIMETA_TMPL_ASSERT_IMPL(...) __VA_ARGS__
#define DIMETA_TMPL_ASSERT(...) DIMETA_TMPL_ASSERT_IMPL

#define DIMETA_TMPL_LOG_ASSERT_IMPL(...) __VA_ARGS__
#define DIMETA_TMPL_LOG_ASSERT(...) DIMETA_TMPL_LOG_ASSERT_IMPL

#define DIMETA_TMPL_LOG_CASE(...)

#endif

#endif //DIMETA_CONSISTENCY_HPP
