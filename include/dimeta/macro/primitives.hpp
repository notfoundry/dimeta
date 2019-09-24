//
// Created by Mark on 9/22/2019.
//

#ifndef DIMETA_MACRO_PRIMITIVES_HPP
#define DIMETA_MACRO_PRIMITIVES_HPP

#include <kvasir/mpl/algorithm/all.hpp>

#include <kvasir/mpl/functional/bind.hpp>
#include <kvasir/mpl/functional/call.hpp>

#include <kvasir/mpl/utility/same_as.hpp>
#include <kvasir/mpl/utility/is_instance.hpp>

#include <kvasir/mpl/types/bool.hpp>

#include <dimeta/detail/consistency.hpp>

namespace dm::macro {
    namespace mpl = kvasir::mpl;

    template <auto...>
    struct wire {};

    template <class T>
    struct is_wire : mpl::false_ {};

    template <auto... Xs>
    struct is_wire<wire<Xs...>> : mpl::true_ {};

    template <class T>
    struct is_wire<T*> : is_wire<T> {};


    template <class... Inputs>
    struct in {
        DIMETA_ASSERT(mpl::call<mpl::all<mpl::cfe<is_wire>>, Inputs...>::value);

        template <class C>
        using f = mpl::call<C, Inputs...>;
    };

    template <class T>
    using is_input_specification = mpl::eager::is_instance<in, T>;


    template <class... Outputs>
    struct out {
        DIMETA_ASSERT(mpl::call<mpl::all<mpl::cfe<is_wire>>, Outputs...>::value);

        template <class C>
        using f = mpl::call<C, Outputs...>;
    };

    template <class T>
    using is_output_specification = mpl::eager::is_instance<out, T>;


    template <class In, class Out, class GateLogic, class Delay>
    struct cell {
        DIMETA_ASSERT(is_input_specification<In>::value);
        DIMETA_ASSERT(is_output_specification<Out>::value);

        using in = In;
        using out = Out;
        using gate_logic = GateLogic;
        using delay = Delay;
    };

    template <class T>
    using is_cell = mpl::eager::is_instance<cell, T>;


    template <class In, class Out, class... Elements>
    struct assembly;

    template <class T>
    using is_assembly = mpl::eager::is_instance<assembly, T>;

    template <class T>
    using is_assembly_element = mpl::bool_<(is_cell<T>::value || is_assembly<T>::value)>;

    template <class In, class Out, class... Elements>
    struct assembly {
        DIMETA_ASSERT(is_input_specification<In>::value);
        DIMETA_ASSERT(is_output_specification<Out>::value);
        DIMETA_ASSERT(mpl::call<mpl::all<mpl::cfe<is_assembly_element>>, Elements...>::value);

        using in = In;
        using out = Out;

        template <class C>
        using f = mpl::call<C, Elements...>;
    };
}

#endif //DIMETA_MACRO_PRIMITIVES_HPP
