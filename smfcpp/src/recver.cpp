#include "recver.hpp"

using smfcpp::RecverBase;

RecverBase::RecverBase(
    smfcpp::Context::SharedPtr context)
{
    (void)context;
}

bool RecverBase::get_ready()
{
    bool res = m_ready.exchange(false);
    return res;
}

void RecverBase::start()
{
    std::thread(std::bind(&RecverBase::listen, this)).detach();
}