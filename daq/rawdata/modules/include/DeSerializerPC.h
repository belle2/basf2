//+
// File : DeSerializerPC.h
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef DESERIALIZERPC_H
#define DESERIALIZERPC_H

#include <string>
#include <vector>
#include <stdlib.h>
#include <sys/time.h>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>

#include <daq/dataflow/EvtSocket.h>
#include <daq/dataobjects/RawCOPPER.h>

//#include <daq/dataobjects/RawROPC.h>
#include <daq/rawdata/modules/DAQConsts.h>
#include <framework/dataobjects/EventMetaData.h>
#include <daq/rawdata/modules/DeSerializer.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DeSerializerPCModule : public DeSerializerModule {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerPCModule();
    virtual ~DeSerializerPCModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void event();


  protected :
    //! Accept connection
    virtual int Connect();

    //! receive data
    virtual int Recv(int fd, char* buf, int data_size_byte, int flag);


    //! receive data
    virtual int* RecvData(int* malloc_flag, int* total_m_size_word, int* num_events_in_sendblock, int* num_nodes_in_sendblock);

    virtual int* GetBufArray() {
      int* tempbuf = 0;
      if (m_num_usedbuf < NUM_EVT_PER_BASF2LOOP) {
        tempbuf = m_bufary[ m_num_usedbuf  ];
        m_num_usedbuf++;
      }
      return tempbuf;
    }

    virtual void ClearNumUsedBuf() {
      m_num_usedbuf = 0;
      return ;
    }

    //! # of connections
    int m_num_connections;

    //! Reciever basf2 Socket
    std::vector<EvtSocketRecv*> m_recv;

    //! Reciever Socket
    std::vector<int> m_socket;

    //! hostname of upstream Data Sources
    std::vector<std::string> m_hostname_from;

    //! port # to connect data sources
    std::vector<int> m_port_from;



    //! # of already used buffers
    int m_num_usedbuf;


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
