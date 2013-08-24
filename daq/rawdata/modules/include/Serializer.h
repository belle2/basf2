//+
// File : Serializer.h
// Description : Module to get data from DataStore and send it to another network node
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdlib.h>
#include <string>
#include <vector>

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>

#include <daq/dataflow/EvtSocket.h>

#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/dataobjects/RawCOPPER.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#include <sys/uio.h>



namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class SerializerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    SerializerModule();
    virtual ~SerializerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    virtual void SendByWriteV(RawCOPPER* rawcpr);
    //    virtual void Connect(const char* hostname, const int port);
    virtual void Accept();

    virtual void FillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawCOPPER* rawcpr);
    // Data members

    //! calculate checksum
    unsigned int CalcXORChecksum(int* buf, int nwords);

  protected :

    enum {
      COPPER = 1,
      ROPC = 2
    };

    //! size of buffer for one event (word)
    int BUF_SIZE_WORD;

    //!Compression parameter
    int m_compressionLevel;

    //! Messaage handler
    MsgHandler* m_msghandler;

    //! No. of sent events
    int n_basf2evt;

    int* m_buffer;

    // Parameters for EvtSocket

    //! Destination Host
    std::string m_hostname_local;

    //! Destination port
    int m_port_to;

    //! Socket ID
    int m_socket;

    //! EvtSocket
    EvtSocketSend* m_sock;
    double m_start_time;

    double time_array0[1000];
    double time_array1[1000];
    double time_array2[1000];

    //! How to handle data
    std::string p_method;
    int p_method_val;

    //! store time info.
    double GetTimeSec();

    //! store time info.
    void RecordTime(int event, double* array);


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
