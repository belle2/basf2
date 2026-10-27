/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#pragma once

#include <framework/core/Module.h>
#include <framework/pcore/RingBuffer.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/core/RandomGenerator.h>

#include <vector>


namespace Belle2 {
  class DataStoreStreamer;

  /** Module to decode data store contents from RingBuffer. */
  class RxModule : public Module {
  public:

    /** Constructor.
     *
     * @param rbuf Use the given RingBuffer for data
     */
    explicit RxModule(RingBuffer* rbuf);
    virtual ~RxModule() override;

    //! Module functions to be called from main process
    virtual void initialize() override;

    virtual void event() override;
    virtual void terminate() override;

    /** Disable handling of Mergeable objects. Useful for special applications like AsyncWrapper. */
    void disableMergeableHandling(bool disable = true) { m_handleMergeable = !disable; }

    /** initialize m_streamer. */
    void initStreamer();

    /** Gets data from m_rbuf and puts it into the data store.
     *
     * The receive buffer is sized on demand: a record that does not fit stays queued and
     * remq() reports the size it needs, so the buffer is grown and the record read on the
     * next turn of the loop. The buffer therefore settles at the size of the largest event
     * seen so far instead of having to be dimensioned for the worst case up front.
     */
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

    /** Receive buffer in words, grown on demand by readEvent().
     *
     * Must never be empty: data() would be nullptr, which remq() reads as a request to
     * discard the record.
     */
    std::vector<int> m_evtbuf;

    bool m_handleMergeable = true; /**< Whether to handle Mergeable objects. */

    /** Random Generator object to receive from TxModule */
    StoreObjPtr<RandomGenerator> m_randomgenerator;
  };

} // end namespace Belle2
