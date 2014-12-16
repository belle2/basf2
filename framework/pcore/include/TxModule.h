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


namespace Belle2 {
  class DataStoreStreamer;

  /** Module for encoding data store contents into a RingBuffer. */
  class TxModule : public Module {

    // Public functions
  public:

    /** Constructor.
     *
     * @param rbuf Use the given RingBuffer for data
     */
    TxModule(RingBuffer* rbuf);
    virtual ~TxModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /** Wether to block until we can insert data into the ring buffer in event().
     *
     * If this is turned off, some previous data will be removed until enough
     * space is available.
     */
    void setBlockingInsert(bool block) { m_blockingInsert = block; }

  private:

    //!Compression parameter
    int m_compressionLevel;

    //! RingBuffer ID
    RingBuffer* m_rbuf;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! No. of sent events
    int m_nsent;

    /** Wether to block until we can insert data into the ring buffer in event(). */
    bool m_blockingInsert;

  };

} // end namespace Belle2

#endif
