//
// Created by Mark on 9/13/2019.
//

#ifndef DIMETA_FUTURE_HPP
#define DIMETA_FUTURE_HPP

#include <kvasir/mpl/sequence/at.hpp>
#include <kvasir/mpl/utility/is_instance.hpp>

#include <dimeta/detail/consistency.hpp>
#include <dimeta/primitives.hpp>
#include <dimeta/delay.hpp>

namespace dm {
    namespace mpl = kvasir::mpl;

    template <class GVTOffset, class Destination, class Output>
    struct future_event {
        DIMETA_ASSERT(is_time_constant<GVTOffset>::value);
        DIMETA_ASSERT(is_logic_connection<Destination>::value);
        DIMETA_ASSERT(is_logic_constant<Output>::value);

        using gvt_offset = GVTOffset;
        using destination = Destination;
        using output = Output;

    };

    template <class T>
    using is_future_event = mpl::eager::is_instance<future_event, T>;


    template <class GVTOffset, class Output>
    struct make_future_event {
        template <class Destination>
        using f = future_event<GVTOffset, Destination, Output>;
    };

    template <class OldLogic, class NewLogic>
    struct output_comparator {
        DIMETA_ASSERT(is_logic_constant<OldLogic>::value);
        DIMETA_ASSERT(is_logic_constant<NewLogic>::value);

        template <class C, class PinID, class NLE, class DelayMap>
        using f = typename C::template f<>;
    };

    template <class OldLogic>
    struct output_comparator<OldLogic, OldLogic> {
        DIMETA_ASSERT(is_logic_constant<OldLogic>::value);

        template <class C, class PinID, class NLE, class DelayMap>
        using f = typename C::template f<>;
    };

    struct rising_logic_output_comparator {
        template <class C, class NLE, class GVTOffset>
        using make_future_events = mpl::call<mpl::unpack<mpl::transform<make_future_event<GVTOffset, typename NLE::output>, C>>, typename NLE::fanout>;

        template <class PinID, class NLE, class DelayMap>
        using rise_time = typename mpl::call<
                mpl::unpack<mpl::at<typename NLE::delay_index>>, DelayMap
        >::template rise_time<PinID, NLE>;

        template <class C, class PinID, class NLE, class DelayMap>
        using f = make_future_events<C, NLE, rise_time<PinID, NLE, DelayMap>>;
    };

    template <>
    struct output_comparator<logic_constant<logic::L>, logic_constant<logic::H>> : rising_logic_output_comparator {};

    template <>
    struct output_comparator<logic_constant<logic::X>, logic_constant<logic::H>> : rising_logic_output_comparator {};

    template <>
    struct output_comparator<logic_constant<logic::Z>, logic_constant<logic::H>> : rising_logic_output_comparator {};

    struct falling_logic_output_comparator {
        template <class C, class NLE, class GVTOffset>
        using make_future_events = mpl::call<mpl::unpack<mpl::transform<make_future_event<GVTOffset, typename NLE::output>, C>>, typename NLE::fanout>;

        template <class PinID, class NLE, class DelayMap>
        using fall_time = typename mpl::call<
                mpl::unpack<mpl::at<typename NLE::delay_index>>, DelayMap
        >::template fall_time<PinID, NLE>;

        template <class C, class PinID, class NLE, class DelayMap>
        using f = make_future_events<C, NLE, fall_time<PinID, NLE, DelayMap>>;
    };

    template <>
    struct output_comparator<logic_constant<logic::H>, logic_constant<logic::L>> : falling_logic_output_comparator {};

    template <>
    struct output_comparator<logic_constant<logic::X>, logic_constant<logic::L>> : falling_logic_output_comparator {};

    template <>
    struct output_comparator<logic_constant<logic::Z>, logic_constant<logic::L>> : falling_logic_output_comparator {};

    template <class C = mpl::listify>
    struct future_event_generator {
        template <class PinID, class OldNLE, class NewNLE, class DelayMap>
        using f = DIMETA_TMPL_ASSERT
                (is_index_constant<PinID>::value,
                        is_netlist_element<OldNLE>::value,
                        is_netlist_element<NewNLE>::value,
                        mpl::eager::all<DelayMap, is_delay>::value)
                (mpl::call<output_comparator<typename OldNLE::output, typename NewNLE::output>, C, PinID, NewNLE, DelayMap>);
    };
}

#endif //DIMETA_FUTURE_HPP
