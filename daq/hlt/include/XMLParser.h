/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

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

  //! XMLParser class
  /*! This class is for parsing of XML input file that is supposed to contain node information
  */
  class XMLParser {
  public:
    //! Constructor
    XMLParser(std::string filename);
    //! Destructor
    ~XMLParser();

    //! Initialize the XMLParser
    EHLTStatus init();
    //! Parsing the XML input file
    EHLTStatus parsing();

    //! Initialize a container for node information with parsed keys
    unsigned int getAllKeys(std::vector<int>& container);
    //! Fill node information parsed into a container by referring specific key
    void fill(int key, NodeInfo& nodeinfo);

    //! Display (Development purpose)
    void display();

  private:
    std::string m_filename;                         /**< XML input file name */

    int m_expNo;                                    /**< Experiment number */
    int m_runStart;                                 /**< Run start number */
    int m_runEnd;                                   /**< Run end number */

    std::string m_managerIP;                        /**< IP address of manager node */

    std::vector<std::string> m_dataSources;         /**< Container for IP addresses of event builder 1 */
    std::vector<std::string> m_dataTargets;         /**< Container for IP addresses of event builder 2 */
    std::map<int, std::string> m_eventSeparators;   /**< Container for IP addresses of event separator nodes */
    std::map<int, std::string> m_workerNodes;       /**< Container for IP addresses of worker nodes */
    std::map<int, std::string> m_eventMergers;      /**< Container for IP addresses of event merger nodes */

    xmlDocPtr m_docPtr;                             /**< XML file document pointer */
    xmlNode* m_curNode;                             /**< Pointer to the current XML tag being parsed */
  };
}

#endif
