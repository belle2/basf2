/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <iostream>
#include <sstream>
#include <string>

#include <daq/hlt/NodeInfo.h>

using namespace Belle2;

NodeInfo::NodeInfo()
    : m_type("NDEF"), m_unitNo(0), m_nodeNo(0),
    m_portBaseDataIn(0), m_portBaseDataOut(0), m_portBaseControl(0),
    m_thisIP("NDEF"), m_managerIP("NDEF")
{
}

NodeInfo::NodeInfo(const std::string type, const int unitNo, const int nodeNo)
    : m_type(type), m_unitNo(unitNo), m_nodeNo(nodeNo),
    m_portBaseDataIn(0), m_portBaseDataOut(0), m_portBaseControl(0),
    m_thisIP("NDEF"), m_managerIP("NDEF")
{
}

NodeInfo::~NodeInfo()
{
}

void NodeInfo::init()
{
}

int NodeInfo::unitNo()
{
  return m_unitNo;
}

int NodeInfo::nodeNo()
{
  return m_nodeNo;
}

std::string NodeInfo::type()
{
  return m_type;
}

void NodeInfo::setPortData(int inPort, int outPort)
{
  m_portBaseDataIn = inPort;
  m_portBaseDataOut = outPort;
}

void NodeInfo::setPortControl(int port)
{
  m_portBaseControl = port;
}

int NodeInfo::portBaseDataIn()
{
  return m_portBaseDataIn;
}

int NodeInfo::portBaseDataOut()
{
  return m_portBaseDataOut;
}

int NodeInfo::portBaseControl()
{
  return m_portBaseControl;
}

void NodeInfo::setThisIP(const std::string ip)
{
  m_thisIP = ip;
}

void NodeInfo::setManagerIP(const std::string ip)
{
  m_managerIP = ip;
}

void NodeInfo::setSourceIP(std::vector<std::string> ip)
{
  m_sourceIP.clear();
  m_sourceIP = ip;
}

void NodeInfo::setSourceIP(std::string ip)
{
  m_sourceIP.clear();
  m_sourceIP.push_back(ip);
}

void NodeInfo::setTargetIP(std::vector<std::string> ip)
{
  m_targetIP.clear();
  m_targetIP = ip;
}

void NodeInfo::setTargetIP(std::string ip)
{
  m_targetIP.clear();
  m_targetIP.push_back(ip);
}

std::string NodeInfo::thisIP()
{
  return m_thisIP;
}

std::string NodeInfo::managerIP()
{
  return m_managerIP;
}

std::vector<std::string> NodeInfo::sourceIP()
{
  return m_sourceIP;
}

std::vector<std::string> NodeInfo::targetIP()
{
  return m_targetIP;
}

std::string NodeInfo::serializedNodeInfo()
{
  std::stringstream ss;
  ss << m_unitNo << " " << m_nodeNo;
  ss << " " << m_type;
  ss << " " << m_portBaseDataIn << " " << m_portBaseDataOut << " " << m_portBaseControl;
  ss << " " << m_thisIP << " " << m_managerIP << " ";

  for (int i = 0; i < m_sourceIP.size(); i++) {
    ss << m_sourceIP[i];
    if (i == m_sourceIP.size() - 1)
      ss << " ";
    else
      ss << "_";
  }

  for (int i = 0; i < m_targetIP.size(); i++) {
    ss << m_targetIP[i];
    if (i != m_targetIP.size() - 1)
      ss << "_";
  }

  return ss.str();
}

void NodeInfo::deserializedNodeInfo(const std::string nodeinfo)
{
  std::string tmpSourceIP;
  std::string tmpTargetIP;

  // Since the targetIP is opposite between NodeManagers in Manager node and Nodes,
  // tmpTargetIP and tmpSourceIP is switched
  std::stringstream ss(nodeinfo);
  ss >> m_unitNo >> m_nodeNo >> m_type >> m_portBaseDataIn >> m_portBaseDataOut
  >> m_portBaseControl >> m_thisIP >> m_managerIP
  >> tmpTargetIP >> tmpSourceIP;

  m_sourceIP.clear();
  m_targetIP.clear();

  std::string::size_type index = tmpSourceIP.find_first_not_of("_", 0);
  std::string::size_type first = tmpSourceIP.find_first_of("_", index);

  while (std::string::npos != first || std::string::npos != index) {
    m_sourceIP.push_back(tmpSourceIP.substr(index, first - index));
    index = tmpSourceIP.find_first_not_of(" ", first);
    first = tmpSourceIP.find_first_of(" ", index);
  }

  index = tmpTargetIP.find_first_not_of("_", 0);
  first = tmpTargetIP.find_first_of("_", index);

  while (std::string::npos != first || std::string::npos != index) {
    m_targetIP.push_back(tmpTargetIP.substr(index, first - index));
    index = tmpTargetIP.find_first_not_of(" ", first);
    first = tmpTargetIP.find_first_of(" ", index);
  }
}

void NodeInfo::Print()
{
  std::cout << "m_type = " << m_type << std::endl;
  std::cout << "m_unitNo = " << m_unitNo << std::endl;
  std::cout << "m_nodeNo = " << m_nodeNo << std::endl;
  std::cout << "m_portBaseDataIn = " << m_portBaseDataIn << std::endl;
  std::cout << "m_portBaseDataOut = " << m_portBaseDataOut << std::endl;
  std::cout << "m_portBaseControl = " << m_portBaseControl << std::endl;
  std::cout << "m_thisIP = " << m_thisIP << std::endl;
  std::cout << "m_managerIP = " << m_managerIP << std::endl;
  std::cout << "m_sourceIP (" << m_sourceIP.size() << ") = ";
  for (int i = 0; i < m_sourceIP.size(); i++)
    std::cout << m_sourceIP[i] << " ";
  std::cout << std::endl << "m_targetIP (" << m_targetIP.size() << ") = ";
  for (int i = 0; i < m_targetIP.size(); i++)
    std::cout << m_targetIP[i] << " ";
  std::cout << std::endl;
  //std::cout << "m_sourceIP = " << m_sourceIP << std::endl;
  //std::cout << "m_targetIP = " << m_targetIP << std::endl;
}
