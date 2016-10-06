//+
// File : Ds2RbufModule.h
// Description : Stream DataStore contents and place in in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 26 - Apr - 2012
//-

#ifndef DS2RBUFMODULE_H
#define DS2RBUFMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/RingBuffer.h>

#include <framework/datastore/DataStore.h>
#include <framework/pcore/DataStoreStreamer.h>
#include <framework/datastore/DataStore.h>

#include <string>

#define RBUFSIZE 100000000

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Ds2RbufModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Ds2RbufModule();
    virtual ~Ds2RbufModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    // Data members
  private:

    //!Compression parameter
    int m_compressionLevel;

    //! RingBuffer ID
    std::string m_rbufname;
    RingBuffer* m_rbuf;

    //! DataStore streamer
    DataStoreStreamer* m_streamer;

    //! No. of sent events
    int m_nsent;

    //! List of objects to be transferred
    std::vector<std::string> m_saveobjs;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
