/*
* Created by Mark Johnson on 6/22/2019.
*/

#ifndef DIMETA_DIMETA_HPP
#define DIMETA_DIMETA_HPP

#include <kvasir/mpl/algorithm/fold_left.hpp>
#include <kvasir/mpl/algorithm/all.hpp>

#include <kvasir/mpl/functional/bind.hpp>
#include <kvasir/mpl/functional/call.hpp>

#include <kvasir/mpl/functions/arithmetic/increment.hpp>

#include <kvasir/mpl/sequence/at.hpp>
#include <kvasir/mpl/sequence/take.hpp>
#include <kvasir/mpl/sequence/push_back.hpp>
#include <kvasir/mpl/sequence/is_list.hpp>

#include <kvasir/mpl/types/bool.hpp>

#include <kvasir/mpl/utility/conditional.hpp>
#include <kvasir/mpl/utility/is_instance.hpp>
#include <kvasir/mpl/utility/same_as.hpp>
#include <kvasir/mpl/utility/always.hpp>

#include <dimeta/primitives.hpp>
#include <dimeta/scheduling.hpp>
#include <dimeta/future.hpp>
#include <dimeta/delay.hpp>

#include <dimeta/detail/mpl/map.hpp>
#include <dimeta/detail/mpl/sequence.hpp>
#include <dimeta/detail/consistency.hpp>

namespace dm {
    namespace mpl = kvasir::mpl;

    template <class C = mpl::listify>
    struct input_assembler {
        template <class Inputs, class PinID, class Output>
        using f = mpl::call<mpl::unpack<detail::mpl::seq::assign<PinID, Output, C>>, Inputs>;
    };

    template <class NetlistElement>
    struct netlist_element_updater {
        DIMETA_ASSERT(is_netlist_element<NetlistElement>::value);

        template <class... AssembledInputs>
        using f = netlist_element<
                typename NetlistElement::delay_index,
                mpl::list<AssembledInputs...>,
                mpl::call<
                        typename NetlistElement::gate_logic,
                        AssembledInputs...
                >,
                typename NetlistElement::fanout,
                typename NetlistElement::gate_logic
        >;
    };

    template <class WorkingNetlist = mpl::list<>, class FutureEventQueue = mpl::list<>>
    struct simulator_step {
        DIMETA_ASSERT(mpl::eager::all<WorkingNetlist, mpl::eager::is_list>::value);
        DIMETA_ASSERT(mpl::call<mpl::unpack<detail::mpl::map::keys<mpl::all<mpl::cfe<is_index_constant>>>>, WorkingNetlist>::value);
        DIMETA_ASSERT(mpl::call<mpl::unpack<detail::mpl::map::values<mpl::all<mpl::cfe<is_netlist_element>>>>, WorkingNetlist>::value);

        DIMETA_ASSERT(mpl::eager::all<FutureEventQueue, is_future_event>::value);

        template <class GateID>
        using or_fallback_to_global = mpl::if_<
                mpl::same_as<void>,
                mpl::always<mpl::unpack<mpl::at<GateID>>>,
                mpl::cfe<mpl::always>
        >;

        template <class GateID, class C = mpl::identity>
        using get_working_netlist_element = mpl::call<mpl::unpack<detail::mpl::map::get<GateID, C>>, WorkingNetlist>;

        template <class GateID, class GlobalNetlist>
        using get_netlist_element_by_id = mpl::call<
                get_working_netlist_element<GateID, or_fallback_to_global<GateID>>,
                GlobalNetlist
        >;

        template <class NLE, class PendingEvent>
        using update_netlist_element = mpl::call<
                input_assembler<netlist_element_updater<NLE>>,
                typename NLE::inputs,
                typename PendingEvent::destination::pin_id,
                typename PendingEvent::output
        >;

        template <class PinID, class OldNLE, class NewNLE, class DelayMap>
        using update_future_event_queue = mpl::eager::join<
                FutureEventQueue,
                mpl::call<
                    future_event_generator<>,
                    PinID, OldNLE, NewNLE, DelayMap
                >
        >;

        template <class GateID, class NewNLE>
        using update_working_netlist = mpl::call<
                mpl::unpack<detail::mpl::map::replace<GateID, NewNLE>>,
                WorkingNetlist
        >;

        template <class Destination, class OldNLE, class NewNLE, class DelayMap>
        using update_simulator_step = simulator_step<
                update_working_netlist<typename Destination::gate_id, NewNLE>,
                update_future_event_queue<typename Destination::pin_id, OldNLE, NewNLE, DelayMap>
        >;

        template <class OldNLE, class PendingEvent, class DelayMap>
        using process_current_element = update_simulator_step<
                typename PendingEvent::destination,
                OldNLE,
                update_netlist_element<OldNLE, PendingEvent>,
                DelayMap
        >;

        template <class PendingEvent, class GlobalNetlist, class DelayMap>
        using f = DIMETA_TMPL_ASSERT
                (is_scheduled_event<PendingEvent>::value,
                        mpl::eager::all<GlobalNetlist, is_netlist_element>::value,
                        mpl::eager::all<DelayMap, is_delay>::value)
                (process_current_element<
                    get_netlist_element_by_id<typename PendingEvent::destination::gate_id, GlobalNetlist>,
                    PendingEvent, DelayMap
                >);

        using working_netlist = WorkingNetlist;

        using future_events = FutureEventQueue;
    };

    template <class Scheduler, class Netlist, class DelayMap>
    struct logic_simulator {
        DIMETA_ASSERT(is_event_scheduler<Scheduler>::value);
        DIMETA_ASSERT(mpl::eager::all<Netlist, is_netlist_element>::value);
        DIMETA_ASSERT(mpl::eager::all<DelayMap, is_delay>::value);

        template <class NewNetlist, class GateID, class NLE>
        using update_global_netlist_element_impl = DIMETA_TMPL_ASSERT
                (mpl::eager::all<NewNetlist, is_netlist_element>::value, is_index_constant<GateID>::value, is_netlist_element<NLE>::value)
                (mpl::call<mpl::unpack<detail::mpl::seq::assign<GateID, NLE>>, NewNetlist>);

        template <class NewNetlist, class GateIDAndNLE>
        using update_global_netlist_element = mpl::call<mpl::unpack<mpl::cfe<update_global_netlist_element_impl>>, GateIDAndNLE, NewNetlist>;

        template <class WorkingNetlist>
        using update_global_netlist = mpl::call<
                mpl::unpack<mpl::fold_left<mpl::cfe<update_global_netlist_element>>>,
                WorkingNetlist,
                Netlist
        >;

        template <class FutureEventQueue>
        using update_scheduler = mpl::call<mpl::unpack<Scheduler>, FutureEventQueue>;

        template <class Step>
        using step_simulator = logic_simulator<
                update_scheduler<typename Step::future_events>,
                update_global_netlist<typename Step::working_netlist>,
                DelayMap
        >;

        template <class Step, class PendingEvent>
        using process_event = mpl::call<Step, PendingEvent, Netlist, DelayMap>;

        using type = typename mpl::call<
                mpl::unpack<mpl::fold_left<mpl::cfe<process_event>, mpl::cfe<step_simulator>>>,
                typename Scheduler::pending_events,
                simulator_step<>
        >::type;
    };

    template <class Netlist, class DelayMap>
    struct logic_simulator<terminated_event_scheduler, Netlist, DelayMap> {
        using type = logic_simulator;

        using netlist = Netlist;
    };
}

#endif //DIMETA_DIMETA_HPP
