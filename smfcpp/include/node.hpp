#ifndef SMFCPP__NODE_HPP_
#define SMFCPP__NODE_HPP_

#include <string>
#include <vector>
#include <functional>
#include <chrono>
#include <thread>

#include "macros.hpp"
#include "node_interface.hpp"

namespace smfcpp
{


class Node: public std::enable_shared_from_this<Node>
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(Node)
    explicit Node(const std::string & node_name);
    virtual ~Node() = default;

    template<typename DurationRepT = int64_t, typename DurationT = std::milli, typename CallbackT>
    typename smfcpp::Timer<CallbackT>::SharedPtr
    create_timer(
        std::chrono::duration<DurationRepT, DurationT> period, 
        CallbackT callback,
        smfcpp::CallbackGroup::SharedPtr group = nullptr);

    template<typename CheckFuncT, typename CallbackT>
    typename smfcpp::Recver<CheckFuncT, CallbackT>::SharedPtr
    create_recver(CheckFuncT check_func, 
        CallbackT callback,
        smfcpp::CallbackGroup::SharedPtr group = nullptr);
    

    const char * get_name() const;

    smfcpp::node_interface::NodeBaseInterface::SharedPtr
    get_node_base_interfase();
    
private:
    SMFCPP_DISABLE_COPY(Node)
    
    const std::string m_name;

    smfcpp::node_interface::NodeBaseInterface::SharedPtr m_node_base;
    smfcpp::node_interface::NodeTimerInterface::SharedPtr m_node_timers;
    smfcpp::node_interface::NodeRecverInterface::SharedPtr m_node_recvers;
};

}

#ifndef RCLCPP__NODE_IMPL_HPP_
// Template implementations
#include "node_impl.hpp"
#endif

#endif