/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTMERGER_H
#define EVENTMERGER_H

#include <iostream>
#include <string>
#include <vector>

#include <daq/hlt/Node.h>
#include <daq/hlt/SignalMan.h>
#include <daq/hlt/OutputServer.h>
#include <daq/hlt/EventProcess.h>

namespace Belle2 {

  //! WorkerNode class for worker node
  /*! This class is derived from Node class
  */
  class EventMerger : public Node {
  public:
    //! Constructor
    EventMerger(void);
    EventMerger(const std::vector<std::string> WNs, const std::string STOR);
    //! Destructor
    ~EventMerger(void);

    //! Initializing this node
    int init(void);

    //! Assigning IP address of worker nodes
    void workerNodes(const std::vector<std::string> WNs);
    //! Assigning IP address of storage
    void storage(const std::string STOR);

  protected:
    //! Initializing SignalMan objects
    int initSignalMan(void);

  private:
    std::vector<std::string> m_workerNodes;
    std::string m_storage;

    std::vector<SignalMan> m_signalManToWNs;
    SignalMan* m_signalManToStorage;         /*!< container for storage */

    OutputServer* m_outputServer;
  };
}

#endif
