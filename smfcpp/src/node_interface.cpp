#include "node_interface.hpp"

using namespace smfcpp::node_interface;

NodeRecver::NodeRecver(
    smfcpp::node_interface::NodeBaseInterface::SharedPtr node_base)
: o_node_base(node_base)
{}

NodeTimer::NodeTimer(
    smfcpp::node_interface::NodeBaseInterface::SharedPtr node_base)
: o_node_base(node_base)
{}

NodeBase::NodeBase(
    const std::string & node_name,
    smfcpp::Context::SharedPtr context,
    smfcpp::CallbackGroup::SharedPtr default_callback_group)
: m_context(context),
  m_default_callback_group(default_callback_group)
{
    // node_init()
    if(m_default_callback_group == nullptr){
        using smfcpp::CallbackGroupType;
        m_default_callback_group = this->create_callback_group(CallbackGroupType::Exclusive);
    }
}

smfcpp::CallbackGroup::SharedPtr
NodeBase::create_callback_group(
    smfcpp::CallbackGroupType group_type)
{
    auto group = smfcpp::CallbackGroup::make_shared(group_type);
    std::lock_guard<std::mutex> lock(m_callback_groups_mutex);
    m_callback_groups.push_back(group);
    return group;
}

smfcpp::Context::SharedPtr
NodeBase::get_context()
{
    return m_context;
}


bool 
NodeBase::callback_group_in_node(
    smfcpp::CallbackGroup::SharedPtr group)
{
    std::lock_guard<std::mutex> lock(m_callback_groups_mutex);
    for(auto & weak_group : m_callback_groups){
        auto cur_group = weak_group.lock();
        if(cur_group && cur_group == group){
            return true;
        }
    }
    return false;
}

smfcpp::CallbackGroup::SharedPtr
NodeBase::get_default_callback_group()
{
    return m_default_callback_group;
}

void
NodeBase::for_each_callback_group(
    const CallbackGroupFunction & func)
{
    std::lock_guard<std::mutex> lock(m_callback_groups_mutex);
    for(auto & weak_group : m_callback_groups){
        smfcpp::CallbackGroup::SharedPtr group = weak_group.lock();
        if(group){
            func(group);
        }
    }
}

void 
NodeTimer::add_timer(
    smfcpp::TimerBase::SharedPtr timer,
    smfcpp::CallbackGroup::SharedPtr callback_group)
{
    if(callback_group){
        if(!o_node_base->callback_group_in_node(callback_group)){
            throw std::runtime_error("Cannot create timer, group not in node.");
        }
    }
    else{
        callback_group = o_node_base->get_default_callback_group();
    }
    callback_group->add_timer(timer);
}

void 
NodeRecver::add_recver(
    smfcpp::RecverBase::SharedPtr recver,
    smfcpp::CallbackGroup::SharedPtr callback_group)
{
    if(callback_group){
        if(!o_node_base->callback_group_in_node(callback_group)){
            throw std::runtime_error("Cannot create timer, group not in node.");
        }
    }
    else{
        callback_group = o_node_base->get_default_callback_group();
    }
    callback_group->add_recver(recver);
}