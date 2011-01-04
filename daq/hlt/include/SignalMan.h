/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef SIGNALMAN_H
#define SIGNALMAN_H

#include <iostream>
#include <string>
#include <vector>

#include <sys/wait.h>
#include <signal.h>

#include <framework/logging/Logger.h>

#include <daq/hlt/HLTDefs.h>
#include <daq/hlt/RingBuffer.h>
#include <daq/hlt/B2SocketException.h>
#include <daq/hlt/EvtSender.h>
#include <daq/hlt/EvtReceiver.h>

namespace Belle2 {

  //! SignalMan class
  /*! This class is a unit of data transmission.
  */
  class SignalMan {
  public:

    //! Constructor
    SignalMan(void);
    SignalMan(const int inPort, const int outPort);
    SignalMan(const int inPort, const int outPort, std::string dest);
    SignalMan(const int inPort, const int outPort, std::vector<std::string> dest, const std::string mode = "control");

    //! Destructor
    ~SignalMan(void);

    //! Initializing this object
    EStatus init(const std::string inBufName, const std::string outBufName);

    EStatus runEvtSender(void);
    EStatus runEvtReceiver(void);
    EStatus doCommunication(void);

    //! Setting port numbers for the communications
    const void setPorts(const int inPort, const int outPort);
    //! Setting IP address of destination which should take data
    const void setDest(const std::string dest);
    const void setDest(std::vector<std::string> dest);

    RingBuffer* getInBuffer(void);
    RingBuffer* getOutBuffer(void);

    //! Returns IP address of destination which should take data
    std::vector<std::string> dest(void);

    int isMother(void);
    int isEvtSender(void);
    int isEvtReceiver(void);

    //! Get data from buffer
    std::string get(void);
    //! Put data into buffer
    void put(const std::string data);

    void broadCasting(void);
    std::string listening(void);

    //! Print information of this SignalMan object (only for internal testing)
    const void Print(void);

  protected:

  private:
    int m_inPort, m_outPort;      /*!< Ports number for data communication */
    std::vector<std::string> m_dest;
    std::string m_mode;           /*!< control / data */

    pid_t m_pidEvtSender, m_pidEvtReceiver;
    RingBuffer* m_inBuf;
    RingBuffer* m_outBuf;

    EvtSender m_sender;           /*!< Event sender */
    EvtReceiver m_receiver;       /*!< Event receiver */
  };
}

#endif
