//+
// File : Ds2SampleModule.h
// Description : Stream DataStore contents and place in in RingBuffer
//               in free running mode
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 12 - Dec - 2013
//-

#ifndef DS2SAMPLEMODULE_H
#define DS2SAMPLEMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/EvtMessage.h>
#include <framework/pcore/RingBuffer.h>

#include <framework/datastore/DataStore.h>
#include <framework/pcore/DataStoreStreamer.h>
//#include <framework/datastore/DataStore.h>

#include <string>

#define RBUFSIZE 100000000

namespace Belle2 {

  /*! A class definition of an input module for Sequential ROOT I/O */

  class Ds2SampleModule : public Module {

    // Public functions
  public:

    //! Constructor / Destructor
    Ds2SampleModule();
    virtual ~Ds2SampleModule();

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

    /** List of objects to be saved. */
    std::vector<std::string> m_saveObjs;

  };

} // end namespace Belle2

#endif // MODULEHELLO_H
