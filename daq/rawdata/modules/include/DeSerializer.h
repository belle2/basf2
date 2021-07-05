/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#ifndef DESERIALIZER_H
#define DESERIALIZER_H

#include <string>
#include <stdlib.h>
#include <sys/time.h>

#include <framework/core/Module.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/slc/readout/RunInfoBuffer.h>
#include <daq/rawdata/CprErrorMessage.h>
#include <daq/rawdata/modules/DAQConsts.h>

#include <rawdata/dataobjects/RawDataBlock.h>

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */


  class DeSerializerModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    DeSerializerModule();
    virtual ~DeSerializerModule();

    //! Module functions to be called from main process
    virtual void initialize();

    virtual void terminate();

    //! Module functions to be called from main process
    virtual void event();

    //! Module functions to be called from event process
    virtual void openOutputFile();

    //! dump binary data
    virtual void dumpData(char* buf, int size);

    //! dump error data
    virtual void printData(int* buf, int nwords);

    //! dump error data
    virtual void printASCIIData(int* buf, int nwords);

  protected :

    // Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    StoreArray<RawDataBlock> raw_datablkarray;     //!  StoreArray


    //! # of events in a run
    int max_nevt;

    //! time to stop a run
    double max_seconds;

    //! Compression Level
    int m_compressionLevel;

    //! size of buffer for one event (word)
    int BUF_SIZE_WORD;

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
    virtual int* getPreAllocBuf();

    //! Getbuffer
    virtual int* getNewBuffer(int nwords, int* delete_flag);

    // For monitoring
    timeval m_t0;
    double m_totbytes;
    double m_prev_totbytes;
    int m_ncycle;
    //! for time monitoring
    double cur_time;

    double m_start_time;
    double m_prev_time;
    double time_array0[1000];
    double time_array1[1000];
    double time_array2[1000];
    double time_array3[1000];
    double time_array4[1000];
    double time_array5[1000];
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

    virtual void clearNumUsedBuf()
    {
      m_num_usedbuf = 0;
      return ;
    }

    //! monitor rate
    void RateMonitor(unsigned int nevt, int subrun = -1, int run = -1);


    //! # of already used buffers
    int m_num_usedbuf;


    //! Node(PC or COPPER) ID
    int m_nodeid;

    //! Node name
    std::string m_nodename;

    //! run no.
    int m_run_no;

    //! run no.
    int m_prev_run_no;

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

    //! buffer class to communicate with NSM client
    static RunInfoBuffer g_status;

    //! buffer for shared memory
    //int* m_cfg_buf;
    //! buffer for shared memory
    //int* m_cfg_sta;

    int monitor_numeve;

    //! start flag
    int m_start_flag;

    //! wrapper for B2LOG system
    CprErrorMessage print_err;


#ifdef NONSTOP
    //! for nonstop DAQ
    int* m_ptr;

    //! for nonstop DAQ
    virtual void openRunPauseNshm();

    //! for nonstop DAQ
    virtual int checkRunPause();

    //! for nonstop DAQ
    virtual int checkRunRecovery();


    //! for nonstop DAQ
    virtual void resumeRun();

    //! for nonstop DAQ
    virtual void pauseRun();

    //! for nonstop DAQ
    virtual void waitResume();

    //! check RunPause
    void callCheckRunPause(std::string& err_str);

    //! check connection status
    int CheckConnection(int socket);

#endif

  private:

  public:
    static RunInfoBuffer& getStatus()
    {
      return g_status;
    }


  };

} // end namespace Belle2

#endif // MODULEHELLO_H
