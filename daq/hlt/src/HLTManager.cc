/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/HLTManager.h>

using namespace Belle2;

/* @brief HLTManager constructor
*/
HLTManager::HLTManager(void)
    : m_expNo(-1), m_runStart(-1), m_runEnd(-1)
{
}

/* @brief HLTManager constructor
*/
HLTManager::HLTManager(char* input)
    : m_inputXML(input)
{
  init();
}

/* @brief HLTManager constructor
 * @param input File name of input XML file
*/
HLTManager::HLTManager(std::string& input)
{
  m_inputXML = (char*)(input.c_str());
  init();
}

/* @brief HLTManager destructor
*/
HLTManager::~HLTManager(void)
{
}

/* @brief Initializing HLTManager
*/
void HLTManager::init(void)
{
  m_units.clear();

  m_XMLParser = new XMLParser(m_inputXML);
  m_XMLParser->parsing();

  initHLT();
  initUnit();
}

/* @brief Broadcasting node information to all nodes in HLT farm
*/
void HLTManager::broadCasting(void)
{
  for (int i = 0; i < (int)m_units.size(); i++) {
    broadCasting(i);
  }
}

/* @brief Broadcasting node information to all nodes in a specific node
 * For extension of functionality and doing nothing at this moment
*/
void HLTManager::broadCasting(int unitNo)
{
  m_units[unitNo].broadCasting();
}

/* @brief Initialize HLT farm information
*/
void HLTManager::initHLT(void)
{
  m_expNo = m_XMLParser->expNo();
  m_runStart = m_XMLParser->runStart();
  m_runEnd = m_XMLParser->runEnd();
  m_inputName = m_XMLParser->inputName();
  m_inputDescription = m_XMLParser->inputDescription();
}

/* @brief Initialize units
*/
void HLTManager::initUnit(void)
{
  std::vector<UnitInfo>::iterator i = m_XMLParser->unitInfo();
  for (int j = 0; j < m_XMLParser->NUnit(); j++) {
    UnitManager unitManager(*i);
    m_units.push_back(unitManager);
    i++;
  }
}

/* @brief Displaying HLT manager information (only for debugging)
*/
void HLTManager::Print(void)
{
  B2INFO("=================================================");
  B2INFO(" HLTManager Summary");
  B2INFO("   HLT Name: " << m_inputName);
  B2INFO("   HLT Description: " << m_inputDescription);
  B2INFO("   Exp = " << m_expNo << " (Run = " << m_runStart << " - " << m_runEnd << ")");
  B2INFO("   Units = " << m_units.size());
  for (std::vector<UnitManager>::iterator i = m_units.begin(); i != m_units.end(); i++)
    (*i).Print();
  B2INFO("=================================================");
}
