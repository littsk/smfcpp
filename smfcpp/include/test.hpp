#ifndef SMFCPP__INTERFACE_HPP_
#define SMFCPP__INTERFACE_HPP_

#include "node.hpp"
#include "macros.hpp"
#include "node_interface.hpp"
#include "timer.hpp"
#include "node_impl.hpp"
#include "context.hpp"
#include "executor.hpp"
#include "callback_group.hpp"

namespace smfcpp
{
namespace interface
{
    
class NodeBaseInterface
{
public:
    SMFCPP_SMART_PTR_ALIASES_ONLY(NodeBaseInterface)
    NodeBaseInterface() = default;

    virtual 
    ~NodeBaseInterface() = default;

    virtual
    smfcpp::Context::SharedPtr 
    get_context() = 0;

     /// Create and return a callback group.

    virtual
    smfcpp::CallbackGroup::SharedPtr
    create_callback_group(
        smfcpp::CallbackGroupType group_type) = 0;

    /// Return the default callback group.
    virtual
    smfcpp::CallbackGroup::SharedPtr
    get_default_callback_group() = 0;

    /// Return true if the given callback group is associated with this node.
    virtual
    bool
    callback_group_in_node(smfcpp::CallbackGroup::SharedPtr group) = 0;


    using CallbackGroupFunction = std::function<void (smfcpp::CallbackGroup::SharedPtr)>;

    virtual
    void
    for_each_callback_group(const CallbackGroupFunction & func) = 0;

private:
    SMFCPP_DISABLE_COPY(NodeBaseInterface)
};


class NodeBase: public NodeBaseInterface, public std::enable_shared_from_this<NodeBase>
{
public:
    SMFCPP_SMART_PTR_ALIASES_ONLY(NodeBase)
    
    /// Constructor.
    /*
     * If nullptr (default) is given for the default_callback_group, one will
     * be created by the constructor using the create_callback_group() method,
     * but virtual dispatch will not occur so overrides of that method will not
     * be used.
     */
    explicit NodeBase(
        const std::string & node_name,
        smfcpp::Context::SharedPtr context,
        smfcpp::CallbackGroup::SharedPtr default_callback_group = nullptr);

    virtual
    ~NodeBase();

    virtual 
    smfcpp::Context::SharedPtr 
    get_context() override;

    smfcpp::CallbackGroup::SharedPtr
    create_callback_group(
        smfcpp::CallbackGroupType group_type) override;

    virtual
    smfcpp::CallbackGroup::SharedPtr
    get_default_callback_group() override;

    virtual
    bool
    callback_group_in_node(smfcpp::CallbackGroup::SharedPtr group) override;

    virtual
    void
    for_each_callback_group(const CallbackGroupFunction & func) override;

private:
    smfcpp::Context::SharedPtr m_context;

    smfcpp::CallbackGroup::SharedPtr m_default_callback_group;
    std::mutex m_callback_groups_mutex;
    std::vector<smfcpp::CallbackGroup::WeakPtr> m_callback_groups;
};

class NodeTimerInterface
{
public:
    SMFCPP_SMART_PTR_ALIASES_ONLY(NodeTimerInterface)

    virtual
    ~NodeTimerInterface() = default;

    virtual
    void
    add_timer(
        smfcpp::TimerBase::SharedPtr timer,
        smfcpp::CallbackGroup::SharedPtr callback_group) = 0;
};

class NodeTimer: public NodeTimerInterface
{
public:
    SMFCPP_SMART_PTR_ALIASES_ONLY(NodeTimer)

    explicit NodeTimer();
    virtual
    ~NodeTimer();

    virtual
    void
    add_timer(
        smfcpp::TimerBase::SharedPtr timer,
        smfcpp::CallbackGroup::SharedPtr callback_group) override;

private:
    SMFCPP_DISABLE_COPY(NodeTimer)
    smfcpp::node_interface::NodeBaseInterface::SharedPtr node_base;
};


}
}

#endif