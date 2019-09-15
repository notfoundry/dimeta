/*
* Created by Mark Johnson on 6/22/2019.
*/

#ifndef DIMETA_SCHEDULING_HPP
#define DIMETA_SCHEDULING_HPP

#include <kvasir/mpl/algorithm/partition.hpp>
#include <kvasir/mpl/algorithm/extreme.hpp>
#include <kvasir/mpl/algorithm/transform.hpp>

#include <kvasir/mpl/algorithm/all.hpp>

#include <kvasir/mpl/functional/bind.hpp>
#include <kvasir/mpl/functional/call.hpp>

#include <kvasir/mpl/functions/comparison/equal.hpp>
#include <kvasir/mpl/functions/arithmetic/plus.hpp>

#include <kvasir/mpl/sequence/push_back.hpp>

#include <kvasir/mpl/utility/is_instance.hpp>
#include <kvasir/mpl/utility/always.hpp>

#include <dimeta/detail/consistency.hpp>
#include <dimeta/future.hpp>
#include <dimeta/primitives.hpp>

namespace dm {
    namespace mpl = kvasir::mpl;

    template <class ScheduledTime, class Destination, class Output>
    struct scheduled_event {
        DIMETA_ASSERT(is_time_constant<ScheduledTime>::value);
        DIMETA_ASSERT(is_logic_connection<Destination>::value);
        DIMETA_ASSERT(is_logic_constant<Output>::value);

        using scheduled_time = ScheduledTime;
        using destination = Destination;
        using output = Output;
    };

    template <class T>
    using is_scheduled_event = mpl::eager::is_instance<scheduled_event, T>;


    template <class Time>
    struct is_event_scheduled_for {
        DIMETA_ASSERT(is_time_constant<Time>::value);

        template <class ScheduledEvent>
        using f = mpl::eager::equal<typename ScheduledEvent::scheduled_time, Time>;
    };


    struct terminated_event_scheduler {};

    template <class ScheduledEventsList, class PendingEventsList, class GVT = time_constant<0>>
    struct event_scheduler {
        DIMETA_ASSERT(mpl::eager::all<ScheduledEventsList, is_scheduled_event>::value);
        DIMETA_ASSERT(mpl::eager::all<PendingEventsList, is_scheduled_event>::value);
        DIMETA_ASSERT(is_time_constant<GVT>::value);

    private:
        template <class NewScheduledEventsList, class NewPendingEventsList, class NewGVT>
        using next_scheduler_step = event_scheduler<NewScheduledEventsList, NewPendingEventsList, NewGVT>;

        template <class MinTime>
        using partition_by_deadline = mpl::partition<is_event_scheduled_for<MinTime>, mpl::push_back<MinTime, mpl::cfe<next_scheduler_step>>>;

        template <class LHS, class RHS>
        using min_time = mpl::call<mpl::conditional<LHS::scheduled_time::value <= RHS::scheduled_time::value>, LHS, RHS>;

        template <class... CombinedEvents>
        using get_min_scheduled_time = typename mpl::call<mpl::extreme<mpl::cfe<min_time>>, CombinedEvents...>::scheduled_time;

        template <class... CombinedEvents>
        using do_scheduling_step = mpl::call<partition_by_deadline<get_min_scheduled_time<CombinedEvents...>>, CombinedEvents...>;

        template <class... CombinedEvents>
        using step_scheduler_or_terminate = mpl::call<
            mpl::call<
                mpl::conditional<sizeof...(CombinedEvents) == 0>,
                mpl::always<terminated_event_scheduler>,
                mpl::cfe<do_scheduling_step>
            >,
            CombinedEvents...
        >;

        template <class... NewPendingEvents>
        using join_current_pending_events = mpl::call<
            mpl::unpack<mpl::cfe<step_scheduler_or_terminate>>,
            ScheduledEventsList, NewPendingEvents...
        >;

        template <class FutureEvent>
        using schedule_event = scheduled_event<
            mpl::eager::plus<GVT, typename FutureEvent::gvt_offset>,
            typename FutureEvent::destination,
            typename FutureEvent::output
        >;

    public:
        template <class... FutureEvents>
        using f = DIMETA_TMPL_ASSERT(mpl::call<mpl::all<mpl::cfe<is_future_event>>, FutureEvents...>::value)
                (mpl::call<mpl::transform<mpl::cfe<schedule_event>, mpl::cfe<join_current_pending_events>>, FutureEvents...>);

        using pending_events = PendingEventsList;
    };

    template <class T>
    using is_event_scheduler = mpl::eager::is_instance<event_scheduler, T>;
}

#endif //DIMETA_SCHEDULING_HPP
