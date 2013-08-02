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

#include <framework/dataobjects/EventMetaData.h>
#include <daq/dataobjects/RawCDC.h>
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
    virtual int Accept();

    //! receive data
    virtual int Recv(int fd, char* buf, int data_size_byte, int flag);

    //! receive data
    virtual int* RecvSocketData(int* malloc_flag, int* total_m_size_word, int* default_buf);

    //! Local host address
    std::string m_local;

    //! Receiver Port
    int m_base_port;

    //! # of connections
    int num_connections;

    //! Reciever Socket
    std::vector<EvtSocketRecv*> m_recv;
    std::vector<int> m_socket;


    // Data members
  private:

    // Parallel processing parameters





  };

} // end namespace Belle2

#endif // MODULEHELLO_H
