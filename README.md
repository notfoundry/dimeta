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

While individual headers for different components do exist, the easiest way to get started with dimeta is to pull in the mega-header.
```C++
#include <dimeta/dimeta.hpp>
```

Having done that, both the simulation and compilation machinery will be available in the `dm` and `dm::macro` namespaces respectively.
Since interacting with the netlist, scheduling and delay datatypes directly can get tedious, it’s generally recommended to use the macro machinery to create the gate systems for simulation. Here’s a basic AND gate to demonstrate.
```C++
namespace dmm = dm::macro;

template <class A, class B, class O>
using unit_and_cell = dmm::cell<
        dmm::in<A, B>,
        dmm::out<O>,
        dm::function::and_function,
        dmm::fixed_delay<dm::time_constant<1>, dm::time_constant<1>>
>;
```

Primitive gates are modeled through the `cell` type, which specifies a set of input wires, a output wire, the logic function used to compute the result of applying a set of inputs, and the delays for the gate operations. This last item raises an important point: delays for primitives must be given up-front. When cells are combined to form more complicated systems, the timing characteristics of those systems are derived from the delays of the primitive components.
A cell alone isn’t enough to construct a system, however. A set of concrete inputs and outputs also have to exist for simulation to proceed, an example of which can be provided as in the case of the next example.  

```C++
template <class A, class B, class O>
using example_assembly = dmm::assembly<
        dmm::in<A, B>,
        dmm::out<O>,
        unit_and_cell<A*, B*, O*>
>;

using compiled_and = dmm::compile<example_assembly<dmm::wire<'A'>, dmm::wire<'B'>, dmm::wire<'C'>>>;
```

```C++
using gate_simulation = dmm::simulate<
        compiled_and,
        dmm::initial_conditions<
                dmm::state_assignment<dmm::wire<'A'>, dm::logic::H>,
                dmm::state_assignment<dmm::wire<'B'>, dm::logic::H>,
        >,
        dmm::monitor_states<dmm::wire<'C'>>
>;
```
