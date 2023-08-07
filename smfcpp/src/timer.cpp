#include "timer.hpp"

using smfcpp::TimerBase;

TimerBase::TimerBase(
    std::chrono::nanoseconds interval,
    smfcpp::Context::SharedPtr context,
    bool repeat)
: interval(interval), 
  m_repeat(repeat),
  m_ready(false)
{}


void TimerBase::start()
{
    this->m_running = true;
    std::thread([=](){
        while(this->m_running)
        {
            std::this_thread::sleep_for(interval);
            if(!this->m_running) break;
            this->m_ready.store(true);
            if(!this->m_running) break;
        }
    }).detach();
}

void TimerBase::stop()
{
    this->m_running = false;
}

bool TimerBase::get_ready()
{
    bool res = m_ready.exchange(false);
    return res;
}


