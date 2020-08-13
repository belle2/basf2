#include "daq/slc/nsm/NSMNode.h"

#include "daq/slc/base/StringUtil.h"

using namespace Belle2;

NSMNode::NSMNode(const std::string& name)
{
  m_name = StringUtil::toupper(name);
  m_id = -1;
}

void NSMNode::setName(const std::string& name)
{
  m_name = StringUtil::toupper(name);
}
