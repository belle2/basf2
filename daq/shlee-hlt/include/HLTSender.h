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
#include <sys/time.h>

#include <boost/lexical_cast.hpp>

#include <daq/hlt/HLTDefs.h>
#include <framework/logging/Logger.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

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
    //! Send serialized data
    EHLTStatus sendData(char* data, int size);

    //! Set ring buffer with predefined key
    EHLTStatus setBuffer();
    //! Set ring buffer with specific integer key
    EHLTStatus setBuffer(unsigned int key);
    //! Set ring buffer with specific string key
    EHLTStatus setBuffer(std::string key);

    //! Set node type
    void setMode(EHLTNodeType nodeType);
    //! Set port number for data transfer
    EHLTStatus setPort(int port);

    //! Encode data to be sent to ensure the singleton of data for a string
    std::string makeSingleton(std::string data);
    //! Encode data to be sent to ensure the singleton of data for general data
    EHLTStatus makeSingleton(char* data, int size);

  protected:
    //! Initialize the HLTSender
    EHLTStatus init();

  private:
    std::string m_destination;    /**< Destination to send the data */
    unsigned int m_port;          /**< Port for the connection */
    int m_tempBufferSize;         /**< Size of temporary buffer */

    RingBuffer* m_buffer;         /**< Pointer to ring buffer for outgoing data */
  };
}

#endif
