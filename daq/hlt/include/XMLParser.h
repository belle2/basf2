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
#include <string>
#include <vector>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <framework/logging/Logger.h>

#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/UnitInfo.h>
#include <daq/hlt/UnitManager.h>

#ifndef XMLPARSER_H
#define XMLPARSER_H

namespace Belle2 {

  /// @class XMLParser
  /// @brief XML parsing class for HLT
  class XMLParser {
  public:
    XMLParser(void);
    XMLParser(char* filename);
    ~XMLParser(void);

    EStatus parsing(void);
    EStatus parsing(xmlNodePtr cur);
    void unitParsing(xmlNodePtr cur, UnitInfo& unit);

    char* inputName(void);
    char* inputDescription(void);
    int expNo(void);
    int runStart(void);
    int runEnd(void);
    int NUnit(void);
    std::vector<UnitInfo>::iterator unitInfo(void);

    void Print(void);

  private:
    EStatus init(void);

    int m_expNo;
    int m_runStart;
    int m_runEnd;

    std::vector<UnitInfo> m_units;

    char* m_filename;
    char* m_inputName;
    char* m_inputDescription;
    char* m_manager;

    xmlDocPtr m_docPtr;
    xmlNode* m_curNode;
  };
}


#endif
