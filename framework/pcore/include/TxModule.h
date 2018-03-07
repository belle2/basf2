//+
// File : txmodule.h
// Description : Module to put DataStore in RingBuffer
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 13 - Aug - 2010
//-

#pragma once

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>


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

    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /** Whether to block until we can insert data into the ring buffer in event().
     *
     * If this is turned off, some previous data will be removed until enough
     * space is available.
     */
    void setBlockingInsert(bool block) { m_blockingInsert = block; }

    /** Disable handling of Mergeable objects. Useful for special applications like AsyncWrapper. */
    void disableMergeableHandling(bool disable = true) { m_handleMergeable = !disable; }

    /** Disable sending of RandomGenerator state. */
    void disableSendRandomState(bool disable = true) { m_sendRandomState = !disable; }
  private:

    //!Compression parameter
    int m_compressionLevel;

    //! RingBuffer (not owned by us)
    RingBuffer* m_rbuf;

    //! DataStoreStreamer
    DataStoreStreamer* m_streamer;

    //! No. of sent events
    int m_nsent;

    /** Whether to block until we can insert data into the ring buffer in event(). */
    bool m_blockingInsert;

    bool m_handleMergeable = true; /**< Whether to handle Mergeable objects. */
    bool m_sendRandomState = true; /**< Whether to transfer the RandomGenerator state. */

    /** Random Generator object to pass to RxModule */
    StoreObjPtr<RandomGenerator> m_randomgenerator;
  };

} // end namespace Belle2
