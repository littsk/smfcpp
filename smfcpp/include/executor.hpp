/// Coordinate the order and timing of available communication tasks.
/**
 * Executor provides spin functions (including spin_node_once and spin_some).
 * It coordinates the nodes and callback groups by looking for available work and completing it,
 * based on the threading or concurrency scheme provided by the subclass implementation.
 * An example of available work is executing a subscription callback, or a timer callback.
 * The executor structure allows for a decoupling of the communication graph and the execution
 * model.
 * See SingleThreadedExecutor and MultiThreadedExecutor for examples of execution paradigms.
 */
#ifndef SMFCPP__EXECUTOR_HPP_
#define SMFCPP__EXECUTOR_HPP_

#include <atomic>

#include "macros.hpp"
#include "node.hpp"

#define NEXT_EXEC_TIMEOUT 1000000 // time for wait next exec (nanoseconds)

namespace smfcpp
{

struct AnyExecutable
{
    AnyExecutable() = default;
    virtual ~AnyExecutable() = default;

    smfcpp::TimerBase::SharedPtr timer;
    smfcpp::RecverBase::SharedPtr recver;
    smfcpp::CallbackGroup::SharedPtr callback_group;
    smfcpp::node_interface::NodeBaseInterface::SharedPtr node_base;
};

class Executor
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS_NOT_COPYABLE(Executor)
    explicit Executor();
    virtual ~Executor() = default;

    /// Do work periodically as it becomes available to us. Blocking call, may block indefinitely.
    // It is up to the implementation of Executor to implement spin.
    virtual void spin() = 0;

    /// Add a callback group to an executor.
    /**
     * An executor can have zero or more callback groups which provide work during `spin` functions.
     * When an executor attempts to add a callback group, the executor checks to see if it is already
     * associated with another executor, and if it has been, then an exception is thrown.
     * Otherwise, the callback group is added to the executor.
     *
     * Adding a callback group with this method does not associate its node with this executor
     * in any way
     *
     * \param[in] group_ptr a shared ptr that points to a callback group
     * \param[in] node_ptr a shared pointer that points to a node base interface
     * \param[in] notify True to trigger the interrupt guard condition during this function. If
     * the executor is blocked at the rmw layer while waiting for work and it is notified that a new
     * callback group was added, it will wake up.
     * \throw std::runtime_error if the callback group is associated to an executor
     */
    // virtual void
    // add_callback_group(
    //     smfcpp::CallbackGroup::SharedPtr group_ptr,
    //     smfcpp::node_interface::NodeBaseInterface::SharedPtr node_ptr,
    //     bool notify = true);

    /// Add a node to the executor.
    /**
     * Nodes have associated callback groups, and this method adds any of those callback groups
     * to this executor which have their automatically_add_to_executor_with_node parameter true.
     * The node is also associated with the executor so that future callback groups which are
     * created on the node with the automatically_add_to_executor_with_node parameter set to true
     * are also automatically associated with this executor.
     *
     * Callback groups with the automatically_add_to_executor_with_node parameter set to false must
     * be manually added to an executor using the rclcpp::Executor::add_callback_group method.
     *
     * If a node is already associated with an executor, this method throws an exception.
     *
     * \param[in] node_ptr Shared pointer to the node to be added.
     * \param[in] notify True to trigger the interrupt guard condition during this function. If
     * the executor is blocked at the rmw layer while waiting for work and it is notified that a new
     * node was added, it will wake up.
     * \throw std::runtime_error if a node is already associated to an executor
     */
    virtual void
    add_node(smfcpp::node_interface::NodeBaseInterface::SharedPtr node_base, bool notify = true);

    /// Convenience function which takes Node and forwards NodeBaseInterface.
    /**
     * \see rclcpp::Executor::add_node
     */
    virtual void
    add_node(smfcpp::Node::SharedPtr node, bool notify = true);

    /// Collect and execute work repeatedly within a duration or until no more work is available.
    /**
     * This function can be overridden. The default implementation is suitable for a
     * single-threaded model of execution.
     * Adding subscriptions, timers, services, etc. with blocking callbacks will cause this function
     * to block (which may have unintended consequences).
     * If the time that waitables take to be executed is longer than the period on which new waitables
     * become ready, this method will execute work repeatedly until `max_duration` has elapsed.
     *
     * \param[in] max_duration The maximum amount of time to spend executing work, must be >= 0.
     *   `0` is potentially block forever until no more work is available.
     * \throw std::invalid_argument if max_duration is less than 0.
     * Note that spin_all() may take longer than this time as it only returns once max_duration has
     * been exceeded.
     */
    // virtual void
    // spin_all(std::chrono::nanoseconds max_duration);

    /// Cancel any running spin* function, causing it to return.
    /**
     * This function can be called asynchonously from any thread.
     * \throws std::runtime_error if there is an issue triggering the guard condition
     */
    // void
    // cancel();

    /// Returns true if the executor is currently spinning.
    /**
     * This function can be called asynchronously from any thread.
     * \return True if the executor is currently spinning.
     */
    bool is_spinning();

    void activate_callback_groups();

protected:
    bool
    get_next_executable(
        AnyExecutable &any_executable,
        std::chrono::nanoseconds timeout = std::chrono::nanoseconds(1000));

    bool
    get_next_ready_executable(AnyExecutable &any_executable);

    /// Find the next available executable and do the work associated with it.
    /**
     * \param[in] any_exec Union structure that can hold any executable type (timer, subscription,
     * service, client).
     * \throws std::runtime_error if there is an issue triggering the guard condition
     */
    // void
    // execute_any_executable(AnyExecutable & any_exec);

    /// Find the next available executable and do the work associated with it.
    /**
     * \param[in] any_exec Union structure that can hold any executable type (timer, subscription,
     * service, client).
     * \throws std::runtime_error if there is an issue triggering the guard condition
     */
    void
    execute_any_executable(AnyExecutable &any_exec);

    static void
    execute_timer(smfcpp::TimerBase::SharedPtr timer);

    static void
    execute_recver(smfcpp::RecverBase::SharedPtr recver);

    mutable std::mutex m_mutex;
    /// Spinning state, used to prevent multi threaded calls to spin and to cancel blocking spins.
    std::atomic<bool> m_spinning;
    std::vector<smfcpp::CallbackGroup::WeakPtr> o_callback_groups;
};

class SingleThreadedExecutor : public smfcpp::Executor
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(SingleThreadedExecutor)
    explicit SingleThreadedExecutor(
        std::chrono::nanoseconds timeout = std::chrono::nanoseconds(NEXT_EXEC_TIMEOUT));
    virtual ~SingleThreadedExecutor() = default;

    /// Single-threaded implementation of spin.
    /**
     * This function will block until work comes in, execute it, and then repeat
     * the process until canceled.
     * It may be interrupt by a call to rclcpp::Executor::cancel() or by ctrl-c
     * if the associated context is configured to shutdown on SIGINT.
     * \throws std::runtime_error when spin() called while already spinning
     */
    virtual
    void 
    spin() override;

private:
    SMFCPP_DISABLE_COPY(SingleThreadedExecutor)
    std::chrono::nanoseconds m_next_exec_timeout;
};

class MultiThreadExecutor : public Executor
{
public:
    SMFCPP_SMART_PTR_DEFINITIONS(MultiThreadExecutor)

    explicit MultiThreadExecutor(
        size_t n_threads = 0,
        std::chrono::nanoseconds timeout = std::chrono::nanoseconds(NEXT_EXEC_TIMEOUT));

    virtual ~MultiThreadExecutor() = default;

    virtual
    void 
    spin() override;

protected:
    void
    run(size_t this_thread_number);

private:
    std::mutex m_wait_mutex;
    size_t m_n_threads;
    std::chrono::nanoseconds m_next_exec_timeout;
};

}

#endif