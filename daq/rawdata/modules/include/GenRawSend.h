//+
// File : genrawsend.h
// Description : Generate Pseudo raw data and send it via socket
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 3 - Sep - 2010
//-

#ifndef GEN_RAW_SEND_H
#define GEN_RAW_SEND_H

#include <framework/core/Module.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <daq/rawdata/modules/DAQConsts.h>
#include <daq/dataflow/EvtSocket.h>

#include <rawdata/dataobjects/RawCOPPER.h>

#ifdef REDUCED_RAWCOPPER
#include <rawdata/dataobjects/RawHeader_latest.h>
#else
#include <rawdata/dataobjects/RawHeader.h>
#endif


namespace Belle2 {

  class GenRawSendModule : public Module {

  public:

    //! Constructor
    GenRawSendModule();

    //! Destructor
    virtual ~GenRawSendModule();

    //! Initialize the Module
    virtual void initialize();

    //! Called when entering a new run
    virtual void beginRun();

    //! Running over all events
    virtual void event();

    //! Is called after processing the last event of a run
    virtual void endRun();

    //! Is called at the end of your Module
    virtual void terminate();

  private:
    //    unsigned int* m_buffer;
    char* m_buffer;
    int m_size;
    int m_maxevt;
    int m_nevt;

    //! size of buffer for one event (word)
    int BUF_SIZE_WORD;

    // Event Meta Data
    StoreObjPtr<EventMetaData> m_eventMetaDataPtr;

    // RawCOPPER object
    StoreObjPtr<RawCOPPER> m_rawcopper;

    // Parameters for EvtSocket

    //! Destination Host
    std::string m_dest;

    //! Destination port
    int m_port;

    //! EvtSocket
    EvtSocketSend* m_sock;


  };
}

#endif /* EXERCISE1_H_ */
