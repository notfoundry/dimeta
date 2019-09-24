/*
* Created by Mark Johnson on 6/22/2019.
*/

#ifndef DIMETA_PRIMITIVES_HPP
#define DIMETA_PRIMITIVES_HPP

#include <kvasir/mpl/algorithm/all.hpp>

#include <kvasir/mpl/functional/call.hpp>

#include <kvasir/mpl/types/integral_constant.hpp>
#include <kvasir/mpl/types/list.hpp>
#include <kvasir/mpl/types/bool.hpp>

#include <kvasir/mpl/utility/same_as.hpp>
#include <kvasir/mpl/utility/is_instance.hpp>

#include <kvasir/mpl/sequence/is_list.hpp>

#include <dimeta/detail/consistency.hpp>

namespace dm {
    namespace mpl = kvasir::mpl;

    enum class logic { L, H, Z, X };

    template <logic L>
    using logic_constant = mpl::integral_constant<logic, L>;

    template <class T>
    struct is_logic_constant : mpl::false_ {};

    template <logic L>
    struct is_logic_constant<logic_constant<L>> : mpl::true_ {};


    using virtual_time = unsigned int;

    template <virtual_time T>
    using time_constant = mpl::integral_constant<virtual_time, T>;

    template <class T>
    struct is_time_constant : mpl::false_ {};

    template <virtual_time T>
    struct is_time_constant<time_constant<T>> : mpl::true_ {};


    using index = unsigned int;

    template <index I>
    using index_constant = mpl::integral_constant<index, I>;

    template <class T>
    struct is_index_constant : mpl::false_ {};

    template <index I>
    struct is_index_constant<index_constant<I>> : mpl::true_ {};


    enum class logic_transition { pass, rise, fall };

    template <logic_transition LT>
    using transition_constant =  mpl::integral_constant<logic_transition, LT>;

    template <class T>
    struct is_transition_constant : mpl::false_ {};

    template <logic_transition LT>
    struct is_transition_constant<transition_constant<LT>> : mpl::true_ {};


    template <class GateID, class PinID>
    struct logic_connection {
        using gate_id = GateID;
        DIMETA_ASSERT(is_index_constant<GateID>::value);

        using pin_id = PinID;
        DIMETA_ASSERT(is_index_constant<PinID>::value);
    };

    template <class T>
    using is_logic_connection = mpl::eager::is_instance<logic_connection, T>;


    template <
            class DelayIndex,
            class InputValues, class OutputValue,
            class FanoutList,
            class GateLogic
    >
    struct netlist_element {
        DIMETA_ASSERT(is_index_constant<DelayIndex>::value);

        DIMETA_ASSERT(mpl::eager::is_list<InputValues>::value);
        DIMETA_ASSERT(mpl::eager::all<InputValues, is_logic_constant>::value);

        DIMETA_ASSERT(is_logic_constant<OutputValue>::value);

        DIMETA_ASSERT(mpl::eager::is_list<FanoutList>::value);
        DIMETA_ASSERT(mpl::eager::all<FanoutList, is_logic_connection>::value);

        DIMETA_ASSERT(mpl::call<mpl::same_as<OutputValue>, mpl::call<mpl::unpack<GateLogic>, InputValues>>::value,
                      "Re-evaluating gate logic with current inputs did not produce current output");

        using delay_index = DelayIndex;
        using inputs = InputValues;
        using output = OutputValue;
        using fanout = FanoutList;
        using gate_logic = GateLogic;
    };

    template <class T>
    using is_netlist_element = mpl::eager::is_instance<netlist_element, T>;
}

#endif //DIMETA_PRIMITIVES_HPP
