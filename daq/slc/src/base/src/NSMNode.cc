#include "NSMNode.h"

#include "Host.h"

#include "StringUtil.h"

using namespace Belle2;

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
         Belle2::form(", '%s', %s, %d", getName().c_str(),
                      ((isUsed()) ? "TRUE" : "FALSE"), id);
}
