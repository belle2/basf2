//+
// File : txmodule.h
// Description : Module to put DataStore in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#ifndef TXMODULE_H
#define TXMODULE_H

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/pcore/DataStoreStreamer.h>


namespace Belle2 {

  /** Module for encoding data store contents into a RingBuffer. */
  class TxModule : public Module {

    // Public functions
  public:

    /** Constructor.
     *
     * @param rbuf Use the given RingBuffer for data
     */
    TxModule(RingBuffer* rbuf = 0);
    virtual ~TxModule();

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
    RingBuffer* m_rbuf;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! No. of sent events
    int m_nsent;

  };

} // end namespace Belle2

#endif
