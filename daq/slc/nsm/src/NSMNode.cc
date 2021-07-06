/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
