#ifndef SMFCPP__TIMER_HPP_
#define SMFCPP__TIMER_HPP_

#include <string>
#include <vector>

#include <functional>
#include <chrono>
#include <thread>
#include <atomic>

#include "macros.hpp"
#include "context.hpp"


namespace smfcpp
{

class TimerBase{
public:
    SMFCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(TimerBase)
    explicit TimerBase(
        std::chrono::nanoseconds interval,
        smfcpp::Context::SharedPtr context,
        bool repeat=true);
    virtual ~TimerBase() = default;

    void start();
    
    void stop();

    bool get_ready();

    virtual
    void
    execute_callback() = 0;


private:
    std::atomic<bool> m_ready;
    std::chrono::nanoseconds interval;
    bool m_repeat;
    bool m_running;
};


template<typename CallbackT>
class Timer: public TimerBase
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(Timer)
    explicit Timer(std::chrono::nanoseconds interval, 
        CallbackT && callback, 
        smfcpp::Context::SharedPtr context,
        bool repeat=true)
    : TimerBase(interval, context, repeat),
      m_callback(std::forward<CallbackT>(callback))
    {}

    ~Timer() = default;

    virtual
    void
    execute_callback() override
    {
        m_callback();
    }
    
private:
    CallbackT m_callback;
};

}

#endif