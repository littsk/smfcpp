#ifndef SMFCPP__RECVER_HPP_
#define SMFCPP__RECVER_HPP_

#include "macros.hpp"
#include "context.hpp"

#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>

namespace smfcpp
{

class RecverBase
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(RecverBase)
    explicit RecverBase(
        smfcpp::Context::SharedPtr context
    );
    virtual ~RecverBase() = default;

    virtual
    void
    listen() = 0;

    virtual
    void
    execute_callback() = 0;

    void start();

    bool
    get_ready();

protected:
    std::atomic<bool> m_ready;
    std::condition_variable m_condv;
    std::mutex m_mutex;
};

template<typename CheckFuncT, typename CallbackT>
class Recver: public RecverBase
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(Recver)
    explicit Recver(
        CheckFuncT && check_func, 
        CallbackT && callback,
        smfcpp::Context::SharedPtr context)
    : m_check_func(std::forward<CheckFuncT>(check_func)),
      m_callback(std::forward<CallbackT>(callback)),
      RecverBase(context)
    {}
    virtual ~Recver() = default;

    virtual
    void
    listen() override
    {
        while(1){
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                m_condv.wait(lock, [=]{ return !this->m_ready.load(); } );
            }
            if(m_check_func()){
                m_ready.store(true);
            }
        }
        
    }

    virtual
    void
    execute_callback() override
    {
        m_callback();
        m_condv.notify_one();
    }



private:
    CheckFuncT m_check_func;
    CallbackT m_callback;
};

}

#endif
