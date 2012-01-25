#include <daq/hlt/XMLParser.h>

using namespace Belle2;

XMLParser::XMLParser(std::string filename)
{
  m_filename = filename;

  m_expNo = -1;
  m_runStart = -1;
  m_runEnd = -1;

  m_dataSources.clear();
  m_dataTargets.clear();

  m_eventSeparators.clear();
  m_workerNodes.clear();
  m_eventMergers.clear();

  m_curNode = NULL;
}

XMLParser::~XMLParser()
{
}

EHLTStatus XMLParser::init()
{
  m_docPtr = xmlParseFile(m_filename.c_str());
  m_curNode = xmlDocGetRootElement(m_docPtr);

  if (xmlStrcmp(m_curNode->name, (const xmlChar*)"HLT")) {
    B2ERROR("Wrong xml format!");
    return c_InitFailed;
  }

  return c_Success;
}

EHLTStatus XMLParser::parsing()
{
  xmlNodePtr cur = m_curNode->xmlChildrenNode;
  xmlChar* key;

  while (cur != NULL) {
    key = xmlNodeListGetString(m_docPtr, cur->xmlChildrenNode, 1);

    if (!xmlStrcmp(cur->name, (const xmlChar*)"ExpNo")) {
      m_expNo = atoi((char*)key);
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"RunStart")) {
      m_runStart = atoi((char*)key);
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"RunEnd")) {
      m_runEnd = atoi((char*)key);
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"Manager")) {
      m_managerIP = (char*)key;
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"DataSource")) {
      m_dataSources.push_back((char*)key);
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"DataTarget")) {
      m_dataTargets.push_back((char*)key);
    } else if (!xmlStrcmp(cur->name, (const xmlChar*)"Unit")) {
      int unitNo = atoi((char*)xmlGetProp(cur, (const xmlChar*)"no"));
      xmlNodePtr curUnit = cur->xmlChildrenNode;

      while (curUnit != NULL) {
        xmlChar* keyUnit = xmlNodeListGetString(m_docPtr, curUnit->xmlChildrenNode, 1);

        if (!xmlStrcmp(curUnit->name, (const xmlChar*)"ES")) {
          int keyES = unitNo * 100;
          if (m_eventSeparators.insert(std::pair<int, std::string>(keyES, (char*)keyUnit)).second == false) {
            B2ERROR("Failed to parse IP of event separator!");
            return c_FuncError;
          }
        } else if (!xmlStrcmp(curUnit->name, (const xmlChar*)"WN")) {
          int nodeNo = atoi((char*)xmlGetProp(curUnit, (const xmlChar*)"no"));
          int keyWN = unitNo * 100 + nodeNo;
          if (m_workerNodes.insert(std::pair <int, std::string>(keyWN, (char*)keyUnit)).second == false) {
            B2ERROR("Failed to parse IP of worker node!");
            return c_FuncError;
          }
        } else if (!xmlStrcmp(curUnit->name, (const xmlChar*)"EM")) {
          int keyEM = unitNo * 100 + 99;
          if (m_eventMergers.insert(std::pair<int, std::string>(keyEM, (char*)keyUnit)).second == false) {
            B2ERROR("Failed to parse IP of event merger!");
            return c_FuncError;
          }
        }

        curUnit = curUnit->next;
      }
    }

    cur = cur->next;
  }

  return c_Success;
}

unsigned int XMLParser::getAllKeys(std::vector<int>& container)
{
  container.clear();
  unsigned int nKeys = 0;

  for (std::map<int, std::string>::const_iterator i = m_eventSeparators.begin();
       i != m_eventSeparators.end(); ++i) {
    container.push_back((*i).first);
    nKeys++;
  }
  for (std::map<int, std::string>::const_iterator i = m_workerNodes.begin();
       i != m_workerNodes.end(); ++i) {
    container.push_back((*i).first);
    nKeys++;
  }
  for (std::map<int, std::string>::const_iterator i = m_eventMergers.begin();
       i != m_eventMergers.end(); ++i) {
    container.push_back((*i).first);
    nKeys++;
  }

  return nKeys;
}

void XMLParser::fill(int key, NodeInfo& nodeinfo)
{
  nodeinfo.expNo(m_expNo);
  nodeinfo.runStart(m_runStart);
  nodeinfo.runEnd(m_runEnd);
  nodeinfo.managerIP(m_managerIP);

  nodeinfo.unitNo(key / 100);
  nodeinfo.nodeNo(key % 100);

  if ((key % 100) == 0) {
    nodeinfo.type("ES");
    nodeinfo.selfIP(m_eventSeparators[key]);
    for (std::vector<std::string>::const_iterator i = m_dataSources.begin();
         i != m_dataSources.end(); ++i)
      nodeinfo.sourceIP(*i);
    for (std::map<int, std::string>::const_iterator i = m_workerNodes.begin();
         i != m_workerNodes.end(); ++i) {
      if (((*i).first / 100) == (key / 100))
        nodeinfo.targetIP((*i).second);
    }
  } else if ((key % 100) == 99) {
    nodeinfo.type("EM");
    nodeinfo.selfIP(m_eventMergers[key]);
    for (std::map<int, std::string>::const_iterator i = m_workerNodes.begin();
         i != m_workerNodes.end(); ++i) {
      if (((*i).first / 100) == (key / 100))
        nodeinfo.sourceIP((*i).second);
    }
    for (std::vector<std::string>::const_iterator i = m_dataTargets.begin();
         i != m_dataTargets.end(); ++i)
      nodeinfo.targetIP(*i);
  } else {
    nodeinfo.type("WN");
    nodeinfo.selfIP(m_workerNodes[key]);
    for (std::map<int, std::string>::const_iterator i = m_eventSeparators.begin();
         i != m_eventSeparators.end(); ++i) {
      if (((*i).first / 100) == (key / 100))
        nodeinfo.sourceIP((*i).second);
    }
    for (std::map<int, std::string>::const_iterator i = m_eventMergers.begin();
         i != m_eventMergers.end(); ++i) {
      if (((*i).first / 100) == (key / 100))
        nodeinfo.targetIP((*i).second);
    }
  }
}

void XMLParser::display()
{
  B2INFO("filename: " << m_filename);
  B2INFO("   exp#=" << m_expNo << "    run#=" << m_runStart << "-" << m_runEnd);
  B2INFO("   manager=" << m_managerIP);
  B2INFO("   data sources:");
  for (std::vector<std::string>::const_iterator i = m_dataSources.begin();
       i != m_dataSources.end(); ++i)
    B2INFO("       " << (*i));
  B2INFO("   data targets:");
  for (std::vector<std::string>::const_iterator i = m_dataTargets.begin();
       i != m_dataTargets.end(); ++i)
    B2INFO("       " << (*i));
  B2INFO("   event separators:");
  for (std::map<int, std::string>::const_iterator i = m_eventSeparators.begin();
       i != m_eventSeparators.end(); ++i)
    B2INFO("       " << (*i).second << " (" << (*i).first << ")");
  B2INFO("   worker nodes:");
  for (std::map<int, std::string>::const_iterator i = m_workerNodes.begin();
       i != m_workerNodes.end(); ++i)
    B2INFO("       " << (*i).second << " (" << (*i).first << ")");
  B2INFO("   event mergers:");
  for (std::map<int, std::string>::const_iterator i = m_eventMergers.begin();
       i != m_eventMergers.end(); ++i)
    B2INFO("       " << (*i).second << " (" << (*i).first << ")");
}
