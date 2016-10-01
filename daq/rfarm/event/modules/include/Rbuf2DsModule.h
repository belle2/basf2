//+
// File : Rbuf2Ds.h
// Description : Module to restore DataStore in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef RBUF2DSMODULE_H
#define RBUF2DSMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/RingBuffer.h>

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include <framework/pcore/DataStoreStreamer.h>
//#include <framework/datastore/DataStore.h>

#define RBUFSIZE 100000000
#define MAXEVTSIZE  400000000


namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Rbuf2DsModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Rbuf2DsModule();
    virtual ~Rbuf2DsModule();

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
    std::string m_rbufname;
    RingBuffer* m_rbuf;

    //! DataStore streamer
    DataStoreStreamer* m_streamer;

    //! Compression Level
    int m_compressionLevel;

    //! No. of sent events
    int m_nrecv;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
