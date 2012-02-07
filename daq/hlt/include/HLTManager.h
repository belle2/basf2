/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTMANAGER_H
#define HLTMANAGER_H

#include <map>
#include <wait.h>

#include <framework/logging/Logger.h>
#include <daq/hlt/HLTDefs.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>

#include <daq/hlt/B2Socket.h>
#include <daq/hlt/XMLParser.h>
#include <daq/hlt/NodeInfo.h>
#include <daq/hlt/HLTSender.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  //! HLTManager class
  /*! This class is for manager node.
  */
  class HLTManager {
  public:
    //! Constructor
    HLTManager();
    //! Destructor
    ~HLTManager();

    //! Initialize HLTSenders for node management
    EHLTStatus initSenders();

    //! Store parsed node information into the container from XMLParser
    EHLTStatus storeNodeInfo(XMLParser* xml);
    //! Encode the node information stored to provide them
    std::string encodeNodeInfo(unsigned int key);
    //! Decode the node information encoded
    void decodeNodeInfo(std::string nodeinfo);

    //! Check if the process is child process
    bool isChild();
    //! Wait for the child processes forked
    EHLTStatus checkChildren();

    //! Display node information stored (Development purpose)
    void printNodeInfo();

  private:
    std::map<int, NodeInfo> m_nodeInfoMap;    /**< Container for node information of all nodes */
    std::vector<pid_t> m_senders;             /**< Container for process identifiers of child processes */
    bool m_isChild;                           /**< Flag that denotes if the process is child or not */

    std::vector<int> m_controlBuffers;        /**< Container for outgoing data */
  };
}

#endif
