#include "NSMNode.hh"

#include "Host.hh"

#include <util/StringUtil.hh>

using namespace B2DAQ;

const std::string NSMNode::getSQLFields() const throw()
{
  return Node::getSQLFields() + ", name text, used boolean, host_id int";
}

const std::string NSMNode::getSQLLabels() const throw()
{
  return Node::getSQLLabels() + ", name, used, host_id";
}

const std::string NSMNode::getSQLValues() const throw()
{
  int id = (getHost() != NULL) ? getHost()->getID() : -1;
  return Node::getSQLValues() +
         B2DAQ::form(", '%s', %s, %d", getName().c_str(),
                     ((isUsed()) ? "TRUE" : "FALSE"), id);
}
