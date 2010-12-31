/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef NODEMANAGER_H
#define NODEMANAGER_H

#include <iostream>
#include <string>

#include <framework/logging/Logger.h>

#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/SignalMan.h>
#include <daq/hlt/Node.h>
#include <daq/hlt/NodeInfo.h>

namespace Belle2 {

  //! NodeManager class
  /*! This class contains the information of a single node.
   *! It is supposed to make a pair in manager node and correspoding node both.
   */
  class NodeManager {
  public:
    //! Constructor
    NodeManager(void);
    NodeManager(int nodeNo, int unitNo);
    NodeManager(NodeInfo* nodeinfo);
    //! Destructor
    ~NodeManager(void);

    //! Initializing the node information
    EStatus init(const std::string manager);
    void setNodeInfo(std::string nodeinfo);

    //! Setting unique unit number of this node
    void setUnitNo(const int unitNo);
    //! Setting unique node number of this node
    void setNodeNo(const int nodeNo);

    std::string manager(void);

    void broadCasting(void);
    std::string listen(void);
    void buildNodeInfo(void);
    NodeInfo* nodeInfo(void);

    int isEvtSender(void);
    int isEvtReceiver(void);

    //! Print information of the current node (only for internal testing)
    void Print(void);

  protected:
    //! Initializing SignalMan object for communications (protected)
    EStatus initSignalMan(void);

  private:
    std::string m_manager;        /*!< IP address of manager node */
    int m_unitNo;                 /*!< Unique unit number of this node */
    int m_nodeNo;                 /*!< Unique node number of this node */

    Node* m_node;                 /*!< Pointer to actual node (only for node side, not for manager side) */
    NodeInfo* m_nodeinfo;
    SignalMan* m_infoSignalMan;   /*!< Pointer to SignalMan object */
  };
}

#endif
