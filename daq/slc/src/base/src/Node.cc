#include "base/Node.h"

#include "base/StringUtil.h"

using namespace Belle2;

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
  return Belle2::form("%d, %d", getVersion(), getID());
}
