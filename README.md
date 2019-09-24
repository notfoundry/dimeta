<h1 align="center">dimeta</h1>

<div align="center">
  <strong>A full-timing gate-level logic simulator in C++ templates</strong>
</div>
<div align="center">
  | :muscle: Performant design | :recycle: Fully deterministic | :feelsgood: O(1) runtime complexity |
</div>
<br />

## Table of Contents
- [Overview](#overview)
- [Tutorial](#tutorial)

## Overview
`dimeta` is a logic simulation and digital modeling library capable of interpreting complex combinational and sequential circuits with static, path-dependant, or state-dependant timing information at any timescale... **at compile-time, only using types**.

The library is primarily composed of the _simulator_ and the _compiler_. The simulator is an event-driven logic simulation engine that processes scheduled state transitions, applies them to the relevant gates stored in a netlist, and generates future state transition events in a manner specified by the associated delay information specified in a delay map. The compiler facilitates the composition of logic gates in an easy manner, and enables arbitrary gate nesting with connections specified symbolically in the form of `wire` types.

Care has been taken to make the simulation reasonably performant (types manipulated in packs whenever possible, vectorized metafunctions used, etc.) but for anything non-trivial, getting *realll* familiar with your friendly neighbourhood `ftemplate-depth` compiler flag (or equivalent) may be in order. Things can get pretty gnarly!

## Tutorial

```C++
#include <dimeta/dimeta.hpp>

namespace dmm = dm::macro;

template <class A, class B, class O>
using unit_and_cell = dmm::cell<
        dmm::in<A, B>,
        dmm::out<O>,
        dm::function::and_function,
        dmm::fixed_delay<dm::time_constant<1>, dm::time_constant<1>>
>;

template <class A, class B, class O>
using example_assembly = dmm::assembly<
        dmm::in<A, B>,
        dmm::out<O>,
        unit_and_cell<A*, B*, O*>
>;

using compiled_and = dmm::compile<example_assembly<dmm::wire<'A'>, dmm::wire<'B'>, dmm::wire<'C'>>>;

using gate_simulation = dmm::simulate<
        compiled_and,
        dmm::initial_conditions<
                dmm::state_assignment<dmm::wire<'A'>, dm::logic::H>,
                dmm::state_assignment<dmm::wire<'B'>, dm::logic::H>,
        >,
        dmm::monitor_states<dmm::wire<'C'>>
>;
```

