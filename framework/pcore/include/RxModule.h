//+
// File : rxmodule.h
// Description : Module to restore DataStore in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef RXMODULE_H
#define RXMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/MsgHandler.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/pcore/RbCtlMgr.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

//#include <framework/datastore/DataStore.h>


namespace Belle2 {

  /** Module to decode data store contents from RingBuffer. */
  class RxModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    RxModule();
    //! Use the given RingBuffer for data
    RxModule(RingBuffer*);
    virtual ~RxModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //! RingBuffer ID
    RingBuffer* m_rbuf;

    //! Messaage handler (obsolete)
    MsgHandler* m_msghandler;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! Compression Level
    int m_compressionLevel;

    //! Process contrl manager
    RbCtlMgr* m_rbctl;

    /** No of events */
    int m_nrecv;
    int m_numEvents; // obsolete

  };

} // end namespace Belle2

#endif
