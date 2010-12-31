/// @file HLTManager.cc
/// @brief HLTManager class implementation
/// @author Soohyung Lee
/// @date Jul 14 2010

#include <daq/hlt/HLTManager.h>

using namespace Belle2;

/// @brief HLTManager class constructor
HLTManager::HLTManager(void)
    : m_expNo(-1), m_runStart(-1), m_runEnd(-1)
{
}

/// @brief HLTManager class constructor
/// @param input Input XML file name
HLTManager::HLTManager(char* input)
    : m_inputXML(input)
{
  init();
}

HLTManager::HLTManager(std::string& input)
{
  m_inputXML = (char*)(input.c_str());
  init();
}

/// @brief HLTManager class destructor
HLTManager::~HLTManager(void)
{
}

/// @brief Initialize HLTManager class
void HLTManager::init(void)
{
  m_units.clear();

  m_XMLParser = new XMLParser(m_inputXML);
  m_XMLParser->parsing();

  initHLT();
  initUnit();
}

void HLTManager::broadCasting(void)
{
  m_units[0].broadCasting();
}

void HLTManager::broadCasting(int unitNo)
{

}

/// @brief Initialize HLT farm information
void HLTManager::initHLT(void)
{
  m_expNo = m_XMLParser->expNo();
  m_runStart = m_XMLParser->runStart();
  m_runEnd = m_XMLParser->runEnd();
  m_inputName = m_XMLParser->inputName();
  m_inputDescription = m_XMLParser->inputDescription();
}

/// @brief Initialize units
void HLTManager::initUnit(void)
{
  std::vector<UnitInfo>::iterator i = m_XMLParser->unitInfo();
  for (int j = 0; j < m_XMLParser->NUnit(); j++) {
    UnitManager unitManager(*i);
    m_units.push_back(unitManager);
    i++;
  }
}

/// @brief Printing internal information
void HLTManager::Print(void)
{
  B2INFO("---------------- \033[22;31mHLT Manager Summary\033[0m ----------------");
  B2INFO("  HLT Name        : " << m_inputName);
  B2INFO("  HLT Description : " << m_inputDescription);
  B2INFO("  Exp = " << m_expNo << " (Run = " << m_runStart << "-" << m_runEnd << ")");
  B2INFO("  Units = " << m_units.size());
  for (std::vector<UnitManager>::iterator i = m_units.begin(); i != m_units.end(); i++)
    (*i).Print();
  B2INFO("-----------------------------------------------------");
}
