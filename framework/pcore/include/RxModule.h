//+
// File : rxmodule.h
// Description : Module to restore DataStore in RingBuffer
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

  /** Module to decode data store contents from RingBuffer. */
  class RxModule : public Module {
  public:

    /** Constructor.
     *
     * @param rbuf Use the given RingBuffer for data
     */
    RxModule(RingBuffer* rbuf);
    virtual ~RxModule();

    //! Module functions to be called from main process
    virtual void initialize();

    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

    /** Disable handling of Mergeable objects. Useful for special applications like AsyncWrapper. */
    void disableMergeableHandling(bool disable = true) { m_handleMergeable = !disable; }

    /** initialize m_streamer. */
    void initStreamer();

    /** Gets data from m_rbuf and puts it into the data store. */
    void readEvent();

  private:
    /** attached RingBuffer. */
    RingBuffer* m_rbuf;

    /** Used for serialization. */
    DataStoreStreamer* m_streamer;

    /**  Compression Level. */
    int m_compressionLevel;

    /** Current event number. */
    int m_nrecv;

    bool m_handleMergeable = true; /**< Whether to handle Mergeable objects. */

    /** Random Generator object to receive from TxModule */
    StoreObjPtr<RandomGenerator> m_randomgenerator;
  };

} // end namespace Belle2
