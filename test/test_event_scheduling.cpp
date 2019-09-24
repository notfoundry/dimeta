/*
* Created by Mark Johnson on 6/22/2019.
*/
#include <kvasir/mpl/types/list.hpp>

#include <dimeta/dimeta.hpp>
#include <dimeta/function/or_function.hpp>
#include <dimeta/function/not_function.hpp>

using namespace dm;

using kvasir::mpl::list;

using delays = list<
    fixed_delay<time_constant<1>, time_constant<2>>,
    fixed_delay<time_constant<2>, time_constant<2>>
>;

using netlist = list<
    netlist_element<
        index_constant<0>,
        list<logic_constant<logic::X>, logic_constant<logic::X>>,
        logic_constant<logic::X>,
        list<logic_connection<index_constant<1>, index_constant<0>>>,
        function::or_function
    >,
    netlist_element<
        index_constant<1>,
        list<logic_constant<logic::X>>,
        logic_constant<logic::X>,
        list<>,
        function::not_function
    >
>;

using initial_state = list<
    scheduled_event<
        time_constant<0>,
        logic_connection<index_constant<0>, index_constant<0>>,
        logic_constant<logic::H>
    >
>;

using simulation = typename logic_simulator<event_scheduler<list<>, initial_state>, netlist, delays>::type;

using not_result = mpl::eager::at<typename simulation::netlist, 1>;

static_assert(not_result::output::value == logic::L, "not gate did not get expected result");

int main() {}