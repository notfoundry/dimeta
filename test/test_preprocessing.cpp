//
// Created by Mark on 9/20/2019.
//

#include <dimeta/primitives.hpp>

#include <dimeta/macro/primitives.hpp>
#include <dimeta/macro/delay.hpp>
#include <dimeta/macro/compiler.hpp>
#include <dimeta/macro/simulator.hpp>

#include <dimeta/function/and_function.hpp>
#include <dimeta/function/or_function.hpp>
#include <dimeta/function/xor_function.hpp>

namespace dmm = dm::macro;

template <class A, class B, class O>
using unit_and = dmm::cell<
        dmm::in<A, B>,
        dmm::out<O>,
        dm::function::and_function,
        dmm::fixed_delay<dm::time_constant<1>, dm::time_constant<1>>
>;

template <class A, class B, class O>
using unit_or = dmm::cell<
        dmm::in<A, B>,
        dmm::out<O>,
        dm::function::or_function,
        dmm::fixed_delay<dm::time_constant<1>, dm::time_constant<1>>
>;

template <class A, class B, class O>
using unit_xor = dmm::cell<
        dmm::in<A, B>,
        dmm::out<O>,
        dm::function::xor_function,
        dm::macro::fixed_delay<dm::time_constant<1>, dm::time_constant<1>>
>;

template <class A, class B, class S, class C>
using unit_half_adder = dmm::assembly<
        dmm::in<A, B>,
        dmm::out<S, C>,

        unit_xor<A*, B*, S*>,
        unit_and<A*, B*, C*>
>;

template <class A, class B, class CIn, class S, class COut>
using unit_full_adder = dmm::assembly<
        dmm::in<A, B, CIn>,
        dmm::out<S, COut>,

        unit_half_adder<A*, B*, dmm::wire<0>, dmm::wire<1>>,
        unit_half_adder<dmm::wire<0>, CIn*, S*, dmm::wire<2>>,
        unit_or<dmm::wire<1>, dmm::wire<2>, COut*>
>;

using adder = unit_full_adder<
        dmm::wire<'A'>, dmm::wire<'B'>, dmm::wire<'C','i','n'>,
        dmm::wire<'S'>, dmm::wire<'C','o','u','t'>
>;

using compiled_adder = dmm::compile<adder>;

using sim = dmm::simulate<
        compiled_adder,
        dmm::initial_conditions<
                dmm::state_assignment<dmm::wire<'A'>, dm::logic::H>,
                dmm::state_assignment<dmm::wire<'B'>, dm::logic::L>,
                dmm::state_assignment<dmm::wire<'C','i','n'>, dm::logic::L>
        >
>;
