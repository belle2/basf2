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
#include <framework/pcore/RingBuffer.h>


namespace Belle2 {
  class DataStoreStreamer;

  /** Module to decode data store contents from RingBuffer. */
  class RxModule : public Module {
  public:

    /** Constructor.
     *
     * @param rbuf Use the given RingBuffer for data
     */
    RxModule(RingBuffer* rbuf = 0);
    virtual ~RxModule();

    //! Module functions to be called from main process
    virtual void initialize();

    //! Module functions to be called from event process
    virtual void beginRun();
    virtual void event();
    virtual void endRun();
    virtual void terminate();

  private:
    /** Gets data from m_rbuf and puts it into the data store. */
    void readEvent();

    /** attached RingBuffer. */
    RingBuffer* m_rbuf;

    /** Used for serialization. */
    DataStoreStreamer* m_streamer;

    /**  Compression Level. */
    int m_compressionLevel;

    /** Current event number. */
    int m_nrecv;

  };

} // end namespace Belle2

#endif
