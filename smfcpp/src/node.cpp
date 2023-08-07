#include "node.hpp"
#include "node_interface.hpp"

namespace smfcpp
{


Node::Node(const std::string & node_name)
: m_name(node_name),
  m_node_base(new smfcpp::node_interface::NodeBase(node_name, smfcpp::Context::make_shared())),
  m_node_timers(new smfcpp::node_interface::NodeTimer(m_node_base)),
  m_node_recvers(new smfcpp::node_interface::NodeRecver(m_node_base))
{}

const char * Node::get_name() const
{
    return this->m_name.c_str();
}

smfcpp::node_interface::NodeBaseInterface::SharedPtr
Node::get_node_base_interfase()
{
    return m_node_base;
}

}



