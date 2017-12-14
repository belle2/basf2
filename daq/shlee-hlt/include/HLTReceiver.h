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
#include <sys/time.h>

#include <boost/lexical_cast.hpp>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/hlt/B2Socket.h>
#include <framework/pcore/RingBuffer.h>

namespace Belle2 {

  //! HLTReceiver class
  /*! This class is basic component to receive data from another node
  */
  class HLTReceiver : public B2Socket {
  public:
    //! Constructor
    HLTReceiver(unsigned int port, unsigned int nSources);
    //! Destructor
    ~HLTReceiver();

    //! Create a static connection with assigned port
    EHLTStatus createConnection();
    //! Start to listen from assigned port
    EHLTStatus listening();
    //! Get data from source
    int getData();

    //! Set buffer with predefined key
    EHLTStatus setBuffer();
    //! Set buffer with specific integer key
    EHLTStatus setBuffer(unsigned int key);
    //! Set buffer with specific string key
    EHLTStatus setBuffer(std::string key);

    ///! Set node type
    void setMode(EHLTNodeType nodeType);
    ///! Set port number for data transfer
    EHLTStatus setPort(int port);

    //! Find EOS tag from the data
    int findEOS(char* data, int size);

  protected:
    //! Initialize the HLTReceiver
    EHLTStatus init();

  private:
    unsigned int m_port;            /**< Port number for the connection */
    unsigned int m_nSources;        /**< Number of data sources */
    int m_tempBufferSize;           /**< Size of temporary buffer */

    RingBuffer* m_buffer;           /**< Pointer to ring buffer for incoming data */
  };
}

#endif
