//
// Created by Mark on 9/23/2019.
//

#ifndef DIMETA_MACRO_SIMULATOR_HPP
#define DIMETA_MACRO_SIMULATOR_HPP

#include <kvasir/mpl/algorithm/transform.hpp>

#include <kvasir/mpl/functional/bind.hpp>
#include <kvasir/mpl/functional/call.hpp>

#include <kvasir/mpl/sequence/join.hpp>

#include <kvasir/mpl/types/list.hpp>

#include <dimeta/primitives.hpp>
#include <dimeta/scheduling.hpp>
#include <dimeta/simulator.hpp>

#include <dimeta/detail/consistency.hpp>
#include <dimeta/detail/mpl/map.hpp>

namespace dm::macro {
    namespace mpl = kvasir::mpl;

    template <class Wire, logic State>
    struct state_assignment {
        using wire = Wire;
        using logic = dm::logic_constant<State>;
    };

    template <class... StateAssignments>
    struct initial_conditions {};

    template <class Netlist, typename RenameFrame, typename ConnectionMap>
    struct simulation_result {
    private:
        template <class WireConnection>
        using get_netlist_element = mpl::call<mpl::unpack<mpl::at<typename WireConnection::gate_id>>, Netlist>;

        template <class OriginalWire>
        using probe_impl =
                mpl::call<
                        mpl::unpack<
                                dm::detail::mpl::map::get<OriginalWire,
                                        mpl::unpack<
                                                mpl::front<
                                                        mpl::cfe<get_netlist_element>>>>>,
                ConnectionMap>;
    public:
        template <class Wire>
        using probe =
                mpl::call<
                        mpl::unpack<
                                dm::detail::mpl::map::get<Wire,
                                        mpl::cfe<probe_impl>>>,
                RenameFrame>;

        using netlist = Netlist;
    };

    namespace detail {

        template <class StateAssignment, class WireRenames>
        struct generate_assignment_events {

            template <class LogicConnection>
            using schedule_event_from_connection = dm::scheduled_event<
                    time_constant<0>,
                    LogicConnection,
                    typename StateAssignment::logic
            >;

            template <class LogicConnectionList>
            using schedule_connection_events = DIMETA_TMPL_ASSERT
                    (!mpl::call<mpl::same_as<void>, LogicConnectionList>::value)
                    (mpl::call<
                            mpl::unpack<mpl::transform<mpl::cfe<schedule_event_from_connection>>>,
                            LogicConnectionList
                    >);

            template <class Wire>
            using get_renamed_wire = DIMETA_TMPL_LOG_ASSERT
                    (DIMETA_TMPL_LOG_CASE(mpl::call<mpl::unpack<dm::detail::mpl::map::contains<Wire>>, WireRenames>::value, Wire))
                    (mpl::call<mpl::unpack<dm::detail::mpl::map::get<Wire>>, WireRenames>);

            template <class... ConnectionMap>
            using f = DIMETA_TMPL_LOG_ASSERT
                    (DIMETA_TMPL_LOG_CASE
                             (mpl::call<dm::detail::mpl::map::contains<get_renamed_wire<typename StateAssignment::wire>>, ConnectionMap...>::value,
                              get_renamed_wire<typename StateAssignment::wire>,
                              typename StateAssignment::wire),
                     DIMETA_TMPL_LOG_CASE(
                             (mpl::call<mpl::size<>, ConnectionMap...>::value > 0)))
                    (mpl::call<
                            dm::detail::mpl::map::get<get_renamed_wire<typename StateAssignment::wire>, mpl::cfe<schedule_connection_events>>,
                            ConnectionMap...
                    >);
        };

        template <class CompiledModule, class InitialConditions>
        struct simulation_driver {

            template <class StateAssignment>
            using schedule_event = mpl::call<mpl::unpack<generate_assignment_events<StateAssignment, typename CompiledModule::renames>>, typename CompiledModule::connections>;

            using scheduled_events = mpl::call<mpl::unpack<mpl::transform<mpl::cfe<schedule_event>, mpl::join<>>>, InitialConditions>;

            using simulation = typename dm::logic_simulator<
                    event_scheduler<mpl::list<>, scheduled_events>,
                    typename CompiledModule::netlist,
                    typename CompiledModule::delay_map
            >::type;

            using type = simulation_result<typename simulation::netlist, typename CompiledModule::renames, typename CompiledModule::connections>;
        };
    }

    template <class CompiledModule, class InitialConditions>
    using simulate = typename detail::simulation_driver<CompiledModule, InitialConditions>::type;
}

#endif //DIMETA_MACRO_SIMULATOR_HPP
