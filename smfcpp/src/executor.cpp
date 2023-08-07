#include "executor.hpp"
#include <iostream>

using namespace smfcpp;

MultiThreadExecutor::MultiThreadExecutor(
    size_t n_threads,
    std::chrono::nanoseconds timeout)
: m_next_exec_timeout(timeout)
{
    m_n_threads = n_threads > 0 ?
        n_threads :
        std::max(std::thread::hardware_concurrency(), 2U);

    if(m_n_threads == 1){
        std::cerr << "Warning : MultiThreadExecutor is used with a single thread.\n"
        << "Use the SingleThreadExecutor instead." << std::endl;
    }
}

void 
MultiThreadExecutor::spin()
{
    if(m_spinning.exchange(true)){
        throw std::runtime_error("spin() called while already spinning");
    }
    activate_callback_groups();
    std::vector<std::thread> threads;
    size_t thread_id = 0;
    {
        std::lock_guard<std::mutex> lock(m_wait_mutex);
        for(; thread_id < m_n_threads - 1; ++ thread_id) {
            auto func = std::bind(&MultiThreadExecutor::run, this, thread_id);
            threads.emplace_back(func);
        }
    }

    run(thread_id);
    for(auto & thread : threads){
        thread.join();
    }
}

void
MultiThreadExecutor::run(size_t this_thread_number)
{
    (void)this_thread_number;
    while(m_spinning.load()){
        smfcpp::AnyExecutable any_exec;
        {
            std::lock_guard<std::mutex> lock(m_wait_mutex);
            if(!m_spinning.load()){
                return;
            }
            if(!get_next_executable(any_exec, m_next_exec_timeout)){
                continue;
            }
        }
        execute_any_executable(any_exec);
    }
}

SingleThreadedExecutor::SingleThreadedExecutor(
    std::chrono::nanoseconds timeout)
: m_next_exec_timeout(timeout)
{}

void 
SingleThreadedExecutor::spin()
{
    if(m_spinning.exchange(true)){
        throw std::runtime_error("spin() called while already spinning");
    }
    activate_callback_groups();
    while(m_spinning.load()){ //smfcpp::ok(this->m_context)
        smfcpp::AnyExecutable any_executable;
        if(get_next_executable(any_executable, m_next_exec_timeout)){
            execute_any_executable(any_executable);
        }
    }
}

Executor::Executor()
: m_spinning(false)
{}

bool
Executor::get_next_executable(
    AnyExecutable & any_executable,
    std::chrono::nanoseconds timeout)
{
    bool success = false;

    success = get_next_ready_executable(any_executable);
    if(!success){
        std::this_thread::sleep_for(timeout);
        if(m_spinning.load()){
            return false;
        }
        success = get_next_ready_executable(any_executable);
    }
    return success;
}


bool
Executor::get_next_ready_executable(AnyExecutable & any_executable)
{   
    bool success = false;
    for(auto & weak_group : o_callback_groups){
        auto group = weak_group.lock();
        if(group){
            // get ready timer
            if(!success){
                any_executable.timer = group->get_ready_timer();
                if(any_executable.timer){
                    any_executable.callback_group = group;
                    success = true;
                }
            }

            // get ready recver
            if(!success){
                any_executable.recver = group->get_ready_recver();
                if(any_executable.recver){
                    any_executable.callback_group = group;
                    success = true;
                }
            }

            // get other
        }
        if(success){
            break;
        }
    }
    return success;
}

void 
Executor::add_node(
    smfcpp::Node::SharedPtr node, 
    bool notify)
{   
    this->add_node(node->get_node_base_interfase(), notify);
}

void
Executor::add_node(
    smfcpp::node_interface::NodeBaseInterface::SharedPtr node_base, 
    bool notify)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    node_base->for_each_callback_group(
        [this, node_base, notify](
            smfcpp::CallbackGroup::SharedPtr group)
        {
            o_callback_groups.push_back(group);
        }
    );
}


void
Executor::execute_any_executable(AnyExecutable & any_exec)
{
    if(!m_spinning.load()){
        return;
    }
    if(any_exec.timer){
        execute_timer(any_exec.timer);
    }
    if(any_exec.recver){
        execute_recver(any_exec.recver);
    }
}

void
Executor::execute_timer(smfcpp::TimerBase::SharedPtr timer)
{
    timer->execute_callback();
}

void
Executor::execute_recver(smfcpp::RecverBase::SharedPtr recver)
{
    recver->execute_callback();
}

bool
Executor::is_spinning()
{
    return m_spinning;
}

void
Executor::activate_callback_groups()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for(auto & weak_group : o_callback_groups){
        auto group = weak_group.lock();
        if(group){
            group->activate();
        }
    }
}