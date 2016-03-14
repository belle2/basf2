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

#include <daq/rawdata/CprErrorMessage.h>
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>
#include <rawdata/dataobjects/RawCOPPER.h>



#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/resource.h>

#include <sys/uio.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


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

    virtual int sendByWriteV(RawDataBlock* rawdblk);
    //    virtual void Connect(const char* hostname, const int port);
    virtual void Accept();

    virtual void fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawDataBlock* rawdblk);
    // Data members

    //! calculate checksum
    unsigned int calcXORChecksum(int* buf, int nwords);

    //! Use shared memory
    int m_shmflag;

    //! open shared memory
    void shmOpen(char* path_cfg, char* path_sta);

    //! Get shared memory
    int* shmGet(int fd, int size_words);

    //! send buffer
    int Send(int socket, char* buf, int size_bytes);

    //!
    void printData(int* buf, int nwords);

    //! file descripter for shm
    int m_shmfd_cfg;

    //! file descripter for shm
    int m_shmfd_sta;

    //! buffer for shared memory
    int* m_cfg_buf;

    //! buffer for shared memory
    int* m_cfg_sta;


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

    double m_prev_time;

    unsigned int m_prev_nevt;

    double m_prev_totbytes;

    double time_array0[1000];
    double time_array1[1000];
    double time_array2[1000];

    //! How to handle data
    std::string p_method;
    int p_method_val;

    //! store time info.
    double getTimeSec();

    //! store time info.
    void recordTime(int event, double* array);

    //! error message program
    CprErrorMessage print_err;

    //! sent data size
    int m_totbytes;

    //! start flag
    int m_start_flag;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
