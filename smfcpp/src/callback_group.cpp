#include "callback_group.hpp"


namespace smfcpp{

CallbackGroup::CallbackGroup(
    CallbackGroupType group_type)
: m_type(group_type)
{}


void
CallbackGroup::activate()
{
    std::lock_guard<std::mutex> lock(m_mutex);

    for(auto & weak_timer : o_timers){
        auto timer = weak_timer.lock();
        if(timer){
            timer->start();
        }
    }
    for(auto & weak_recver : o_recvers){
        auto recver = weak_recver.lock();
        if(recver){
            recver->start();
        }
    }
}
void 
CallbackGroup::add_timer(
    TimerBase::SharedPtr timer)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    o_timers.push_back(timer);
    o_timers.erase(
        std::remove_if(
            o_timers.begin(),
            o_timers.end(),
            [](smfcpp::TimerBase::WeakPtr timer){ return timer.expired(); }),
        o_timers.end());
}

smfcpp::TimerBase::SharedPtr
CallbackGroup::get_ready_timer(){
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto & weak_timer : o_timers){
        auto timer = weak_timer.lock();
        if(timer->get_ready()){
            return timer;
        }
    }
    return nullptr;
}

void 
CallbackGroup::add_recver(smfcpp::RecverBase::SharedPtr recver)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    o_recvers.push_back(recver);
    o_recvers.erase(
        std::remove_if(
            o_recvers.begin(),
            o_recvers.end(),
            [](smfcpp::RecverBase::WeakPtr recver){ return recver.expired(); }),
        o_recvers.end());
}

smfcpp::RecverBase::SharedPtr
CallbackGroup::get_ready_recver()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto & weak_recver : o_recvers){
        auto recver = weak_recver.lock();
        if(recver->get_ready()){
            return recver;
        }
    }
    return nullptr;
}

}