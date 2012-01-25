/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010-2011  Belle II Collaboration                         *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/NodeInfo.h>

using namespace Belle2;

NodeInfo::NodeInfo()
{
  m_unitNo = -1;
  m_nodeNo = -1;
  m_expNo = -1;
  m_runStart = -1;
  m_runEnd = -1;

  m_type.clear();
  m_managerIP.clear();
  m_selfIP.clear();

  m_sourceIP.clear();
  m_targetIP.clear();
}

NodeInfo::NodeInfo(const NodeInfo& nodeinfo)
{
  m_unitNo = nodeinfo.m_unitNo;
  m_nodeNo = nodeinfo.m_nodeNo;

  m_expNo = nodeinfo.m_expNo;
  m_runStart = nodeinfo.m_runStart;
  m_runEnd = nodeinfo.m_runEnd;

  m_type = nodeinfo.m_type;

  m_managerIP = nodeinfo.m_managerIP;
  m_selfIP = nodeinfo.m_selfIP;

  m_sourceIP = nodeinfo.m_sourceIP;
  m_targetIP = nodeinfo.m_targetIP;
}

NodeInfo::~NodeInfo()
{
}

NodeInfo& NodeInfo::operator=(const NodeInfo& nodeinfo)
{
  if (this != &nodeinfo) {
    m_unitNo = nodeinfo.m_unitNo;
    m_nodeNo = nodeinfo.m_nodeNo;

    m_expNo = nodeinfo.m_expNo;
    m_runStart = nodeinfo.m_runStart;
    m_runEnd = nodeinfo.m_runEnd;

    m_type = nodeinfo.m_type;

    m_managerIP = nodeinfo.m_managerIP;
    m_selfIP = nodeinfo.m_selfIP;

    m_sourceIP = nodeinfo.m_sourceIP;
    m_targetIP = nodeinfo.m_targetIP;

    return *this;
  }
}

void NodeInfo::clear()
{
  m_unitNo = -1;
  m_nodeNo = -1;
  m_expNo = -1;
  m_runStart = -1;
  m_runEnd = -1;

  m_type.clear();
  m_managerIP.clear();
  m_selfIP.clear();

  m_sourceIP.clear();
  m_targetIP.clear();
}

void NodeInfo::unitNo(int unitNo)
{
  m_unitNo = unitNo;
}

const int NodeInfo::unitNo()
{
  return m_unitNo;
}

void NodeInfo::nodeNo(int nodeNo)
{
  m_nodeNo = nodeNo;
}

const int NodeInfo::nodeNo()
{
  return m_nodeNo;
}

const int NodeInfo::generateKey()
{
  return m_unitNo * 100 + m_nodeNo;
}

void NodeInfo::expNo(int expNo)
{
  m_expNo = expNo;
}

const int NodeInfo::expNo()
{
  return m_expNo;
}

void NodeInfo::runStart(int runStart)
{
  m_runStart = runStart;
}

const int NodeInfo::runStart()
{
  return m_runStart;
}

void NodeInfo::runEnd(int runEnd)
{
  m_runEnd = runEnd;
}

const int NodeInfo::runEnd()
{
  return m_runEnd;
}

void NodeInfo::type(std::string type)
{
  m_type = type;
}

const std::string NodeInfo::type()
{
  return m_type;
}

void NodeInfo::managerIP(std::string managerIP)
{
  m_managerIP = managerIP;
}

const std::string NodeInfo::managerIP()
{
  return m_managerIP;
}

void NodeInfo::selfIP(std::string selfIP)
{
  m_selfIP = selfIP;
}

const std::string NodeInfo::selfIP()
{
  return m_selfIP;
}

void NodeInfo::sourceIP(std::string sourceIP)
{
  m_sourceIP.push_back(sourceIP);
}

const std::vector<std::string> NodeInfo::sourceIP()
{
  return m_sourceIP;
}

void NodeInfo::targetIP(std::string targetIP)
{
  m_targetIP.push_back(targetIP);
}

const std::vector<std::string> NodeInfo::targetIP()
{
  return m_targetIP;
}

void NodeInfo::display()
{
  B2INFO("unit#=" << m_unitNo << "    nodeNo =" << m_nodeNo);
  B2INFO("type =" << m_type << "    manager=" << m_managerIP);
  B2INFO("sources:");
  for (std::vector<std::string>::const_iterator i = m_sourceIP.begin();
       i != m_sourceIP.end(); ++i)
    B2INFO("          " << (*i));
  B2INFO("targets:");
  for (std::vector<std::string>::const_iterator i = m_targetIP.begin();
       i != m_targetIP.end(); ++i)
    B2INFO("          " << (*i));
}
