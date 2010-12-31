/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef EVENTSEPARATOR_H
#define EVENTSEPARATOR_H

#include <iostream>
#include <string>
#include <vector>

#include <daq/hlt/Node.h>
#include <daq/hlt/SignalMan.h>
#include <daq/hlt/EventServer.h>

namespace Belle2 {

  //! EventSeparator class which is supposed to be used on event separator node
  /*! This class is derived from Node class
  */
  class EventSeparator : public Node {
  public:
    //! Constructor
    EventSeparator(int nproc);
    EventSeparator(const std::string EB, const std::vector<std::string> WNs);
    //! Destructor
    ~EventSeparator(void);

    //! Initializing the event separator node
    int init(void);

    virtual RingBuffer* getInBuffer(void);
    virtual RingBuffer* getOutBuffer(void);

    //! Assinging an IP address of event builder
    void eventBuilder(const std::string EB);
    //! Assigning IP addresses of worker nodes
    void workerNodes(const std::vector<std::string> WNs);

    //! Pushing data in buffer through network
    void push(void);
    //! Event separation by using EventServer class
    void process(const std::string input);

    //! Printing information of this node (internal testing only)
    void Print(void);

  protected:
    //! Initializing SignalMan objects
    int initSignalMan(void);

  private:
    std::string m_eventBuilder;                   /*!< IP address of event bulder */
    std::vector<std::string> m_workerNodes;       /*!< IP addresses of worker nodes */
    int m_nproc;

    SignalMan* m_signalMan;

    EventServer* m_eventServer;                   /*!< EventServer object to separate events actually */
  };
}

#endif
