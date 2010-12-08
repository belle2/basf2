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

#include <framework/pcore/RingBuffer.h>

#include "framework/dcore/B2SocketException.h"
#include "framework/dcore/EvtSender.h"
#include "framework/dcore/EvtReceiver.h"
#include "framework/dcore/HLTBuffer.h"

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
    int init(void);
    //! Clearing buffer
    int clearBuffer(void);

    //! Setting port numbers for the communications
    const void setPorts(const int inPort, const int outPort);
    //! Setting IP address of destination which should take data
    const void setDest(const std::string dest);
    const void setDest(std::vector<std::string> dest);
    //! Connecting buffers which are initialized by Node class
    const void setBuffer(HLTBuffer* inBuffer, HLTBuffer* outBuffer);

    RingBuffer* getInBuffer(void);
    RingBuffer* getOutBuffer(void);

    //! Returns IP address of destination which should take data
    std::vector<std::string> dest(void);

    int isMother(void);

    //! Get data from buffer
    std::string get(void);
    //! Put data into buffer
    void put(const std::string data);

    void broadCasting(void);
    std::string listening(void);

    //! Print information of this SignalMan object (only for internal testing)
    const void Print(void);

  private:
    int m_inPort, m_outPort;      /*!< Ports number for data communication */
    std::vector<std::string> m_dest;
    std::string m_mode;           /*!< control / data */

    HLTBuffer* m_inBuffer;        /*!< Incoming buffer */
    HLTBuffer* m_outBuffer;       /*!< Outgoing buffer */

    pid_t m_pidEvtSender, m_pidEvtReceiver;
    RingBuffer* m_inBuf;
    RingBuffer* m_outBuf;

    EvtSender m_sender;           /*!< Event sender */
    EvtReceiver m_receiver;       /*!< Event receiver */
  };
}

#endif
