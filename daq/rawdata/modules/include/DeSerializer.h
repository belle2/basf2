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
    int* m_bufary[NUM_EVT_PER_BASF2LOOP];

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

    //! Messaage handler
    MsgHandler* m_msghandler;


    // Data members
  private:

    // Parallel processing parameters





  };

} // end namespace Belle2

#endif // MODULEHELLO_H
