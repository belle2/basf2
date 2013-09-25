#include "Node.hh"

#include <util/StringUtil.hh>

using namespace B2DAQ;

const std::string Node::getSQLFields() const throw()
{
  return "version int, id int";
}

const std::string Node::getSQLLabels() const throw()
{
  return "version, id";
}

const std::string Node::getSQLValues() const throw()
{
  return B2DAQ::form("%d, %d", getVersion(), getID());
}
