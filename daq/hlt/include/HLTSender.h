/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTSENDER_H
#define HLTSENDER_H

#include <string>

#include <boost/lexical_cast.hpp>

#include <daq/hlt/HLTDefs.h>
#include <framework/logging/Logger.h>

#include <daq/hlt/B2Socket.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  //! HLTSender class
  /*! This class is basic component to send data to another node
  */
  class HLTSender : public B2Socket {
  public:
    //! Constructor
    HLTSender(const std::string destination, unsigned int port);
    //! Destructor
    ~HLTSender();

    //! Create a static connection to assigned destination
    EHLTStatus createConnection();
    //! Send data that is read from ring buffer for outgoing data
    EHLTStatus broadcasting();
    //! Send a specific packet of data
    EHLTStatus broadcasting(std::string data);

    //! Set ring buffer with predefined key
    EHLTStatus setBuffer();
    //! Set ring buffer with specific key
    EHLTStatus setBuffer(unsigned int key);

    //! Encode data to be sent to ensure the singleton of data
    std::string makeSingleton(std::string data);

  protected:
    //! Initialize the HLTSender
    EHLTStatus init();

  private:
    std::string m_destination;    /**< Destination to send the data */
    unsigned int m_port;          /**< Port for the connection */

    RingBuffer* m_buffer;         /**< Pointer to ring buffer for outgoing data */
  };
}

#endif
