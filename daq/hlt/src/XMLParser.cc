/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <daq/hlt/XMLParser.h>
#include <framework/logging/Logger.h>

using namespace Belle2;

/// @brief XMLParser constructor
///        Initializes variables
XMLParser::XMLParser(void)
    : m_filename(NULL), m_expNo(-1), m_runStart(-1), m_runEnd(-1)
{
  m_units.clear();
}

/// @brief XMLParser constructor
///        Initializes variables and does XML parsing from input file
/// @param filename File name to parse
XMLParser::XMLParser(char* filename)
    : m_filename(filename), m_expNo(-1), m_runStart(-1), m_runEnd(-1)
{
  m_units.clear();

  init();
}

/// @brief XMLParser destructor
XMLParser::~XMLParser(void)
{
  xmlCleanupParser();
}

/// @brief Initialize the parsing
/// @return 0 for success
/// @return -1 for error (Wrong format)
int XMLParser::init(void)
{
  m_docPtr = xmlParseFile(m_filename);
  m_curNode = xmlDocGetRootElement(m_docPtr);

  if (xmlStrcmp(m_curNode->name, (const xmlChar*)"HLT")) {
    B2ERROR("Wrong format input");
    return -1;
  }

  return 0;
}

/// @brief Do parsing (Unnecessary function?)
/// @return 0 for success
/// @return -1 for error
/// @sa parsing
int XMLParser::parsing(void)
{
  return parsing(m_curNode);
}

/// @brief Actual parsing part
/// @param cur Pointer to a node in XML tree
/// @return 0 for success
/// @return -1 for error
int XMLParser::parsing(xmlNodePtr cur)
{
  //std::cout << "[\033[22;35mManager\033[0m] Starting to parse XML file" << std::endl;

  xmlChar* key;
  cur = cur->xmlChildrenNode;

  while (cur != NULL) {
    key = xmlNodeListGetString(m_docPtr, cur->xmlChildrenNode, 1);

    if (!xmlStrcmp(cur->name, (const xmlChar*)"ExpNo")) {
      if (m_expNo < 0)
        m_expNo = atoi((char*)key);
      else {
        B2ERROR("Parsing Error: Redundant exp no.");
        return -1;
      }
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"RunStart")) {
      if (m_runStart < 0)
        m_runStart = atoi((char*)key);
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"RunEnd")) {
      if (m_runEnd < 0 && m_runStart <= atoi((char*)key))
        m_runEnd = atoi((char*)key);
      else if (m_runStart > atoi((char*)key)) {
        B2ERROR("Parsing Error: Wrong run no. range");
        return -1;
      }
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"Name")) {
      m_inputName = (char*)key;
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"Description")) {
      m_inputDescription = (char*)key;
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"Manager")) {
      m_manager = (char*)key;
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"Unit")) {
      UnitInfo unit(atoi((char*)xmlGetProp(cur, (const xmlChar*)"no")));

      xmlNodePtr cur2 = cur->xmlChildrenNode;
      unitParsing(cur2, unit);

      m_units.push_back(unit);
    }

    cur = cur->next;
  }

  return 0;
}

/// @brief Parsing a single unit
/// @param cur Pointer to a node
/// @param unit Container to take unit information
void XMLParser::unitParsing(xmlNodePtr cur, UnitInfo& unit)
{
  xmlChar* key;

  while (cur != NULL) {
    key = xmlNodeListGetString(m_docPtr, cur->xmlChildrenNode, 1);

    if (!xmlStrcmp(cur->name, (const xmlChar*)"ES"))
      unit.eventSeparator((char*)key);
    else if (!xmlStrcmp(cur->name, (const xmlChar*)"EM"))
      unit.eventMerger((char*)key);
    else if (!xmlStrcmp(cur->name, (const xmlChar*)"WN"))
      unit.workerNodes((char*)key);

    unit.manager(m_manager);

    cur = cur->next;
  }
}

/// @brief Return input name
/// @return Input name which is specified in XML input file
char* XMLParser::inputName(void)
{
  return m_inputName;
}

/// @brief Return input description
/// @return Input description which is specified in XML input file
char* XMLParser::inputDescription(void)
{
  return m_inputDescription;
}

/// @brief Return experiment number
/// @return Experiment number which is specified in XML input file
int XMLParser::expNo(void)
{
  return m_expNo;
}

/// @brief Return start run number
/// @return Run start number which is specified in XML input file
int XMLParser::runStart(void)
{
  return m_runStart;
}

/// @brief Return end run number
/// @return Run end number which is specified in XML input file
int XMLParser::runEnd(void)
{
  return m_runEnd;
}

/// @brief Return the number of units
/// @return The number of units assigned
int XMLParser::NUnit(void)
{
  return m_units.size();
}

/// @brief Return information of units
/// @return Iterator (starting point) of the container which has unit information
std::vector<UnitInfo>::iterator XMLParser::unitInfo(void)
{
  return m_units.begin();
}

/// @brief Print the entire information
void XMLParser::Print(void)
{
  std::cout << "Name = " << m_inputName << std::endl;
  std::cout << "Description = " << m_inputDescription << std::endl;
  std::cout << "ExpNo = " << m_expNo << std::endl;
  std::cout << "RunNo = " << m_runStart << "-" << m_runEnd << std::endl;

  std::cout << "# of Units = " << m_units.size() << std::endl;
  for (std::vector<UnitInfo>::iterator i = m_units.begin(); i != m_units.end(); i++)
    (*i).Print();
}
