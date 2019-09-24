//
// Created by Mark on 9/22/2019.
//

#ifndef DIMETA_MACRO_DELAY_HPP
#define DIMETA_MACRO_DELAY_HPP

namespace dm::macro {
    template <class RiseTime, class FallTime>
    struct fixed_delay {

    };

    template <class... PinDelayEntries>
    struct path_dependant_delay {

    };

    template <class Wire, class Delay>
    struct path_delay_entry {

    };

    template <class... PathDelayEntries>
    struct state_dependant_delay {

    };

    template <class PinStates, class Delay>
    struct state_delay_entry {

    };
}

#endif //DIMETA_MACRO_DELAY_HPP
