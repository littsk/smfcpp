#ifndef SMFCPP__CALLBACK_GROUP_HPP_
#define SMFCPP__CALLBACK_GROUP_HPP_

#include <mutex>
#include <algorithm>

#include "timer.hpp"
#include "recver.hpp"
#include "macros.hpp"

namespace smfcpp{

namespace node_interface{
    class NodeTimer;
    class NodeRecver;
}

enum class CallbackGroupType
{
    Exclusive,
    Reentrant
};

class CallbackGroup
{
    friend class smfcpp::node_interface::NodeTimer;
    friend class smfcpp::node_interface::NodeRecver;
    
public:
    SMFCPP_SMART_PTR_DEFINITIONS(CallbackGroup)
    explicit CallbackGroup(
        CallbackGroupType group_type);

    virtual
    ~CallbackGroup() = default;

    void activate();

    smfcpp::TimerBase::SharedPtr 
    get_ready_timer();

    smfcpp::RecverBase::SharedPtr
    get_ready_recver();

protected:
    CallbackGroupType m_type;
    mutable std::mutex m_mutex;

    std::vector<smfcpp::TimerBase::WeakPtr> o_timers;
    std::vector<smfcpp::RecverBase::WeakPtr> o_recvers;

    void
    add_timer(smfcpp::TimerBase::SharedPtr timer);

    void
    add_recver(smfcpp::RecverBase::SharedPtr recver);

};


}

#endif