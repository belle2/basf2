/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Soohyung Lee                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#ifndef HLTRECEIVER_H
#define HLTRECEIVER_H

#include <sstream>
#include <fstream>

#include <boost/lexical_cast.hpp>

#include <daq/hlt/B2Socket.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  //! HLTReceiver class
  /*! This class is basic component to receive data from another node
  */
  class HLTReceiver : public B2Socket {
  public:
    //! Constructor
    HLTReceiver(unsigned int port);
    //! Destructor
    ~HLTReceiver();

    //! Create a static connection with assigned port
    EHLTStatus createConnection();
    //! Start to listen from assigned port
    EHLTStatus listening();

    //! Set buffer with predefined key
    EHLTStatus setBuffer();
    //! Set buffer with specific key
    EHLTStatus setBuffer(unsigned int key);

    //! Decode received data to ensure a singleton of data
    EHLTStatus decodeSingleton(std::string data, std::vector<std::string>& container);
    int decodeSingleton(char* data, int size, char** container, int* sizes);

    int findEOS(char* data, int size);
    void writeFile(char* file, char* data, int size);

  protected:
    //! Initialize the HLTReceiver
    EHLTStatus init();
    EHLTStatus flushInternalBuffer();

  private:
    unsigned int m_port;            /**< Port number for the connection */

    RingBuffer* m_buffer;           /**< Pointer to ring buffer for incoming data */

    char* m_internalBuffer;
    int m_internalBufferWriteIndex;
    int m_internalBufferEntries;
  };
}

#endif
