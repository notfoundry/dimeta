//
// Created by Mark on 9/22/2019.
//

#ifndef DIMETA_MACRO_COMPILER_HPP
#define DIMETA_MACRO_COMPILER_HPP

#include <kvasir/mpl/algorithm/transform.hpp>
#include <kvasir/mpl/algorithm/zip_with.hpp>
#include <kvasir/mpl/algorithm/remove_if.hpp>
#include <kvasir/mpl/algorithm/filter.hpp>
#include <kvasir/mpl/algorithm/fold_left.hpp>

#include <kvasir/mpl/functional/bind.hpp>

#include <kvasir/mpl/functions/arithmetic/plus.hpp>

#include <kvasir/mpl/sequence/make_sequence.hpp>
#include <kvasir/mpl/sequence/join.hpp>

#include <kvasir/mpl/types/list.hpp>
#include <kvasir/mpl/types/traits.hpp>

#include <dimeta/primitives.hpp>
#include <dimeta/delay.hpp>

#include <dimeta/macro/primitives.hpp>
#include <dimeta/macro/delay.hpp>

#include <dimeta/detail/mpl/map.hpp>
#include <dimeta/detail/consistency.hpp>
#include <dimeta/detail/mpl/sequence.hpp>

namespace dm::macro {
    namespace detail {
        namespace mpl = kvasir::mpl;

        template <class OldName, class NewName>
        struct name_change {
            using old_name = OldName;
            using new_name = NewName;
        };

        template <class... NameChangeEntries>
        struct rename_stack_frame {};


        template <class T, unsigned int I = 0>
        struct pointer_depth {
            using type = mpl::uint_<I>;
        };

        template <class T, unsigned int I>
        struct pointer_depth<T*, I> : pointer_depth<T, I+1> {};

        template <class T, unsigned int I>
        struct pointer_depth<T**, I> : pointer_depth<T, I+2> {};

        template <class T, unsigned int I>
        struct pointer_depth<T****, I> : pointer_depth<T, I+4> {};


        template <class T>
        struct remove_all_pointers {
            using type = T;
        };

        template <class T>
        struct remove_all_pointers<T*> : remove_all_pointers<T> {};

        template <class T>
        struct remove_all_pointers<T**> : remove_all_pointers<T> {};

        template <class T>
        struct remove_all_pointers<T****> : remove_all_pointers<T> {};


        template <class RenameStack, class C = mpl::identity>
        struct translate_wire {
            template <class Wire>
            using f = mpl::call<
                    mpl::unpack<
                            mpl::drop<typename pointer_depth<Wire>::type,
                                    mpl::front<
                                            mpl::unpack<
                                                    dm::detail::mpl::map::get<typename remove_all_pointers<Wire>::type,
                                                            C>>>>>,
                    RenameStack
            >;
        };

        template <class RenameCounter>
        struct make_rename_frame_for_wires {
            template <class Wire, class Index>
            using make_name_change = name_change<Wire, wire<RenameCounter::value + Index::value>>;

            template <class... Wires>
            using f =
                    mpl::call<
                            mpl::fork<
                                    mpl::listify,
                                    mpl::size<mpl::make_int_sequence<>>,
                                    mpl::zip_with<mpl::cfe<make_name_change>,
                                            mpl::cfe<rename_stack_frame>>>,
                    Wires...>;
        };

        template <class C = mpl::listify>
        struct join_element_inputs_and_outputs {
            template <class E>
            using f = mpl::call<
                        mpl::join<C>,
                        mpl::call<mpl::unpack<mpl::listify>, typename E::in>,
                        mpl::call<mpl::unpack<mpl::listify>, typename E::out>>;
        };

        template <class C = mpl::listify>
        struct get_local_wires {
            template <class... Elements>
            using f = mpl::call<
                    mpl::transform<join_element_inputs_and_outputs<>,
                            mpl::join<
                                    mpl::remove_if<mpl::is_pointer<>,
                                        dm::detail::mpl::seq::unique<C>>>>,
                    Elements...>;
        };


       template <bool Compound>
        struct assembly_flattener;

        template <class NewCells, class RenameCounter>
        struct assembly_flattening_state {
            using new_cells = NewCells;
            using rename_counter = RenameCounter;
        };

        struct rename_assembly_cell {
            template <class Wires, class RenameStack, class C>
            using translate = mpl::call<mpl::unpack<mpl::transform<translate_wire<RenameStack>, C>>, Wires>;

            template <class In, class RenameStack>
            using new_in = translate<In, RenameStack, mpl::cfe<dm::macro::in>>;

            template <class Out, class RenameStack>
            using new_out = translate<Out, RenameStack, mpl::cfe<dm::macro::out>>;

            template <class In, class Out, class GateLogic, class Delay, class RenameStack>
            using new_cell = cell<new_in<In, RenameStack>, new_out<Out, RenameStack>, GateLogic, Delay>;

            template <class Cell, class RenameStack>
            using f = new_cell<typename Cell::in, typename Cell::out, typename Cell::gate_logic, typename Cell::delay, RenameStack>;
        };

        template <class RenameStack, class RenameCounter, class... Elements>
        struct flatten_assembly_elements;

        template <template <class...> class RenameStack, class... RenameFrames, class RenameCounter, class... Elements>
        struct flatten_assembly_elements<RenameStack<RenameFrames...>, RenameCounter, Elements...> {
            using local_wires_in_children = mpl::call<get_local_wires<make_rename_frame_for_wires<RenameCounter>>, Elements...>;

            using updated_stack = mpl::call<mpl::push_front<local_wires_in_children>, RenameFrames...>;
            using updated_rename_counter = mpl::eager::plus<mpl::call<mpl::unpack<mpl::size<>>, local_wires_in_children>, RenameCounter>;

            template <class OldState, class NewState>
            using merge_flattening_step = assembly_flattening_state<
                    mpl::call<mpl::join<>, typename OldState::new_cells, typename NewState::new_cells>,
                    typename NewState::rename_counter
            >;

            template <class State, class Element>
            using advance_flattening_state =
                    merge_flattening_step<State,
                            typename assembly_flattener<Element::compound>::template f<Element, updated_stack, typename State::rename_counter>>;

            using type =
                    mpl::call<
                            mpl::fold_left<
                                    mpl::cfe<advance_flattening_state>>,
                    assembly_flattening_state<mpl::list<>, updated_rename_counter>, Elements...>;
        };

        template <
                template <class...> class RenameStack, class... RenameFrames,
                class RenameCounter,
                class In0, class Out0, class GL0, class D0
        >
        struct flatten_assembly_elements<RenameStack<RenameFrames...>, RenameCounter,
                cell<In0, Out0, GL0, D0>
        > {
            using local_wires_in_children =
                    mpl::call<
                            get_local_wires<make_rename_frame_for_wires<RenameCounter>>,
                            cell<In0, Out0, GL0, D0>>;

            using updated_stack = mpl::call<mpl::push_front<local_wires_in_children>, RenameFrames...>;
            using updated_rename_counter = mpl::eager::plus<mpl::call<mpl::unpack<mpl::size<>>, local_wires_in_children>, RenameCounter>;

            using type =
                    assembly_flattening_state<
                            mpl::list<
                                    typename rename_assembly_cell::template f<cell<In0, Out0, GL0, D0>, updated_stack>>,
                            updated_rename_counter>;
        };

        template <
                template <class...> class RenameStack, class... RenameFrames,
                class RenameCounter,
                class In0, class Out0, class GL0, class D0,
                class In1, class Out1, class GL1, class D1
        >
        struct flatten_assembly_elements<RenameStack<RenameFrames...>, RenameCounter,
                cell<In0, Out0, GL0, D0>,
                cell<In1, Out1, GL1, D1>
        > {
            using local_wires_in_children =
                    mpl::call<
                            get_local_wires<make_rename_frame_for_wires<RenameCounter>>,
                            cell<In0, Out0, GL0, D0>,
                            cell<In1, Out1, GL1, D1>>;

            using updated_stack = mpl::call<mpl::push_front<local_wires_in_children>, RenameFrames...>;
            using updated_rename_counter = mpl::eager::plus<mpl::call<mpl::unpack<mpl::size<>>, local_wires_in_children>, RenameCounter>;

            using type =
                    assembly_flattening_state<
                            mpl::list<
                                    typename rename_assembly_cell::template f<cell<In0, Out0, GL0, D0>, updated_stack>,
                                    typename rename_assembly_cell::template f<cell<In1, Out1, GL1, D1>, updated_stack>>,
                            updated_rename_counter>;
        };

        template <
                template <class...> class RenameStack, class... RenameFrames,
                class RenameCounter,
                class In0, class Out0, class GL0, class D0,
                class In1, class Out1, class GL1, class D1,
                class In2, class Out2, class GL2, class D2
        >
        struct flatten_assembly_elements<RenameStack<RenameFrames...>, RenameCounter,
                cell<In0, Out0, GL0, D0>,
                cell<In1, Out1, GL1, D1>,
                cell<In2, Out2, GL2, D2>
        > {
            using local_wires_in_children =
                    mpl::call<
                            get_local_wires<make_rename_frame_for_wires<RenameCounter>>,
                            cell<In0, Out0, GL0, D0>,
                            cell<In1, Out1, GL1, D1>,
                            cell<In2, Out2, GL2, D2>>;

            using updated_stack = mpl::call<mpl::push_front<local_wires_in_children>, RenameFrames...>;
            using updated_rename_counter = mpl::eager::plus<mpl::call<mpl::unpack<mpl::size<>>, local_wires_in_children>, RenameCounter>;

            using type =
                    assembly_flattening_state<
                            mpl::list<
                                    typename rename_assembly_cell::template f<cell<In0, Out0, GL0, D0>, updated_stack>,
                                    typename rename_assembly_cell::template f<cell<In1, Out1, GL1, D1>, updated_stack>,
                                    typename rename_assembly_cell::template f<cell<In2, Out2, GL2, D2>, updated_stack>>,
                            updated_rename_counter>;
        };

        template <
                template <class...> class RenameStack, class... RenameFrames,
                class RenameCounter,
                class In0, class Out0, class GL0, class D0,
                class In1, class Out1, class GL1, class D1,
                class In2, class Out2, class GL2, class D2,
                class In3, class Out3, class GL3, class D3
        >
        struct flatten_assembly_elements<RenameStack<RenameFrames...>, RenameCounter,
                cell<In0, Out0, GL0, D0>,
                cell<In1, Out1, GL1, D1>,
                cell<In2, Out2, GL2, D2>,
                cell<In3, Out3, GL3, D3>
        > {
            using local_wires_in_children =
                    mpl::call<
                            get_local_wires<make_rename_frame_for_wires<RenameCounter>>,
                            cell<In0, Out0, GL0, D0>,
                            cell<In1, Out1, GL1, D1>,
                            cell<In2, Out2, GL2, D2>,
                            cell<In3, Out3, GL3, D3>>;

            using updated_stack = mpl::call<mpl::push_front<local_wires_in_children>, RenameFrames...>;
            using updated_rename_counter = mpl::eager::plus<mpl::call<mpl::unpack<mpl::size<>>, local_wires_in_children>, RenameCounter>;

            using type =
                    assembly_flattening_state<
                            mpl::list<
                                    typename rename_assembly_cell::template f<cell<In0, Out0, GL0, D0>, updated_stack>,
                                    typename rename_assembly_cell::template f<cell<In1, Out1, GL1, D1>, updated_stack>,
                                    typename rename_assembly_cell::template f<cell<In2, Out2, GL2, D2>, updated_stack>,
                                    typename rename_assembly_cell::template f<cell<In3, Out3, GL3, D3>, updated_stack>>,
                            updated_rename_counter>;
        };

        template <>
        struct assembly_flattener<true> {
            template <class Assembly, class RenameStack, class RenameCounter>
            using f = typename Assembly::template f<mpl::cfe<flatten_assembly_elements>, RenameStack, RenameCounter>::type;
        };

        template <>
        struct assembly_flattener<false> {
            template <class Cell, class RenameStack, class RenameCounter>
            using f =
                    assembly_flattening_state<
                            mpl::list<
                                    typename rename_assembly_cell::template f<Cell, RenameStack>>,
                            RenameCounter>;
        };


        struct sentinel_function {
            template <class A>
            using f = A;
        };

        template <class Wire>
        struct mark_as_sentinel;

        template <auto... Xs>
        struct mark_as_sentinel<wire<Xs...>> {
            using type = wire<'S', Xs...>;
        };

        template <class Outputs>
        struct invert_renamed_outputs {
            template <class Rename>
            using is_renamed_output =
                    mpl::call<
                            mpl::unpack<
                                    dm::detail::mpl::set::contains<typename Rename::old_name>>,
                    Outputs>;

            template <class Rename>
            using invert_names = name_change<typename Rename::new_name, typename Rename::old_name>;

            template <class... Renames>
            using f =
                    mpl::call<
                            mpl::filter<mpl::cfe<is_renamed_output>,
                                    mpl::transform<mpl::cfe<invert_names>,
                                            mpl::cfe<rename_stack_frame>>>,
                    Renames...>;
        };

        template <class RenameFrame, class Outputs, class C = mpl::listify>
        struct attach_output_sentinels {
            using inverted_renames = mpl::call<mpl::unpack<invert_renamed_outputs<Outputs>>, RenameFrame>;

            template <class Cell>
            using get_cell_output = mpl::call<mpl::unpack<mpl::front<>>, typename Cell::out>;

            template <class Cell>
            using has_output_connection =
                    mpl::call<
                            mpl::unpack<
                                    dm::detail::mpl::map::contains<get_cell_output<Cell>>>,
                    inverted_renames>;

            template <class OutputWire>
            using make_sentinel_cell = cell<
                    in<OutputWire>,
                    out<typename mark_as_sentinel<OutputWire>::type>,
                    sentinel_function,
                    macro::fixed_delay<time_constant<1>, time_constant<1>>
            >;

            template <class... Cells>
            using attach_sentinels =
                    mpl::call<
                            mpl::fork<
                                    mpl::listify,
                                    mpl::transform<mpl::cfe<get_cell_output>,
                                            dm::detail::mpl::seq::unique<
                                                    mpl::transform<mpl::cfe<make_sentinel_cell>>>>,
                                    mpl::join<>>,
                    Cells...>;

            template <class... Cells>
            using f = mpl::call<
                    mpl::fork<
                            mpl::remove_if<mpl::cfe<has_output_connection>>,
                            mpl::filter<mpl::cfe<has_output_connection>,
                                    mpl::cfe<attach_sentinels>>,
                            mpl::join<>>,
                    Cells...>;
        };


        template <class Wire, class WireIndex, class CellIndex>
        struct wire_index_info {
            using wire = Wire;
            using connection = dm::logic_connection<dm::index_constant<CellIndex::value>, dm::index_constant<WireIndex::value>>;
        };

        template <class WireInfo, class C = mpl::identity>
        struct update_wire_connection_list {
            template <class Wire, class ConnectionList>
            using f = mpl::call<C, mpl::call<mpl::unpack<mpl::push_back<typename WireInfo::connection>>, ConnectionList>>;
        };

        template <class C = mpl::identity>
        struct make_wire_connection_map {
            template <class WireConnectionMap, class WireInfo>
            using update_wire_connection_map =
                    mpl::call<
                            mpl::unpack<
                                    dm::detail::mpl::map::update<
                                            mpl::list<typename WireInfo::wire, mpl::list<typename WireInfo::connection>>,
                                            update_wire_connection_list<WireInfo>>>,
                    WireConnectionMap>;

            template <class... WireInfo>
            using build_connection_map_from_enumerated_wires =
                    mpl::call<
                            mpl::fold_left<
                                    mpl::cfe<update_wire_connection_map>,
                                            C>,
                    mpl::list<>, WireInfo...>;

            template <class Cell, class Index>
            using join_enumerated_cell_inputs =
                    mpl::call<
                            mpl::unpack<
                                    mpl::fork<
                                            mpl::listify,
                                            mpl::size<mpl::make_int_sequence<>>,
                                            mpl::zip_with<
                                                    mpl::push_back<Index,
                                                            mpl::cfe<wire_index_info>>>>>,
                    typename Cell::in>;

            template <class... Cells>
            using f =
                    mpl::call<
                        mpl::fork<
                                mpl::listify,
                                mpl::size<mpl::make_int_sequence<>>,
                                mpl::zip_with<mpl::cfe<join_enumerated_cell_inputs>,
                                        mpl::join<mpl::cfe<build_connection_map_from_enumerated_wires>>>>,
                    Cells...>;
        };


        template <class Delay>
        struct expand_delay_macro;

        template <class RiseTime, class FallTime>
        struct expand_delay_macro<dm::macro::fixed_delay<RiseTime, FallTime>> {
            using type = dm::fixed_delay<RiseTime, FallTime>;
        };

        template <class... PinDelayEntries>
        struct expand_delay_macro<dm::macro::path_dependant_delay<PinDelayEntries...>> {
//            using type =
        };

        template <class... PathDelayEntries>
        struct expand_delay_macro<dm::macro::state_dependant_delay<PathDelayEntries...>> {
//            using type =
        };

        template <class CellIndexMap, class DelayMap>
        struct delay_data {
            using cell_index_map = CellIndexMap;

            using delay_map = DelayMap;
        };

        template <class C = mpl::identity>
        struct delay_data_generator {

            template <class Cell>
            using compiled_delay_from_cell = typename expand_delay_macro<typename Cell::delay>::type;

            template <class Cell>
            using delay_from_cell = typename Cell::delay;

            template <class Delay, class Index>
            using cell_index_delay_entry = mpl::list<Delay, Index>;

            using make_cell_index_map =
                    mpl::transform<mpl::cfe<delay_from_cell>,
                        dm::detail::mpl::seq::unique<
                                mpl::fork<
                                        mpl::listify,
                                        mpl::size<mpl::make_int_sequence<>>,
                                        mpl::zip_with<mpl::cfe<cell_index_delay_entry>>>>>;

            using make_delay_map = mpl::transform<mpl::cfe<compiled_delay_from_cell>>;

            template <class... Cells>
            using f = mpl::call<mpl::fork<make_cell_index_map, make_delay_map, mpl::cfe<delay_data>>, Cells...>;
        };
    }

    template <class Netlist, class DelayMap, class ConnectionMap, class WireRenames>
    struct compiled_module {
        using netlist = Netlist;
        using delay_map = DelayMap;
        using connections = ConnectionMap;
        using renames = WireRenames;
    };

    template <class Module>
    struct module_compiler {
        using start_frame = mpl::call<detail::join_element_inputs_and_outputs<detail::make_rename_frame_for_wires<mpl::uint_<0>>>, Module>;
        using start_index = mpl::call<mpl::unpack<mpl::size<>>, start_frame>;

        using initial_flattened = typename detail::assembly_flattener<Module::compound>::template f<Module, mpl::list<start_frame>, start_index>::new_cells;

        using flattened = mpl::call<mpl::unpack<detail::attach_output_sentinels<start_frame, typename Module::out>>, initial_flattened>;

        using connection_map = mpl::call<mpl::unpack<detail::make_wire_connection_map<>>, flattened>;

        using delay_data = mpl::call<mpl::unpack<detail::delay_data_generator<>>, flattened>;

        template <class Cell>
        using fanout_for_cell =
                mpl::call<
                        mpl::unpack<
                                dm::detail::mpl::map::branch<mpl::call<mpl::unpack<mpl::front<>>, typename Cell::out>,
                                        mpl::identity,
                                        mpl::always<mpl::list<>>>>,
                connection_map>;

        template <class Cell>
        using netlist_element_from_cell = dm::netlist_element<
                dm::index_constant<mpl::call<mpl::unpack<dm::detail::mpl::map::get<typename Cell::delay>>, typename delay_data::cell_index_map>::value>,
                mpl::call<mpl::unpack<mpl::transform<mpl::always<dm::logic_constant<dm::logic::X>>>>, typename Cell::in>,
                dm::logic_constant<dm::logic::X>,
                fanout_for_cell<Cell>,
                typename Cell::gate_logic
        >;

        using netlist =
                mpl::call<
                        mpl::unpack<
                                mpl::transform<mpl::cfe<netlist_element_from_cell>>>,
                flattened>;

        using type = compiled_module<netlist, typename delay_data::delay_map, connection_map, start_frame>;
    };

    template <class Module>
    using compile = typename module_compiler<Module>::type;
}

#endif //DIMETA_MACRO_COMPILER_HPP
