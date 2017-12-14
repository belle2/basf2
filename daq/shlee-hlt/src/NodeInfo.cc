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

/// @brief NodeInfo constructor
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

/// @brief NodeInfo copy constructor
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

/// @brief NodeInfo destructor
NodeInfo::~NodeInfo()
{
}

/// @brief NodeInfo = operator overloading
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

/// @brief Clear the node information
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

/// @brief Set unit number of the node
/// @param unitNo Unit number of the node
void NodeInfo::unitNo(int unitNo)
{
  m_unitNo = unitNo;
}

/// @brief Get unit number of the node
/// @return Unit number of the node
int NodeInfo::unitNo()
{
  return m_unitNo;
}

/// @brief Set node number of the node
/// @param nodeNo Node number of the node
void NodeInfo::nodeNo(int nodeNo)
{
  m_nodeNo = nodeNo;
}

/// @brief Get node number of the node
/// @return Node number of the node
int NodeInfo::nodeNo()
{
  return m_nodeNo;
}

/// @brief Generate a unique key based on unit and node number of the node
/// @return Generated key (unit# * 100 + node#)
int NodeInfo::generateKey()
{
  return m_unitNo * 100 + m_nodeNo;
}

/// @brief Set experiment number
/// @param expNo Experiment number to be set
void NodeInfo::expNo(int expNo)
{
  m_expNo = expNo;
}

/// @brief Get experiment number
/// @return Experiment number of the current configuration
int NodeInfo::expNo()
{
  return m_expNo;
}

/// @brief Set run start number
/// @param runStart Run start number to be set
void NodeInfo::runStart(int runStart)
{
  m_runStart = runStart;
}

/// @brief Get run start number
/// @return Run start number of the current configuration
int NodeInfo::runStart()
{
  return m_runStart;
}

/// @brief Set run end number
/// @param runEnd Run end number to be set
void NodeInfo::runEnd(int runEnd)
{
  m_runEnd = runEnd;
}

/// @brief Get run end number
/// @return Run end number of the current configuration
int NodeInfo::runEnd()
{
  return m_runEnd;
}

/// @brief Set type of the node
/// @param type Type of the node
void NodeInfo::type(std::string type)
{
  m_type = type;
}

/// @brief Get type of the node
/// @return Type of the node
const std::string NodeInfo::type()
{
  return m_type;
}

/// @brief Set IP address of the manager node
/// @param managerIP IP address of the manager node
void NodeInfo::managerIP(std::string managerIP)
{
  m_managerIP = managerIP;
}

/// @brief Get IP address of the manager node
/// @return IP address of the manager node
const std::string NodeInfo::managerIP()
{
  return m_managerIP;
}

/// @brief Set IP address of the node
/// @param selfIP IP address of the node
void NodeInfo::selfIP(std::string selfIP)
{
  m_selfIP = selfIP;
}

/// @brief Get IP address of the node
/// @return IP address of the node
const std::string NodeInfo::selfIP()
{
  return m_selfIP;
}

/// @brief Add an IP address of a data source
/// @param sourceIP IP address of a data source
void NodeInfo::sourceIP(std::string sourceIP)
{
  m_sourceIP.push_back(sourceIP);
}

/// @brief Get the container that contains IP addresses of data sources
/// @return Standart vector container for data sources
const std::vector<std::string> NodeInfo::sourceIP()
{
  return m_sourceIP;
}

/// @brief Add an IP address of a target node
/// @param targetIP IP address of a target node
void NodeInfo::targetIP(std::string targetIP)
{
  m_targetIP.push_back(targetIP);
}

/// @brief Get the container that contains IP addresses of target node
/// @return Standart vector container for target node
const std::vector<std::string> NodeInfo::targetIP()
{
  return m_targetIP;
}

/// @brief Display the node information (development purpose only)
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
