/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef WORKERNODE_H
#define WORKERNODE_H

#include <iostream>
#include <string>
#include <vector>

#include <daq/hlt/Node.h>
#include <daq/hlt/SignalMan.h>
#include <daq/hlt/EventProcess.h>

namespace Belle2 {

  //! WorkerNode class for worker node
  /*! This class is derived from Node class
  */
  class WorkerNode : public Node {
  public:
    //! Constructor
    WorkerNode(void);
    WorkerNode(const std::string ES, const std::string EM);
    //! Destructor
    ~WorkerNode(void);

    //! Initializing this node
    int init(void);

    //! Assigning IP address of event separator
    void eventSeparator(const std::string ES);
    //! Assigning IP address of event merger
    void eventMerger(const std::string EM);

  protected:
    //! Initializing SignalMan objects
    int initSignalMan(void);

  private:
    std::string m_eventSeparator;       /*!< IP address of event separator */
    std::string m_eventMerger;          /*!< IP address of event merger */

    SignalMan* m_signalManToES;         /*!< container for event separator */
    SignalMan* m_signalManToEM;         /*!< container for event merger */

    EventProcess* m_eventProcess;       /*!< EventProcess object which processes the events actually */
  };
}

#endif
