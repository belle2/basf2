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
#include <rawdata/dataobjects/ErrorMessage.h>

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/RawCDC.h>
#include <rawdata/dataobjects/RawSVD.h>
#include <rawdata/dataobjects/RawECL.h>
#include <rawdata/dataobjects/RawEPID.h>
#include <rawdata/dataobjects/RawBPID.h>
#include <rawdata/dataobjects/RawKLM.h>
#include <rawdata/dataobjects/RawFTSW.h>


//#include <rawdata/RawROPC.h>
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
    virtual int recvFD(int fd, char* buf, int data_size_byte, int flag);


    //! receive data
    virtual int* recvData(int* malloc_flag, int* total_m_size_word, int* num_events_in_sendblock, int* num_nodes_in_sendblock);



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


    //    StoreArray<RawDataBlock> raw_dblkarray;
    StoreArray<RawCOPPER> rawcprarray;
    StoreArray<RawCDC> raw_cdcarray;
    StoreArray<RawSVD> raw_svdarray;
    StoreArray<RawBPID> raw_bpidarray;
    StoreArray<RawECL> raw_eclarray;
    StoreArray<RawEPID> raw_epidarray;
    StoreArray<RawKLM> raw_klmarray;
    StoreArray<RawFTSW> raw_ftswarray;

    ErrorMessage print_err;

    int event_diff;

    unsigned int m_prev_copper_ctr;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
