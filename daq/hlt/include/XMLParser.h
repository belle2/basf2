#ifndef XMLPARSER_H
#define XMLPARSER_H

#include <string>
#include <vector>
#include <map>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

#include <daq/hlt/NodeInfo.h>

namespace Belle2 {
  class XMLParser {
  public:
    XMLParser(std::string filename);
    ~XMLParser();

    EHLTStatus init();
    EHLTStatus parsing();

    unsigned int getAllKeys(std::vector<int>& container);
    void fill(int key, NodeInfo& nodeinfo);

    void display();

  private:
    std::string m_filename;

    int m_expNo, m_runStart, m_runEnd;

    std::string m_managerIP;

    std::vector<std::string> m_dataSources;
    std::vector<std::string> m_dataTargets;
    std::map<int, std::string> m_eventSeparators;
    std::map<int, std::string> m_workerNodes;
    std::map<int, std::string> m_eventMergers;

    xmlDocPtr m_docPtr;
    xmlNode* m_curNode;
  };
}

#endif
