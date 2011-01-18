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

/* @brief NodeInfo constructor
*/
NodeInfo::NodeInfo()
{
  m_type = "NDEF";
  m_unitNo = 0;
  m_nodeNo = 0;
  m_portBaseDataIn = 0;
  m_portBaseDataOut = 0;
  m_portBaseControl = 0;
  m_thisIP = "NDEF";
  m_managerIP = "NDEF";
}

/* @brief NodeInfo constructor
 * @param type Type of the node
 * @param unitNo Unit number of the node
 * @param nodeNo Node number of the node
*/
NodeInfo::NodeInfo(const std::string type, const int unitNo, const int nodeNo)
{
  m_type = type;
  m_unitNo = unitNo;
  m_nodeNo = nodeNo;
  m_portBaseDataIn = 0;
  m_portBaseDataOut = 0;
  m_portBaseControl = 0;
  m_thisIP = "NDEF";
  m_managerIP = "NDEF";
}

/* @brief NodeInfo destructor
*/
NodeInfo::~NodeInfo()
{
}

/* @brief Initializing NodeInfo
 * Do nothing at this moment and maybe unnecessary
*/
void NodeInfo::init()
{
}

/* @brief Get unit number of the node
 * @return Unit number of the node
*/
int NodeInfo::unitNo()
{
  return m_unitNo;
}

/* @brief Get node number of the node
 * @return Node number of the node
*/
int NodeInfo::nodeNo()
{
  return m_nodeNo;
}

/* @brief Get type of the node
 * @return Type of the node
*/
std::string NodeInfo::type()
{
  return m_type;
}

/* @brief Set ports for data communication
 * @param inPort incoming port number
 * @param outPort outgoing port number
*/
void NodeInfo::setPortData(int inPort, int outPort)
{
  m_portBaseDataIn = inPort;
  m_portBaseDataOut = outPort;
}

/* @brief Set port for control
 * @param port Control port number
*/
void NodeInfo::setPortControl(int port)
{
  m_portBaseControl = port;
}

/* @brief Get port number for incoming data
 * @return Port number for incoming data
*/
int NodeInfo::portBaseDataIn()
{
  return m_portBaseDataIn;
}

/* @brief Get port number for outgoing data
 * @return Port number for outgoing data
*/
int NodeInfo::portBaseDataOut()
{
  return m_portBaseDataOut;
}

/* @brief Get port number for control
 * @return Port number for control
*/
int NodeInfo::portBaseControl()
{
  return m_portBaseControl;
}

/* @brief Set IP address of this node
 * @param ip IP address of this node
*/
void NodeInfo::setThisIP(const std::string ip)
{
  m_thisIP = ip;
}

/* @brief Set IP address of manager node
 * @param ip IP address of manager node
*/
void NodeInfo::setManagerIP(const std::string ip)
{
  m_managerIP = ip;
}

/* @brief Set IP addresses of sources of data
 * @param ip Vector container which contains IP addresses of sources
*/
void NodeInfo::setSourceIP(std::vector<std::string> ip)
{
  m_sourceIP.clear();
  m_sourceIP = ip;
}

/* @brief Set a IP address of a single source
 * @param ip IP address of a source
*/
void NodeInfo::setSourceIP(std::string ip)
{
  m_sourceIP.clear();
  m_sourceIP.push_back(ip);
}

/* @brief Set IP addresses of targets
 * @param ip Vector container which contains IP addresses of targets
*/
void NodeInfo::setTargetIP(std::vector<std::string> ip)
{
  m_targetIP.clear();
  m_targetIP = ip;
}

/* @brief Set a IP address of a single target
 * @param ip IP address of a target
*/
void NodeInfo::setTargetIP(std::string ip)
{
  m_targetIP.clear();
  m_targetIP.push_back(ip);
}

/* @brief Get IP address of this node
 * @return IP address of this node
*/
std::string NodeInfo::thisIP()
{
  return m_thisIP;
}

/* @brief Get IP address of manager node
 * @return IP address of manager node
*/
std::string NodeInfo::managerIP()
{
  return m_managerIP;
}

/* @brief Get IP addresses of sources
 * @return Vector container which contains IP addresses of sources
*/
std::vector<std::string> NodeInfo::sourceIP()
{
  return m_sourceIP;
}

/* @brief Get IP addresses of targets
 * @return Vector container which contains IP addresses of targets
*/
std::vector<std::string> NodeInfo::targetIP()
{
  return m_targetIP;
}

void NodeInfo::setSteering(const char* steering)
{
  std::ifstream input;
  input.open(steering, std::ios::binary);

  input.seekg(0, std::ios::end);
  int size = input.tellg();
  input.seekg(0, std::ios::beg);

  m_steeringContents = new char[size];

  input.read(m_steeringContents, size);
  input.close();
}

void NodeInfo::getSteering()
{
  std::ofstream output;
  output.open(m_steeringName, std::ios::binary);

  output.write(m_steeringContents, strlen(m_steeringContents));
}

/* @brief Serializing the NodeInfo object
 * This uses very simple serialization method called stringstream-based serialization.
 * This approach is actually not good at all.
 * @todo Changing object serialization part
 * @return Serialized object as a string
*/
std::string NodeInfo::serializedNodeInfo()
{
  std::stringstream ss;
  ss << m_unitNo << " " << m_nodeNo;
  ss << " " << m_type;
  ss << " " << m_portBaseDataIn << " " << m_portBaseDataOut << " " << m_portBaseControl;
  ss << " " << m_thisIP << " " << m_managerIP << " " << m_steeringName;
  ss << " " << m_steeringContents;

  for (unsigned int i = 0; i < m_sourceIP.size(); i++) {
    ss << m_sourceIP[i];
    if (i == m_sourceIP.size() - 1)
      ss << " ";
    else
      ss << "_";
  }

  for (unsigned int i = 0; i < m_targetIP.size(); i++) {
    ss << m_targetIP[i];
    if (i != m_targetIP.size() - 1)
      ss << "_";
  }

  return ss.str();
}

/* @brief Deserializing the NodeInfo object
 * @param nodeinfo Serialized NodeInfo object
*/
void NodeInfo::deserializedNodeInfo(const std::string nodeinfo)
{
  std::string tmpSourceIP;
  std::string tmpTargetIP;

  // Since the targetIP is opposite between NodeManagers in Manager node and Nodes,
  // tmpTargetIP and tmpSourceIP is switched
  std::stringstream ss(nodeinfo);
  ss >> m_unitNo >> m_nodeNo >> m_type >> m_portBaseDataIn >> m_portBaseDataOut
  >> m_portBaseControl >> m_thisIP >> m_managerIP
  >> tmpTargetIP >> tmpSourceIP >> m_steeringName >> m_steeringContents;

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

/* @brief Displaying NodeInfo object (only for debugging)
*/
void NodeInfo::Print()
{
  B2INFO("=================================================");
  B2INFO(" NodeInfo Summary");
  B2INFO("   m_type = " << m_type);
  B2INFO("   m_unitNo = " << m_unitNo);
  B2INFO("   m_nodeNo = " << m_nodeNo);
  B2INFO("   m_portBaseDataIn = " << m_portBaseDataIn);
  B2INFO("   m_portBaseDataOut = " << m_portBaseDataOut);
  B2INFO("   m_portBaseControl = " << m_portBaseControl);
  B2INFO("   m_thisIP = " << m_thisIP);
  B2INFO("   m_managerIP = " << m_managerIP);
  B2INFO("   m_steeringName = " << m_steeringName);
  B2INFO("   m_steeringContents = " << m_steeringContents);
  B2INFO("   m_sourceIP (" << m_sourceIP.size() << ")");
  for (unsigned int i = 0; i < m_sourceIP.size(); i++)
    B2INFO("     " << m_sourceIP[i]);
  B2INFO("   m_targetIP (" << m_targetIP.size() << ")");
  for (unsigned int i = 0; i < m_targetIP.size(); i++)
    B2INFO("     " << m_targetIP[i]);
  B2INFO("=================================================");
}
