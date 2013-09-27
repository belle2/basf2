//+
// File : DeSerializer.h
// Description : Base class for Module to receive data from outside and store it to DataStore
//
// Author : Satoru Yamada Itoh, IPNS, KEK
// Date : 2 - Aug - 2013
//-

#ifndef DESERIALIZER_H
#define DESERIALIZER_H

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
#include <framework/dataobjects/EventMetaData.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


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

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    virtual void OpenOutputFile();

    virtual void DumpData(char* buf, int size);

  protected :

    // Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    //! data
    StoreObjPtr<RawCOPPER> m_rawcopper;

    //!    StoreObjPtr<RawCOPPER> m_rawcopper;
    StoreArray<RawCOPPER> rawcprarray;

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
    std::string dump_fname;

    //! dump file descripter
    FILE* fp_dump;

    //! buffer
    int* m_buffer;

    //! buffer
    int* m_bufary[ NUM_PREALLOC_BUF ];
    //    int* m_bufary[NUM_EVT_PER_BASF2LOOP];

    //! Getbuffer
    virtual int* GetPreAllocBuf();

    //! Getbuffer
    virtual int* GetBuffer(int nwords, int* malloc_flag);

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
    double GetTimeSec();

    //! store time info.
    void RecordTime(int event, double* array);

    //! check data
    int check_data(char* buf, int prev_eve, int* cur_eve);

    //! calculate checksum
    unsigned int CalcSimpleChecksum(int* buf, int nwords);

    //! calculate checksum
    unsigned int CalcXORChecksum(int* buf, int nwords);

    virtual void ClearNumUsedBuf() {
      m_num_usedbuf = 0;
      return ;
    }




    //! # of already used buffers
    int m_num_usedbuf;


    //! Messaage handler
    MsgHandler* m_msghandler;

    //! Node(PC or COPPER) ID
    int m_nodeid;

    //! run no.
    int m_run_no;

    //! exp no.
    int m_exp_no;

    //! data type
    int m_data_type;

    //! trunc mask
    int m_trunc_mask;

    //! Use shared memory
    int m_shmflag;

    //! open shared memory
    void ShmOpen(char* path_cfg, char* path_sta);

    //! Get shared memory
    int* ShmGet(int fd, int size_words);

    //! file descripter for shm
    int m_shmfd_cfg;

    //! file descripter for shm
    int m_shmfd_sta;

    //! buffer for shared memory
    int* m_cfg_buf;

    //! buffer for shared memory
    int* m_cfg_sta;


    // Data members
  private:


    // Parallel processing parameters



  };

} // end namespace Belle2

#endif // MODULEHELLO_H
