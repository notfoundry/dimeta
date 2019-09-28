//
// Created by Mark on 9/22/2019.
//

#ifndef DIMETA_MACRO_COMPILER_HPP
#define DIMETA_MACRO_COMPILER_HPP

#include <kvasir/mpl/algorithm/transform.hpp>
#include <kvasir/mpl/algorithm/zip_with.hpp>
#include <kvasir/mpl/algorithm/remove_if.hpp>
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
        struct name_change {};

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

        template <class Module, class RenameStack, class RenameCounter>
        struct assembly_flattener;

        template <class In, class Out, class... Elements, class RenameStack, class RenameCounter>
        struct assembly_flattener<assembly<In, Out, Elements...>, RenameStack, RenameCounter> {
            using local_wires_in_children = mpl::call<get_local_wires<make_rename_frame_for_wires<RenameCounter>>, Elements...>;

            using updated_stack = mpl::call<mpl::unpack<mpl::push_front<local_wires_in_children>>, RenameStack>;
            using updated_rename_counter = mpl::eager::plus<mpl::call<mpl::unpack<mpl::size<>>, local_wires_in_children>, RenameCounter>;

            template <class Element>
            using process_element = typename assembly_flattener<Element, updated_stack, updated_rename_counter>::type;

            using type = mpl::call<mpl::transform<mpl::cfe<process_element>, mpl::join<>>, Elements...>;
        };

        template <class In, class Out, class GateLogic, class Delay, class RenameStack, class RenameCounter>
        struct assembly_flattener<cell<In, Out, GateLogic, Delay>, RenameStack, RenameCounter> {
            template <class Wires, class C>
            using translate = mpl::call<mpl::unpack<mpl::transform<translate_wire<RenameStack>, C>>, Wires>;

            using new_in = translate<In, mpl::cfe<dm::macro::in>>;
            using new_out = translate<Out, mpl::cfe<dm::macro::out>>;
            using new_delay = Delay;
            using new_cell = cell<new_in, new_out, GateLogic, new_delay>;

            using type = mpl::list<new_cell>;
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
                                        mpl::zip_with<mpl::push_back<Index,
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

    template <class Netlist, class DelayMap, class ConnectionMap, class FlattenedCells, class WireRenames>
    struct compiled_module {
        using netlist = Netlist;
        using delay_map = DelayMap;
        using connections = ConnectionMap;
        using cells = FlattenedCells;
        using renames = WireRenames;
    };

    template <class Module>
    struct module_compiler {
        using start_frame = mpl::call<detail::join_element_inputs_and_outputs<detail::make_rename_frame_for_wires<mpl::uint_<0>>>, Module>;
        using start_index = mpl::call<mpl::unpack<mpl::size<>>, start_frame>;

        using flattened = typename detail::assembly_flattener<Module, mpl::list<start_frame>, start_index>::type;

        using connection_map = mpl::call<mpl::unpack<detail::make_wire_connection_map<>>, flattened>;

        using delay_data = mpl::call<mpl::unpack<detail::delay_data_generator<>>, flattened>;

        template <class Cell>
        using fanout_for_cell =
                mpl::call<
                        mpl::unpack<
                                dm::detail::mpl::map::get<mpl::call<mpl::unpack<mpl::front<>>, typename Cell::out>,
                                        mpl::if_<mpl::same_as<void>, mpl::always<mpl::list<>>, mpl::identity>>>,
                connection_map>;

        template <class Cell, class Index>
        using netlist_element_from_cell = dm::netlist_element<
                dm::index_constant<mpl::call<mpl::unpack<dm::detail::mpl::map::get<typename Cell::delay>>, typename delay_data::cell_index_map>::value>,
                mpl::call<mpl::unpack<mpl::transform<mpl::always<dm::logic_constant<dm::logic::X>>>>, typename Cell::in>,
                dm::logic_constant<dm::logic::X>,
                fanout_for_cell<Cell>,
                typename Cell::gate_logic
        >;

        using netlist =
                mpl::call<
                        mpl::unpack<mpl::fork<
                                mpl::listify,
                                mpl::size<mpl::make_int_sequence<>>,
                                mpl::zip_with<
                                        mpl::cfe<netlist_element_from_cell>>>>,
                flattened>;

        using type = compiled_module<netlist, typename delay_data::delay_map, connection_map, flattened, start_frame>;
    };

    template <class Module>
    using compile = typename module_compiler<Module>::type;
}

#endif //DIMETA_MACRO_COMPILER_HPP
