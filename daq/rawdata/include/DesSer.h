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
    static void printData(const int* buf, int nwords);

    //! # of events in a run
    int max_nevt = 0;

    //! time to stop a run
    double max_seconds = 0.0;

    //! Compression Level
    int m_compressionLevel = 0;

    //! No. of sent events
    int n_basf2evt = 0;

    //! No. of prev sent events
    int m_prev_nevt = 0;

    //! dump filename
    std::string m_dump_fname;

    //! dump file descripter
    FILE* m_fp_dump = nullptr;

    //! buffer
    int* m_buffer = nullptr;

    //! buffer
    int* m_bufary[ NUM_PREALLOC_BUF ] = {};
    //    int* m_bufary[NUM_EVT_PER_BASF2LOOP];

    //! Getbuffer
    int* getPreAllocBuf();

    //! Getbuffer
    int* getNewBuffer(int nwords, int* delete_flag);

    // For monitoring
    timeval m_t0;
    double m_recvd_totbytes = 0.0;
    double m_recvd_prev_totbytes = 0.0;
    double m_sent_totbytes = 0.0;
    double m_sent_prev_totbytes = 0.0;
    int m_ncycle = 0;
    //! for time monitoring
    double cur_time = 0.0;

    double m_start_time = 0.0;
    double m_prev_time = 0.0;
    int prev_event = 0;

    //! store time info.
    static double getTimeSec();

    //! store time info.
    void recordTime(int event, double* array);

    //! calculate checksum
    unsigned int calcSimpleChecksum(int* buf, int nwords);

    //! calculate checksum
    static unsigned int calcXORChecksum(const int* buf, int nwords);


    void clearNumUsedBuf()
    {
      m_num_usedbuf = 0;
      return ;
    }

    //! monitor rate
    void RateMonitor(unsigned int nevt);


    //! # of already used buffers
    int m_num_usedbuf = 0;

    //! Node Name for SLC
    std::string m_nodename;

    //! Node ID for SLC
    int m_nodeid = 0;

    //! Run info buffer
    RunInfoBuffer m_status;

    //! run no.
    unsigned int m_exprunsubrun_no;

    //! run no.
    unsigned int m_prev_exprunsubrun_no;

    //! exp no.
    int m_exp_no = 0;

    //! data type
    int m_data_type = 0;

    //! trunc mask
    int m_trunc_mask = 0;

    //! Use shared memory
    int m_shmflag = 0;

    //! open shared memory
    static void shmOpen(char* path_cfg, char* path_sta);

    //! Get shared memory
    int* shmGet(int fd, int size_words);

    //! file descripter for shm
    int m_shmfd_cfg = 0;

    //! file descripter for shm
    int m_shmfd_sta = 0;

    //! buffer for shared memory
    //int* m_cfg_buf;
    //! buffer for shared memory
    //int* m_cfg_sta;

    int monitor_numeve = 0;

    //! start flag
    int m_start_flag = 0;

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
    int* m_cfg_buf = nullptr;

    //! buffer for shared memory
    int* m_cfg_sta = nullptr;

    //! flag to show that run-controller pauses a run
    int m_run_pause = 0;

    //! flag to show that there is some errors with which DAQ cannot continue.
    int m_run_error = 0;

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

    int event_diff = 0;

    unsigned int m_prev_copper_ctr = 0;

    unsigned int m_prev_evenum = 0;

    ///////////////////////////////
    // From Serializer.cc
    ///////////////////////////////

    //! Reciever Socket
    int m_socket_send = 0;

    enum {
      COPPER = 1,
      ROPC = 2
    };

    //! Destination Host
    std::string m_hostname_local;

    //! Destination port
    int m_port_to = 0;

    //! EvtSocket
    //    EvtSocketSend* m_sock;

    //! How to handle data
    std::string p_method;
    int p_method_val = 0;

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
    void callCheckRunPause(const std::string& err_str);

    //! Read data/check a connection
    int CheckConnection(int socket);
#endif

    //! which format is used
    RawHeader_v2 tmp_header; // Should be the latest version before ver.4(PCIe40)

  };

} // end namespace Belle2

#endif // DESSERCOPPER_H
