#ifndef SMFCPP__NODE_IMPL_HPP_
#define SMFCPP__NODE_IMPL_HPP_

#include <chrono>
#include <type_traits>

#include "create_obj.hpp"

#ifndef SMFCPP__NODE__HPP_
#include "node.hpp"
#endif

namespace smfcpp{

template<typename DurationRepT, typename DurationT, typename CallbackT>
typename smfcpp::Timer<CallbackT>::SharedPtr
Node::create_timer(
    std::chrono::duration<DurationRepT, DurationT> period, 
    CallbackT callback,
    smfcpp::CallbackGroup::SharedPtr group)
{
    return smfcpp::create_timer(
        period,
        std::move(callback),
        group,
        m_node_base,
        m_node_timers);
}


template<typename CheckFuncT, typename CallbackT>
typename smfcpp::Recver<CheckFuncT, CallbackT>::SharedPtr
Node::create_recver(CheckFuncT check_func, 
    CallbackT callback,
    smfcpp::CallbackGroup::SharedPtr group)
{
    static_assert(std::is_invocable_v<CheckFuncT>, "check_func must be invocable");
    static_assert(std::is_invocable_v<CallbackT>, "callback must be invocable");

    using RetType = std::invoke_result_t<CheckFuncT>;

    if(!std::is_same_v<RetType, bool>){
        throw std::invalid_argument("check_func return value is not a bool");
    }

    return smfcpp::create_recver(
        std::move(check_func),
        std::move(callback),
        group,
        m_node_base,
        m_node_recvers);
}

}


#endif