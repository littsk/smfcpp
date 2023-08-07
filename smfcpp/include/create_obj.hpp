#ifndef SMFCPP__CREATE_OBJ_HPP_
#define SMFCPP__CREATE_OBJ_HPP_

#include <chrono>

#include "macros.hpp"
#include "node_interface.hpp"
#include "timer.hpp"

namespace smfcpp
{

namespace detail
{

    /**
     * \tparam DurationRepT
     * \tparam DurationT
     * \param period: std::chrono::duration
     * \return period: std::chrono::nanosecoond
     * \throws std::invalid_argument if period is nagative or too large
     */
    template <typename DurationRepT, typename DurationT>
    std::chrono::nanoseconds
    safe_cast_to_period_in_ns(std::chrono::duration<DurationRepT, DurationT> period)
    {
        if (period < std::chrono::duration<DurationRepT, DurationT>::zero())
        {
            throw std::invalid_argument{"timer period cannot be negative"};
        }

        constexpr auto maximum_safe_cast_ns = 
            std::chrono::nanoseconds::max() - std::chrono::duration<DurationRepT, DurationT>(1);

        constexpr auto ns_max_as_double =
            std::chrono::duration_cast<std::chrono::duration<double, std::chrono::nanoseconds::period>>(
            maximum_safe_cast_ns);

        if(period > ns_max_as_double){
            throw std::invalid_argument{
                "timer period must be less than std::chrono::nanoseconds::max()"
            };
        }

        const auto period_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(period);
        if(period_ns < std::chrono::nanoseconds::zero()){
            throw std::runtime_error{
                "casting timer period to nanoseconds resulted in integer overflow"
            };
        }

        return period_ns;
    }
}

template <typename DurationRepT, typename DurationT, typename CallbackT>
typename Timer<CallbackT>::SharedPtr
create_timer(
    std::chrono::duration<DurationRepT, DurationT> period, 
    CallbackT && callback,
    smfcpp::CallbackGroup::SharedPtr group,
    node_interface::NodeBaseInterface::SharedPtr node_base,
    node_interface::NodeTimerInterface::SharedPtr node_timers)
{

    if (node_base == nullptr) {
        throw std::invalid_argument{"input node_base cannot be null"};
    }

    if (node_timers == nullptr) {
        throw std::invalid_argument{"input node_timers cannot be null"};
    }

    auto period_ns = smfcpp::detail::safe_cast_to_period_in_ns(period);
    auto timer = Timer<CallbackT>::make_shared(
        period_ns, std::move(callback), node_base->get_context());
    node_timers->add_timer(timer, group);
    return timer;
}


template<typename CheckFuncT, typename CallbackT>
typename smfcpp::Recver<CheckFuncT, CallbackT>::SharedPtr
create_recver(CheckFuncT && check_func, 
    CallbackT && callback,
    smfcpp::CallbackGroup::SharedPtr group,
    node_interface::NodeBaseInterface::SharedPtr node_base,
    node_interface::NodeRecverInterface::SharedPtr node_recvers)
{
    if (node_base == nullptr) {
        throw std::invalid_argument{"input node_base cannot be null"};
    }

    if (node_recvers == nullptr) {
        throw std::invalid_argument{"input node_recvers cannot be null"};
    }

    auto recver = Recver<CheckFuncT, CallbackT>::make_shared(
        std::move(check_func), std::move(callback), node_base->get_context());
    node_recvers->add_recver(recver, group);
    return recver;
}

}

#endif