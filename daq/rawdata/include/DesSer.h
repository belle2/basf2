/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DESSER_H
#define DESSER_H

#include <string>
#include <vector>

#include <sys/time.h>

#include <daq/slc/readout/RunInfoBuffer.h>
#include <daq/rawdata/CprErrorMessage.h>
#include <daq/rawdata/modules/DAQConsts.h>
#include <rawdata/dataobjects/PreRawCOPPERFormat_v2.h> // Should be the latest version before ver.4(PCIe40)
#include <rawdata/switch_basf2_standalone.h>

#include <daq/dataobjects/SendHeader.h>
#include <daq/dataobjects/SendTrailer.h>

#define BUF_SIZE_WORD 4800

////////////////////////////////////////////
// From Serializer.cc
////////////////////////////////////////////


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class DesSer {

    // Public functions
  public:

    //! Constructor / Destructor
    DesSer();

    //    DesSer();
    virtual ~DesSer();

    void initialize(bool close_listen = true);

    //! Module functions to be called from event process
    virtual void DataAcquisition() = 0;

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

    //! Node Name for SLC
    std::string m_nodename;

    //! Node ID for SLC
    int m_nodeid;

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

    //! report status to SLC
    //    static RunInfoBuffer g_status;
    //    RunInfoBuffer g_status;

    //! Use ver.2 for the moment(ver.4 -> PCI40)
    PreRawCOPPERFormat_v2 m_pre_rawcpr; // Should be the latest version before ver.4(PCIe40)

    ///////////////////////////////
    // From Serializer.cc
    ///////////////////////////////

    //! Module functions to be called from main process
    void initializeNode();

    //! Module functions to be called from event process
    void beginRun();

    void endRun();
    void terminate();

    //    int sendByWriteV(RawDataBlock* rawdblk);
    int sendByWriteV(RawDataBlockFormat* rawdblk);
    //    void Connect(const char* hostname, const int port);
    void Accept(bool close_listen = true);

    //    void fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawDataBlock* rawdblk);
    void fillSendHeaderTrailer(SendHeader* hdr, SendTrailer* trl, RawDataBlockFormat* rawdblk);

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
    //! check data contents
    //   StoreArray<RawDataBlock> raw_datablkarray;

    //! # of connections
    int m_num_connections;

    //! Reciever basf2 Socket
    //    std::vector<EvtSocketRecv*> m_recv;

    //! hostname of upstream Data Sources
    std::vector<std::string> m_hostname_from;

    //! port # to connect data sources
    std::vector<int> m_port_from;

    std::vector<int> m_socket_recv;

    //    StoreArray<RawFTSW> raw_ftswarray;

    //    StoreArray<RawCOPPER> rawcprarray;

    //    StoreArray<ReducedRawCOPPER> rawcprarray;

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
    //    EvtSocketSend* m_sock;

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
    RawHeader_v2 tmp_header; // Should be the latest version before ver.4(PCIe40)

  };

} // end namespace Belle2

#endif // DESSERCOPPER_H
