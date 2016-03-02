//+
// File : DeSerializerPrePC.h
// Description : Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada, IPNS, KEK
// Date : 4 - Sep - 2014
//-

#ifndef DESSERPREPC_H
#define DESSERPREPC_H

#include <string>
#include <vector>
#include <stdlib.h>
#include <sys/time.h>

/* #include <framework/core/Module.h> */
/* #include <framework/pcore/EvtMessage.h> */
/* #include <framework/pcore/MsgHandler.h> */
/* #include <framework/datastore/DataStore.h> */
/* #include <framework/datastore/StoreObjPtr.h> */
/* #include <framework/datastore/StoreArray.h> */

#include <daq/rawdata/modules/DeSerializer.h>
#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/dataflow/EvtSocket.h>
#include <daq/slc/readout/RunInfoBuffer.h>

#include <rawdata/dataobjects/RawCOPPER.h>
#include <rawdata/dataobjects/PreRawCOPPERFormat_latest.h>


#include <rawdata/dataobjects/RawFTSW.h>
#include <rawdata/dataobjects/RawTLU.h>
//#include <rawdata/RawROPC.h>
//#include <framework/dataobjects/EventMetaData.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>


#define BUF_SIZE_WORD 4800
#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

////////////////////////////////////////////
// From Serializer.cc
////////////////////////////////////////////


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DesSerPrePC {

    // Public functions
  public:

    //! Constructor / Destructor
    DesSerPrePC(string host_recv, int port_recv, string host_send, int port_send, int shmflag,
                const std::string& nodename, int nodeid);
    //    DesSerPrePC();
    virtual ~DesSerPrePC();

    void initialize();

    //! Module functions to be called from event process
    void DataAcquisition();

    //! dump error data
    void printData(int* buf, int nwords);

    //! # of events in a run
    int max_nevt;

    //! time to stop a run
    double max_seconds;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int n_basf2evt;

    //! No. of prev sent events
    int m_prev_nevt;

    //! dump filename
    std::string m_dump_fname;

    //! dump file descripter
    FILE* m_fp_dump;

    //! buffer
    int* m_buffer;

    //! buffer
    int* m_bufary[ NUM_PREALLOC_BUF ];
    //    int* m_bufary[NUM_EVT_PER_BASF2LOOP];

    //! Getbuffer
    int* getPreAllocBuf();

    //! Getbuffer
    int* getNewBuffer(int nwords, int* delete_flag);

    // For monitoring
    timeval m_t0;
    double m_recvd_totbytes;
    double m_recvd_prev_totbytes;
    double m_sent_totbytes;
    double m_sent_prev_totbytes;
    int m_ncycle;
    //! for time monitoring
    double cur_time;

    double m_start_time;
    double m_prev_time;
    int prev_event;

    //! store time info.
    double getTimeSec();

    //! store time info.
    void recordTime(int event, double* array);

    //! check data
    int checkData(char* buf, int prev_eve, int* cur_eve);

    //! calculate checksum
    unsigned int calcSimpleChecksum(int* buf, int nwords);

    //! calculate checksum
    unsigned int calcXORChecksum(int* buf, int nwords);

    void clearNumUsedBuf()
    {
      m_num_usedbuf = 0;
      return ;
    }

    //! monitor rate
    void RateMonitor(unsigned int nevt);


    //! # of already used buffers
    int m_num_usedbuf;


    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Node(PC or COPPER) ID
    int m_nodeid;

    //! Node name
    std::string m_nodename;

    //! Run info buffer
    RunInfoBuffer m_status;


    //! run no.
    unsigned int m_exprunsubrun_no;

    //! run no.
    unsigned int m_prev_exprunsubrun_no;

    //! exp no.
    int m_exp_no;

    //! data type
    int m_data_type;

    //! trunc mask
    int m_trunc_mask;

    //! Use shared memory
    int m_shmflag;

    //! open shared memory
    void shmOpen(char* path_cfg, char* path_sta);

    //! Get shared memory
    int* shmGet(int fd, int size_words);

    //! file descripter for shm
    int m_shmfd_cfg;

    //! file descripter for shm
    int m_shmfd_sta;

    //! buffer for shared memory
    //int* m_cfg_buf;
    //! buffer for shared memory
    //int* m_cfg_sta;

    int monitor_numeve;

    //! start flag
    int m_start_flag;

    //! wrapper for B2LOG system
    CprErrorMessage print_err;


    ///////////////////////////////
    // From Serializer.cc
    ///////////////////////////////

    //! Module functions to be called from main process
    void initializeNode();

    //! Module functions to be called from event process
    void beginRun();

    void endRun();
    void terminate();

    int sendByWriteV(RawDataBlock* rawdblk);
    //    void Connect(const char* hostname, const int port);
    void Accept();

    void fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawDataBlock* rawdblk);
    // Data members

    //! send buffer
    int Send(int socket, char* buf, int size_bytes);


    //! buffer for shared memory
    int* m_cfg_buf;

    //! buffer for shared memory
    int* m_cfg_sta;

    //! flag to show that run-controller pauses a run
    int m_run_pause;

    //! flag to show that there is some errors with which DAQ cannot continue.
    int m_run_error;

  protected :
    //! Accept connection
    int Connect();

    //! receive data
    int recvFD(int fd, char* buf, int data_size_byte, int flag);

    //! receive data
    int* recvData(int* delete_flag, int* total_m_size_word, int* num_events_in_sendblock, int* num_nodes_in_sendblock);

    //! attach buffer to RawDataBlock
    void setRecvdBuffer(RawDataBlock* raw_datablk, int* delete_flag);

    //! check data contents
    void checkData(RawDataBlock* raw_datablk, unsigned int* eve_copper_0);


    //! check data contents
    //   StoreArray<RawDataBlock> raw_datablkarray;

    //! # of connections
    int m_num_connections;

    //! Reciever basf2 Socket
    std::vector<EvtSocketRecv*> m_recv;

    //! hostname of upstream Data Sources
    std::vector<std::string> m_hostname_from;

    //! port # to connect data sources
    std::vector<int> m_port_from;

    std::vector<int> m_socket_recv;

    StoreArray<RawFTSW> raw_ftswarray;

    StoreArray<RawCOPPER> rawcprarray;

    //    StoreArray<ReducedRawCOPPER> rawcprarray;
    ///
    PreRawCOPPERFormat_latest m_pre_rawcpr;

    int event_diff;

    unsigned int m_prev_copper_ctr;

    unsigned int m_prev_evenum;

    ///////////////////////////////
    // From Serializer.cc
    ///////////////////////////////

    //! Reciever Socket
    int m_socket_send;

    enum {
      COPPER = 1,
      ROPC = 2
    };

    //! Destination Host
    std::string m_hostname_local;

    //! Destination port
    int m_port_to;

    //! EvtSocket
    EvtSocketSend* m_sock;

    //! How to handle data
    std::string p_method;
    int p_method_val;

#ifdef NONSTOP
    //! for nonstop DAQ
    int* m_ptr;

    //! run no.
    int m_run_no;

    //! run no.
    int m_prev_run_no;

    //! for nonstop DAQ
    int checkRunPause();

    //! for nonstop DAQ
    int checkRunRecovery();

    //! for nonstop DAQ
    void openRunPauseNshm();

    //! for nonstop DAQ
    void resumeRun();

    //! for nonstop DAQ
    void pauseRun();

    //! for nonstop DAQ
    void waitResume();

    //! call checkrunstop
    void callCheckRunPause(std::string& err_str);

    //! Read data/check a connection
    int CheckConnection(int socket);
#endif

    //! which format is used
    RawHeader_latest tmp_header;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
