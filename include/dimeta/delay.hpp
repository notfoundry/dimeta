/*
* Created by Mark Johnson on 6/22/2019.
*/

#ifndef DIMETA_DELAY_HPP
#define DIMETA_DELAY_HPP

#include <kvasir/mpl/functional/call.hpp>

#include <dimeta/primitives.hpp>

#include <dimeta/detail/consistency.hpp>
#include <dimeta/detail/mpl/map.hpp>
#include <kvasir/mpl/algorithm/all.hpp>

namespace dm {
    namespace mpl = kvasir::mpl;

    template <class RiseTime, class FallTime>
    struct fixed_delay {
        DIMETA_ASSERT(is_time_constant<RiseTime>::value);
        DIMETA_ASSERT(is_time_constant<FallTime>::value);

        template <class PinID, class NLE>
        using rise_time = DIMETA_TMPL_ASSERT(is_index_constant<PinID>::value, is_netlist_element<NLE>::value)
                (RiseTime);

        template <class PinID, class NLE>
        using fall_time = DIMETA_TMPL_ASSERT(is_index_constant<PinID>::value, is_netlist_element<NLE>::value)
                (FallTime);
    };

    template <class T>
    using is_fixed_delay = mpl::eager::is_instance<fixed_delay, T>;

    template <class... PinDelayEntries>
    struct path_dependant_delay {
//        DIMETA_ASSERT(detail::mpl::map::is_map<PinDelayEntries...>::value);
        DIMETA_ASSERT(mpl::call<detail::mpl::map::keys<mpl::all<mpl::cfe<is_index_constant>>>, PinDelayEntries...>::value);
        DIMETA_ASSERT(mpl::call<detail::mpl::map::values<mpl::all<mpl::cfe<is_fixed_delay>>>, PinDelayEntries...>::value);
    private:
        template <class PinID>
        using pin_delay = mpl::call<detail::mpl::map::get<PinID>, PinDelayEntries...>;
    public:
        template <class PinID, class NLE>
        using rise_time = DIMETA_TMPL_ASSERT(is_index_constant<PinID>::value, is_netlist_element<NLE>::value)
                (typename pin_delay<PinID>::rise_time);

        template <class PinID, class NLE>
        using fall_time = DIMETA_TMPL_ASSERT(is_index_constant<PinID>::value, is_netlist_element<NLE>::value)
                (typename pin_delay<PinID>::fall_time);
    };

    template <class T>
    using is_path_dependant_delay = mpl::eager::is_instance<path_dependant_delay, T>;

    template <class... PathDelayEntries>
    struct state_dependant_delay {
//        DIMETA_ASSERT(detail::mpl::map::is_map<PathDelayEntries...>::value);
        DIMETA_ASSERT(mpl::call<detail::mpl::map::keys<mpl::all<mpl::cfe<mpl::eager::is_list>>>, PathDelayEntries...>::value);
//        check that all key list elements are logic constants
        DIMETA_ASSERT(mpl::call<detail::mpl::map::values<mpl::all<mpl::cfe<is_path_dependant_delay>>>, PathDelayEntries...>::value);
    private:
        template <class PinStates>
        using path_delay = mpl::call<detail::mpl::map::get<PinStates>, PathDelayEntries...>;
    public:
        template <class PinID, class NLE>
        using rise_time = DIMETA_TMPL_ASSERT(is_index_constant<PinID>::value, is_netlist_element<NLE>::value)
                (typename path_delay<typename NLE::inputs>::template rise_time<PinID>);

        template <class PinID, class NLE>
        using fall_time = DIMETA_TMPL_ASSERT(is_index_constant<PinID>::value, is_netlist_element<NLE>::value)
                (typename path_delay<typename NLE::inputs>::template fall_time<PinID>);
    };

    template <class T>
    using is_state_dependant_delay = mpl::eager::is_instance<state_dependant_delay, T>;

    template <class T>
    using is_delay = mpl::bool_<(
            is_fixed_delay<T>::value
            || is_path_dependant_delay<T>::value
            || is_state_dependant_delay<T>::value)>;
}

#endif //DIMETA_DELAY_HPP
